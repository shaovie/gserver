#include "scp_module.h"
#include "player_obj.h"
#include "db_proxy.h"
#include "sub_scp_obj.h"
#include "scp_obj.h"
#include "scp_log.h"
#include "scp_mgr.h"
#include "clsid.h"
#include "global_param_cfg.h"
#include "sys_log.h"
#include "guild_module.h"
#include "scene_config.h"
#include "player_mgr.h"
#include "scp_config.h"
#include "team_mgr.h"
#include "huo_yue_du_module.h"
#include "cheng_jiu_module.h"
#include "behavior_log.h"
#include "time_util.h"
#include "clt_impl_scp.h"
#include "guild_zhu_di_scp.h"
#include "jing_ji_scp.h"
#include "lue_duo_scp.h"
#include "guild_scp.h"
#include "xszc_scp.h"
#include "mblock_pool.h"
#include "package_module.h"
#include "mail_config.h"

// Lib header
#include <assert.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

int scp_module::handle_db_get_scp_log_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(scp_log) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    scp_log *info = (scp_log *)bf;

    if (scp_config::instance()->get_scp_cfg_obj(info->cid_) == NULL)
      continue;
    scp_log *new_info = new scp_log();
    ::memcpy(new_info, info, sizeof(scp_log));
    player->scp_log_.push_back(new_info);
  }
  return 0;
}
void scp_module::destroy(player_obj *player)
{
  while (!player->scp_log_.empty())
    delete player->scp_log_.pop_front();
}
void scp_module::db_insert_scp_log(player_obj *player, scp_log *sl)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr((const char *)sl, sizeof(scp_log));
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_SCP_LOG, &os);
}
void scp_module::db_update_scp_log(player_obj *player, scp_log *sl)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr((const char *)sl, sizeof(scp_log));
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_SCP_LOG, &os);
}
scp_log *scp_module::find_scp_log(player_obj *player, const int scene_cid)
{
  for (ilist_node<scp_log *> *itor = player->scp_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == scene_cid)
      return itor->value_;
  }
  return NULL;
}
scp_obj *scp_module::construct_scp(const int scene_cid,
                                   const int char_id,
                                   const int /*team_id*/,
                                   const int guild_id)
{
  scp_obj *so = NULL;
  if (scene_cid == global_param_cfg::guild_zhu_di_cid)
    so = new guild_zhu_di_scp(scene_cid, guild_id);
  else if (scene_cid == global_param_cfg::jing_ji_map)
    so = new jing_ji_scp(scene_cid, char_id);
  else if (scene_cid == global_param_cfg::lue_duo_map)
    so = new lue_duo_scp(scene_cid, char_id);
  else if (scp_config::instance()->scp_type(scene_cid) == scp_cfg_obj::SCP_SINGLE
           || clsid::is_tui_tu_scp_scene(scene_cid))
    so = new clt_impl_scp(scene_cid, char_id);
  else if (scene_cid == global_param_cfg::guild_scp_cid)
    so = new guild_scp(scene_cid, guild_id);
  else if (scene_cid == global_param_cfg::battle_scene_cid)
    so = new xszc_scp_obj(scene_cid);
  else
    e_log->rinfo("unknow scp cid %d", scene_cid);
  return so;
}
sub_scp_obj *scp_module::construct_sub_scp(const int /*scene_cid*/)
{
  sub_scp_obj *sso = NULL;
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return sso;
}
void scp_module::on_char_login(player_obj *player)
{
  if (!clsid::is_scp_scene(player->scene_cid())) return ;
  player->do_resume_last_position();
  if (BIT_ENABLED(player->unit_status(), OBJ_DEAD))
  {
    player->del_status(OBJ_DEAD);
    int relive_energy_per = global_param_cfg::relive_energy_p;
    player->do_add_hp(player->total_hp() * relive_energy_per / 100);
    player->do_add_mp(player->total_mp() * relive_energy_per / 100);
  }
}
void scp_module::on_new_day(player_obj *player, const bool when_login)
{
  bool to_clear = false;
  for (ilist_node<scp_log *> *itor = player->scp_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->enter_cnt_ > 0)
    {
      itor->value_->enter_cnt_ = 0;
      itor->value_->enter_time_ = 0;
      to_clear = true;
    }
  }

  if (to_clear)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << player->id();
    db_proxy::instance()->send_request(player->id(), REQ_CLEAR_SCP_LOG, &os);

    if (!when_login)
      scp_module::do_push_all_scp_list(player);
  }
}
void scp_module::on_enter_game(player_obj *player)
{
  scp_module::do_push_all_scp_list(player);
}
void scp_module::do_push_all_scp_list(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  char *cnt = mb.wr_ptr();
  *cnt = 0;
  mb << *cnt;
  for (ilist_node<scp_log *> *itor = player->scp_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    mb << itor->value_->cid_ << itor->value_->enter_cnt_ << itor->value_->enter_time_;
    ++(*cnt);
  }

  player->do_delivery(NTF_ALL_SCP_LIST, &mb);
}
void scp_module::on_char_logout(player_obj *player)
{
  if (clsid::is_scp_scene(player->scene_cid()))
    scp_mgr::instance()->exit_scp(player->scene_id(), player->id(), scp_obj::CHAR_INITIATIVE_EXIT);
}
void scp_module::on_transfer_scene(player_obj *player,
                                   const int from_scene_id,
                                   const int from_scene_cid,
                                   const int /*to_scene_id*/,
                                   const int to_scene_cid)
{
  if (from_scene_cid == to_scene_cid) return ;
  if (!clsid::is_scp_scene(from_scene_cid)) return ;
  if (!clsid::is_scp_scene(to_scene_cid))
    scp_mgr::instance()->exit_scp(from_scene_id, player->id(), scp_obj::CHAR_INITIATIVE_EXIT);
}
void scp_module::on_char_dead(player_obj *player, const int killer_id)
{
  if (!clsid::is_scp_scene(player->scene_cid())) return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_CHAR_DEAD);
  mb << player->id() << killer_id;
  scp_mgr::instance()->do_something(player->scene_id(), &mb, NULL, NULL);
}
void scp_module::on_kill_somebody(player_obj *player, const int char_id)
{
  if (!clsid::is_scp_scene(player->scene_cid())) return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_KILL_SOMEBODY);
  mb << player->id() << char_id;
  scp_mgr::instance()->do_something(player->scene_id(), &mb, NULL, NULL);
}
int scp_module::can_enter_scp(player_obj *player, const int scene_cid)
{
  if (player->scene_id() != player->scene_cid()) // in scp
    return ERR_CLIENT_OPERATE_ILLEGAL;

  scp_cfg_obj *sco = scp_config::instance()->get_scp_cfg_obj(scene_cid);
  if (sco == NULL) return ERR_CONFIG_NOT_EXIST;
  if (!sco->is_valid_) return ERR_SCP_IS_INVALID;

  if (sco->scp_type_ == scp_cfg_obj::SCP_TEAM)
  {
    if (player->team_id() == 0) return ERR_SELF_NOT_HAVE_TEAM;
    if (!team_mgr::instance()->is_leader(player->team_id(), player->id()))
      return ERR_SCP_NOT_LEADER;
  }

  scp_log *sl = scp_module::find_scp_log(player, scene_cid);
  if (sl != NULL
      && sl->enter_cnt_ >= sco->enter_cnt_)
    return ERR_SCP_ENTER_CNT_OUT_LIMIT;

  if (scp_mgr::instance()->get_scp_by_char_owner(player->id()) != NULL)
    return ERR_CAN_NOT_ENTER_SCP;
  return player->can_transfer_to(scene_cid);
}
void player_obj::do_exit_scp()
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
  {
    this->del_status(OBJ_DEAD);
    int relive_energy_per = global_param_cfg::relive_energy_p;
    this->do_add_hp(this->obj_attr_.total_hp() * relive_energy_per / 100);
    this->do_add_mp(this->obj_attr_.total_mp() * relive_energy_per / 100);
    this->broadcast_hp_mp();
  }
  this->do_transfer_to(this->char_info_->last_scene_cid_,
                       this->char_info_->last_scene_cid_,
                       this->char_info_->last_coord_x_,
                       this->char_info_->last_coord_y_);
}
int player_obj::do_enter_scp(const int scene_id, const int scene_cid)
{
  sub_scp_obj *sso = scp_mgr::instance()->get_sub_scp_obj(scene_id);
  if (sso == NULL
      || sso->master_scp() == NULL)
    return ERR_SCP_NOT_OPENED;

  coord_t enter_pos = scp_config::instance()->get_enter_pos(scene_cid);
  if (enter_pos.x_ == -1) return ERR_CONFIG_NOT_EXIST;
  int ret = this->do_transfer_to(scene_cid,
                                 scene_id,
                                 enter_pos.x_,
                                 enter_pos.y_);
  if (ret != 0) return ret;

  sso->master_scp()->enter_scp(this->id(), scene_id);

  this->enter_scp_time_ = time_util::now;

  behavior_log::instance()->store(BL_ENTER_SCP,
                                  time_util::now,
                                  "%d|%d",
                                  this->id_, scene_cid);
  return 0;
}
int scp_module::do_enter_single_scp(player_obj *player, const int scene_cid)
{
  scp_obj *so = scp_module::construct_scp(scene_cid,
                                          player->id(),
                                          0,
                                          0);
  if (so == NULL || so->open() != 0)
  {
    delete so;
    return ERR_SCP_CREATE_FAILED;
  }
  return player->do_enter_scp(so->first_scene_id(), scene_cid);
}
int scp_module::do_enter_team_scp(player_obj *player, const int scene_cid)
{
  scp_obj *so = scp_module::construct_scp(scene_cid,
                                          0,
                                          player->team_id(),
                                          0);
  if (so == NULL || so->open() != 0)
  {
    delete so;
    return ERR_SCP_CREATE_FAILED;
  }
  int valid_team_member[MAX_TEAM_MEMBER];
  valid_team_member[0] = player->id();
  int valid_team_member_cnt = 1;

  team_info *ti = team_mgr::instance()->find(player->team_id());
  if (ti != NULL)
  {
    for (int i = 1; i < MAX_TEAM_MEMBER; ++i)
    {
      if (ti->mem_id_[i] != 0
          && ti->mem_id_[i] != player->id())
      {
        if (player->is_in_my_view(ti->mem_id_[i]))
          valid_team_member[valid_team_member_cnt++] = ti->mem_id_[i];
      }
    }
  }
  for (int i = 0; i < valid_team_member_cnt; ++i)
  {
    player_obj *member = player_mgr::instance()->find(valid_team_member[i]);
    if (member != NULL)
      member->do_enter_scp(so->first_scene_id(), scene_cid);
  }
  return 0;
}
void player_obj::do_scp_end()
{
  int scene_cid = this->scene_cid();
  // save scp enter log
  if (scene_cid != global_param_cfg::jing_ji_map
      && scene_cid != global_param_cfg::guild_scp_cid
      && scene_cid != global_param_cfg::battle_scene_cid
      && scene_cid != global_param_cfg::lue_duo_map)
  {
    scp_log *sl = scp_module::find_scp_log(this, scene_cid);
    if (sl == NULL)
    {
      sl = new scp_log();
      sl->char_id_ = this->id_;
      sl->cid_ = scene_cid;
      sl->enter_cnt_ = 1;
      sl->enter_time_ = this->enter_scp_time_;
      this->scp_log_.push_back(sl);
      scp_module::db_insert_scp_log(this, sl);
    }else
    {
      sl->enter_cnt_ += 1;
      sl->enter_time_ = this->enter_scp_time_;
      scp_module::db_update_scp_log(this, sl);
    }
    out_stream os(client::send_buf, client::send_buf_len);
    os << scene_cid << sl->enter_cnt_ << sl->enter_time_;
    this->send_request(NTF_SINGLE_SCP_INFO, &os);
  }

  huo_yue_du_module::on_enter_scp(this, scene_cid);
  cheng_jiu_module::on_enter_scp(this, scene_cid);

  return ;
}
void player_obj::do_clear_scp_award()
{
  this->scp_award_exp_ = 0;
  this->scp_award_items_.clear();
}
void player_obj::do_give_scp_award()
{
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
  *mb << this->scp_award_exp_;
  this->post_aev(AEV_GOT_EXP, mb);

  ilist_node<item_amount_bind_t> *itor = this->scp_award_items_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    package_module::do_insert_or_mail_item(this,
                                           itor->value_.cid_,
                                           itor->value_.amount_,
                                           itor->value_.bind_,
                                           mail_config::TUI_TU_MST_DROP,
                                           MONEY_GOT_PICK_UP,
                                           ITEM_GOT_PICKUP,
                                           this->scene_cid_,
                                           0);
  }
}
void player_obj::do_notify_scp_award_to_clt()
{
  if (this->scp_award_exp_ == 0
      && this->scp_award_items_.empty())
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->scp_award_exp_
    << (char)this->scp_award_items_.size();
  ilist_node<item_amount_bind_t> *itor = this->scp_award_items_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    mb << itor->value_.cid_
      << itor->value_.amount_;
  }
  this->do_delivery(NTF_NOTIFY_SCP_AWARD, &mb);
}
int scp_module::dispatch_msg(player_obj *player,
                             const int msg_id,
                             const char *msg,
                             const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = scp_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_ENTER_SCP,                clt_enter_scp);
    SHORT_CODE(REQ_EXIT_SCP,                 clt_exit_scp);
    SHORT_CODE(REQ_SCP_END,                  clt_scp_end);
    SHORT_CODE(REQ_IN_SCP_KILL_MONSTER,      clt_in_scp_kill_mst);
    SHORT_CODE(REQ_IN_SCP_RELIVE,            clt_in_scp_relive);
    SHORT_CODE(REQ_MUSHROOM_SCP_END,         clt_mushroom_scp_end);
    SHORT_CODE(REQ_GET_TOTAL_MSTAR,          clt_get_total_mstar);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int scp_module::clt_enter_scp(player_obj *player, const char *msg, const int len)
{
  if (clsid::is_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  in_stream is(msg, len);
  int scene_cid = 0;
  is >> scene_cid;

  scp_cfg_obj *sco = scp_config::instance()->get_scp_cfg_obj(scene_cid);
  if (sco == NULL)
    return player->send_respond(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int ret = scp_module::can_enter_scp(player, scene_cid);
  if (ret != 0)
    return player->send_respond(NTF_OPERATE_RESULT, ret);

  if (sco->scp_type_ == scp_cfg_obj::SCP_SINGLE)
  {
    player->do_clear_scp_award();
    ret = scp_module::do_enter_single_scp(player, scene_cid);
  }else if (sco->scp_type_ == scp_cfg_obj::SCP_TEAM)
  {
    ret = scp_module::do_enter_team_scp(player, scene_cid);
  }

  if (ret != 0)
    return player->send_respond(NTF_OPERATE_RESULT, ret);
  return 0;
}
int scp_module::clt_exit_scp(player_obj *player, const char *, const int )
{
  if (!clsid::is_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player->do_exit_scp();
  return 0;
}
int scp_module::clt_in_scp_kill_mst(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_scp_scene(player->scene_cid())
      || scp_config::instance()->control_type(player->scene_cid()) != scp_cfg_obj::SCP_CLT_IMPL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int mst_cid = 0;
  short x = 0;
  short y = 0;
  in_stream is(msg, len);
  is >> mst_cid >> x >> y;

  bool got_award = false;
  player->on_clt_kill_mst(mst_cid, x, y, NTF_IN_SCP_KILL_MST_AWARD, got_award);
  if (got_award)
    player->do_notify_scp_award_to_clt();
  return 0;
}
int scp_module::clt_in_scp_relive(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  char relive_mode = 0;
  in_stream is(msg, len);
  is >> relive_mode;

  if (relive_mode != 2) // 原地
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  int ret = player->do_relive(relive_mode);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int scp_module::clt_scp_end(player_obj *player, const char *, const int )
{
  if (!clsid::is_scp_scene(player->scene_cid())
      || scp_config::instance()->control_type(player->scene_cid()) != scp_cfg_obj::SCP_CLT_IMPL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player->do_scp_end();

  player->do_give_scp_award();
  return 0;
}
int scp_module::clt_mushroom_scp_end(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_scp_scene(player->scene_cid())
      || scp_config::instance()->control_type(player->scene_cid()) != scp_cfg_obj::SCP_CLT_IMPL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player->do_scp_end();

  int score = 0;
  in_stream is(msg, len);
  is >> score;
  int coin = (int)::ceil((double)score * (double)player->zhan_li() * (double)global_param_cfg::mushroom_coin_coe / 10000.0);
  player->do_got_money(coin, M_COIN, MONEY_GOT_SCP, player->scene_cid());
  return 0;
}
int scp_module::clt_get_total_mstar(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->char_extra_info_->total_mstar_;
  return player->send_respond_ok(RES_GET_TOTAL_MSTAR, &os);
}
