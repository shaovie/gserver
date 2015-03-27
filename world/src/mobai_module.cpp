#include "mobai_module.h"
#include "player_obj.h"
#include "sys_log.h"
#include "error.h"
#include "guild_module.h"
#include "unused_arg.h"
#include "sys.h"
#include "worship_info.h"
#include "all_char_info.h"
#include "player_mgr.h"
#include "global_param_cfg.h"
#include "cache_module.h"
#include "db_proxy.h"
#include "package_module.h"
#include "behavior_id.h"
#include "time_util.h"
#include "lvl_param_cfg.h"
#include "vip_module.h"
#include "task_module.h"
#include "cheng_jiu_module.h"
#include "huo_yue_du_module.h"

// Lib header

// Defines
#define MAX_WORSHIP_COIN 99999999

worship_info *mobai_module::wrc_info = new worship_info();

static ilog_obj *s_log = sys_log::instance()->get_ilog("mobai");
static ilog_obj *e_log = err_log::instance()->get_ilog("mobai");

void mobai_module::on_guild_demise(const int guild_id)
{
  if (guild_module::get_ghz_winner() != guild_id)
    return ;
  mobai_module::do_replace_castellan(guild_module::get_ghz_winner_chairman());
}
void mobai_module::on_guild_dismiss(const int guild_id)
{
  if (guild_module::get_ghz_winner() != guild_id)
    return ;
  mobai_module::do_replace_castellan(0);
}
void mobai_module::on_ghz_result(const int old_win_guild_id, const int win_guild_id)
{
  if (old_win_guild_id == win_guild_id) return ;
  mobai_module::do_replace_castellan(guild_module::get_ghz_winner_chairman());
}
void mobai_module::do_replace_castellan(const int new_id)
{
  mobai_module::wrc_info->left_coin_ = 0;
  mobai_module::wrc_info->acc_coin_  = 0;
  mobai_module::wrc_info->acc_worship_cnt_ = 0;
  mobai_module::update_wrc_info();

  char career = -1;
  char_brief_info *new_cbi = all_char_info::instance()->get_char_brief_info(new_id);
  if (new_cbi != NULL)
    career = new_cbi->career_;
  mobai_module::do_bro_worsip_statue_info_to_clt(career);
}
void mobai_module::do_bro_worsip_statue_info_to_clt(const char career)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << career;
  player_mgr::instance()->broadcast_to_scene(global_param_cfg::ghz_scene_cid, NTF_WORSHIP_STATUE_INFO, &mb);
}
void mobai_module::on_transfer_scene(player_obj *player,
                                     const int from_scene_cid,
                                     const int to_scene_cid)
{
  if (from_scene_cid != to_scene_cid
      && to_scene_cid == global_param_cfg::ghz_scene_cid)
    mobai_module::do_notify_worsip_statue_info_to_clt(player);
}
void mobai_module::on_enter_game(player_obj *player)
{
  if (player->scene_cid() == global_param_cfg::ghz_scene_cid)
    mobai_module::do_notify_worsip_statue_info_to_clt(player);
}
void mobai_module::do_notify_worsip_statue_info_to_clt(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  const int chairman_id = guild_module::get_ghz_winner_chairman();
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(chairman_id);
  if (cbi == NULL) return ;
  mb << cbi->career_;
  player->do_delivery(NTF_WORSHIP_STATUE_INFO, &mb);
}
void mobai_module::update_wrc_info()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << stream_ostr((char *)mobai_module::wrc_info, sizeof(worship_info));
  db_proxy::instance()->send_request(0, REQ_UPDATE_WORSHIP_INFO, &os);
}
int mobai_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = mobai_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_WORSHIP_INFO, clt_obtain_worship_info);
    SHORT_CODE(REQ_WORSHIP_CASTELLAN,   clt_worship_castellan);
    SHORT_CODE(REQ_GET_WORSHIP_COIN,    clt_get_worship_coin);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int mobai_module::clt_obtain_worship_info(player_obj *player, const char *, const int )
{
  const int chairman_id = guild_module::get_ghz_winner_chairman();
  if (chairman_id == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  player_obj *chairman = player_mgr::instance()->find(chairman_id);
  if (chairman == NULL)
  {
    if (all_char_info::instance()->get_char_brief_info(chairman_id) == NULL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

    ilist<item_obj *> *elist = cache_module::get_player_equip(chairman_id);
    if (elist != NULL) // cache
      mobai_module::do_build_worship_info(player, NULL, chairman_id, elist, os);
    else // miss
    {
      os << player->db_sid() << chairman_id << (char)PKG_EQUIP;
      db_proxy::instance()->send_request(chairman_id, REQ_GET_CASTELLAN_ITEM_SINGLE_PKG, &os);
      return 0;
    }
  }else // online
  {
    static ilist<item_obj*> equip_list;
    equip_list.clear();
    package_module::find_all_item_in_pkg(chairman, PKG_EQUIP, equip_list);
    mobai_module::do_build_worship_info(player, chairman, chairman_id, &equip_list, os);
  }
  return player->send_respond_ok(RES_OBTAIN_WORSHIP_INFO, &os);
}
void mobai_module::handle_db_get_castellan_item_single_pkg_result(player_obj *player, in_stream &is)
{
  int target_id = 0;
  int cnt = 0;
  is >> target_id >> cnt;

  ilist<item_obj*> *elist = new ilist<item_obj *>();
  for (int i = 0; i < cnt; ++i)
  {
    item_obj *io = package_module::alloc_new_item();
    is.rd_ptr(sizeof(short)); // Refer: db_proxy::proxy_obj.cpp::proc_result::for (mblock
    is >> io;
    elist->push_back(io);
  }
  cache_module::equip_cache_add_player(target_id, elist);

  out_stream os(client::send_buf, client::send_buf_len);
  elist = cache_module::get_player_equip(target_id);
  if (elist != NULL) // cache
    mobai_module::do_build_worship_info(player, NULL, target_id, elist, os);
  player->send_respond_ok(RES_OBTAIN_WORSHIP_INFO, &os);
}
void mobai_module::do_build_worship_info(player_obj *self,
                                         player_obj *target,
                                         const int target_id,
                                         ilist<item_obj*> *list,
                                         out_stream &os)
{
  if (target != NULL)
  {
    os << target->id()
      << stream_ostr(target->name(), ::strlen(target->name()))
      << target->career()
      << target->lvl()
      << vip_module::vip_lvl(target)
      << target->zhan_li();
  }else
  {
    char_brief_info *info = all_char_info::instance()->get_char_brief_info(target_id);
    if (info == NULL) return;

    os << target_id
      << stream_ostr(info->name_, ::strlen(info->name_))
      << info->career_
      << info->lvl_
      << info->vip_
      << info->zhan_li_;
  }
  os << (char)(list->size());
  ilist_node<item_obj*> *iter = list->head();
  for (; iter != NULL; iter = iter->next_)
    package_module::do_build_item_info(iter->value_, os);

  os << guild_module::get_ghz_winner_chairman();
  const int guild_id = guild_module::get_ghz_winner();
  char *guild_name = guild_module::get_guild_name(guild_id);
  if (guild_name == NULL) return ;
  os << stream_ostr(guild_name, ::strlen(guild_name))
    << mobai_module::wrc_info->left_coin_
    << mobai_module::wrc_info->acc_coin_
    << mobai_module::wrc_info->acc_worship_cnt_;

  const int left_time = global_param_cfg::worship_cd - time_util::now + self->worship_time();
  os << (left_time > 0 ? left_time : 0)
    << self->worship_cnt();
}
int mobai_module::clt_worship_castellan(player_obj *player, const char *, const int )
{
  const int chairman_id = guild_module::get_ghz_winner_chairman();
  if (chairman_id == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (player->worship_cnt()
      >= global_param_cfg::daily_worship_cnt + vip_module::to_get_add_worship_cnt(player))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_TURN_FREE_CNT_NOT_ENOUGH);

  if (time_util::now - player->worship_time() < global_param_cfg::worship_cd)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_NO_REACH_TIME);

  const int got_coin = lvl_param_cfg::instance()->worship_coin(player->lvl());
  if (player->is_money_upper_limit(M_COIN, got_coin))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);
  player->do_got_money(got_coin, M_COIN, MONEY_GOT_WROSHIP, 0);

  const int64_t got_exp = lvl_param_cfg::instance()->worship_exp(player->lvl());
  player->do_got_exp(got_exp);

  const int chairman_got_coin = got_coin * global_param_cfg::worship_coin_per / 10000;
  const int64_t left_coin = mobai_module::wrc_info->left_coin_ + chairman_got_coin;
  mobai_module::wrc_info->left_coin_ = left_coin > MAX_WORSHIP_COIN ? MAX_WORSHIP_COIN : left_coin;
  const int64_t acc_coin = mobai_module::wrc_info->acc_coin_ + chairman_got_coin;
  mobai_module::wrc_info->acc_coin_ = acc_coin > MAX_WORSHIP_COIN ? MAX_WORSHIP_COIN : acc_coin;
  mobai_module::wrc_info->acc_worship_cnt_++;
  mobai_module::update_wrc_info();

  player->worship_cnt(player->worship_cnt() + 1);
  player->worship_time(time_util::now);
  player->db_save_char_extra_info();
  player->db_save_daily_clean_info();

  task_module::on_mo_bai(player);
  cheng_jiu_module::on_mo_bai(player);
  huo_yue_du_module::on_mo_bai(player);

  out_stream os(client::send_buf, client::send_buf_len);
  os << mobai_module::wrc_info->acc_worship_cnt_
    << mobai_module::wrc_info->left_coin_
    << mobai_module::wrc_info->acc_coin_
    << player->worship_cnt();
  return player->send_respond_ok(RES_WORSHIP_CASTELLAN, &os);
}
int mobai_module::clt_get_worship_coin(player_obj *player, const char *, const int )
{
  if (player->id() != guild_module::get_ghz_winner_chairman())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const int got_coin = mobai_module::wrc_info->left_coin_;
  if (player->is_money_upper_limit(M_COIN, got_coin))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);

  if (got_coin <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_OVER_GET);

  player->do_got_money(got_coin, M_COIN, MONEY_GOT_BE_WROSHIPED, 0);

  mobai_module::wrc_info->left_coin_ = 0;
  mobai_module::update_wrc_info();
  return player->send_respond_ok(RES_GET_WORSHIP_COIN);
}
