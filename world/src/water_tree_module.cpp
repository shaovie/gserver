#include "water_tree_module.h"
#include "water_tree_cfg.h"
#include "water_tree_info.h"
#include "lvl_param_cfg.h"
#include "global_param_cfg.h"
#include "time_util.h"
#include "istream.h"
#include "player_obj.h"
#include "db_proxy.h"
#include "sys_log.h"
#include "package_module.h"
#include "behavior_id.h"
#include "mail_config.h"
#include "mail_module.h"
#include "mail_info.h"
#include "task_module.h"
#include "cheng_jiu_module.h"
#include "huo_yue_du_module.h"
#include "goods_award_module.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("water_tree");
static ilog_obj *e_log = err_log::instance()->get_ilog("water_tree");

water_tree_info water_tree_module::wti;

void water_tree_module::on_enter_game(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << water_tree_module::wti.lvl_ << water_tree_module::wti.cheng_zhang_;
  player->send_request(NTF_WATER_TREE_INFO, &os);
}
int water_tree_module::dispatch_msg(player_obj *player,
                                    const int msg_id,
                                    const char *msg,
                                    const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = water_tree_module::FUNC(player, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GET_WATER_TIME,  clt_get_water_time);
    SHORT_CODE(REQ_WATER_TREE,      clt_water_tree);

    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle water_tree msg %d return %d", msg_id, ret);
  return ret;
}
int water_tree_module::clt_get_water_time(player_obj *player, const char *, const int )
{
  int time = global_param_cfg::guangai_cooling_time - (time_util::now - player->water_tree_time());
  out_stream os(client::send_buf, client::send_buf_len);
  os << (time > 0 ? time : 0);
  return player->send_respond_ok(RES_GET_WATER_TIME, &os);
}
int water_tree_module::clt_water_tree(player_obj *player, const char *, const int )
{
  water_tree_obj *wto = water_tree_cfg::instance()->get_water_tree_obj(water_tree_module::wti.lvl_);
  if (wto == NULL)
    return player->send_respond_err(RES_WATER_TREE, ERR_CONFIG_NOT_EXIST);
  if (water_tree_module::wti.lvl_ >= global_param_cfg::guangai_lvl_limit)
    return player->send_respond_err(RES_WATER_TREE, ERR_CLIENT_OPERATE_ILLEGAL);
  if (time_util::now - player->water_tree_time() < global_param_cfg::guangai_cooling_time)
    return player->send_respond_err(RES_WATER_TREE, ERR_AWARD_NO_REACH_TIME);
  int cai_liao_amt = package_module::calc_item_amount(player, wto->cai_liao_cid_);
  if (cai_liao_amt < wto->cai_liao_cnt_)
    return player->send_respond_err(RES_WATER_TREE, ERR_ITEM_AMOUNT_NOT_ENOUGH);

  package_module::do_remove_item(player,
                                 wto->cai_liao_cid_,
                                 wto->cai_liao_cnt_,
                                 ITEM_LOSE_WATER_TREE,
                                 0);
  player->water_tree_time(time_util::now);
  player->db_save_char_extra_info();

  int exp = int(lvl_param_cfg::instance()->guangai_exp(player->lvl()) * (wto->attr_param_ / 1000.0));
  if (exp > 0)
  {
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
    *mb << exp;
    player->post_aev(AEV_GOT_EXP, mb);
  }
  int coin = int(lvl_param_cfg::instance()->guangai_coin(player->lvl()) * (wto->attr_param_ / 1000.0));
  if (coin > 0)
    player->do_got_money(coin, M_COIN, MONEY_GOT_WATER_TREE, 0);

  water_tree_module::wti.cheng_zhang_++;
  if (water_tree_module::wti.cheng_zhang_ >= wto->exp_)
    water_tree_module::tree_level_up(player, wto);

  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid()
    << stream_ostr((char *)&water_tree_module::wti, sizeof(water_tree_info));
  db_proxy::instance()->send_request(0, REQ_UPDATE_WATER_TREE_INFO, &db_os);

  player->broadcast_water_tree_info();

  goods_award_module::on_guan_gai(player, wto);

  task_module::on_guan_gai(player);
  cheng_jiu_module::on_water_tree(player);
  huo_yue_du_module::on_water_tree(player);
  return player->send_respond_ok(RES_WATER_TREE);
}
void water_tree_module::tree_level_up(player_obj *player, water_tree_obj *wto)
{
  water_tree_module::wti.lvl_++;
  water_tree_module::wti.cheng_zhang_ = 0;

  const mail_obj *one_mo = mail_config::instance()->get_mail_obj(mail_config::WATER_TREE_LEVEL_UP_ONE);
  if (one_mo != NULL)
  {
    int size = 1;
    ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_NUMBER, water_tree_module::wti.lvl_);
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(one_mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   size,
                                   content);
    ilist<item_amount_bind_t> attach_list;
    attach_list.push_back(wto->role_reward_);
    mail_module::do_send_mail(player->id(), player->career(), player->db_sid(),
                              mail_info::MAIL_TYPE_PRIVATE,
                              one_mo->sender_name_, one_mo->title_, content,
                              attach_list);
    mail_module::do_notify_haved_new_mail(player, 1); 
  }

  const mail_obj *all_mo = mail_config::instance()->get_mail_obj(mail_config::WATER_TREE_LEVEL_UP_ALL);
  if (all_mo != NULL)
  {
    int size = 1;
    ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_NUMBER, water_tree_module::wti.lvl_);
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(all_mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   size,
                                   content);
    mail_module::do_send_mail_2_all(all_mo->sender_name_, all_mo->title_, content,
                                    0, 0, wto->bind_diamond_,
                                    "", 0);
  }
}
void player_obj::broadcast_water_tree_info()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << water_tree_module::wti.lvl_
    << water_tree_module::wti.cheng_zhang_;
  ph->set(0, NTF_WATER_TREE_INFO, 0, mb.length());
  this->do_broadcast(&mb, true);
}

