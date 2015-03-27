#include "dxc_module.h"
#include "player_obj.h"
#include "message.h"
#include "error.h"
#include "sys_log.h"
#include "clsid.h"
#include "dxc_config.h"
#include "vip_module.h"
#include "scp_module.h"
#include "scp_config.h"
#include "scp_obj.h"
#include "behavior_log.h"
#include "time_util.h"
#include "global_param_cfg.h"
#include "dxc_info.h"
#include "db_proxy.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("dxc");
static ilog_obj *e_log = err_log::instance()->get_ilog("dxc");

int dxc_module::dispatch_msg(player_obj *player,
                             const int msg_id,
                             const char *msg,
                             const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = dxc_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_ENTER_DIXIACHENG,         clt_enter);
    //SHORT_CODE(REQ_EXIT_DIXIACHENG,          clt_exit); //use scp_module::clt_exit_scp
    SHORT_CODE(REQ_FINISH_DIXIACHENG,        clt_finish);
    SHORT_CODE(REQ_GET_DIXIACHENG_INFO,      clt_get_info);
    //
    SHORT_DEFAULT;
  }
  return ret;
}

int dxc_module::clt_enter(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_world_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  in_stream is(msg, len);
  char type = -1;
  int scene_cid = 0;
  is >> type >> scene_cid;

  if (type <= 0 || type >= DXC_END)
    return player->send_respond(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const dxc_cfg_obj *dco = dxc_config::instance()->get_dxc_cfg_obj(scene_cid);
  if (dco == NULL)
    return player->send_respond(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int vip_add = vip_config::instance()->effect_val1(vip_module::vip_lvl(player),
                                                    VIP_EFF_DXC_ENTER_ADD);
  if (player->dxc_enter_cnt() >= global_param_cfg::dxc_enter_cnt + vip_add)
    return player->send_respond(NTF_OPERATE_RESULT, ERR_DXC_NORMAL_ENTER_CNT_OUT_LIMIT);

  dxc_info *cur_info = NULL, *prev_info = NULL;
  ilist_node<dxc_info *> *iter = player->dxc_info_list_.head();
  for (; iter != NULL && cur_info == NULL; iter = iter->next_)
  {
    if (iter->value_->scene_cid_ == scene_cid)
      cur_info = iter->value_;
    else if (iter->value_->scene_cid_ + 1 == scene_cid)
      prev_info = iter->value_;
  }
  if (cur_info != NULL)
  {
    if (cur_info->grade_ + 1 < type)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }else if (prev_info != NULL)
  {
    if (prev_info->grade_ == 0 || type != DXC_NORMAL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }else if (scene_cid != DXC_FIRST_SCENE || type != DXC_NORMAL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (player->zhan_li() < dco->zhanli_limit_[int(type)])
    return player->send_respond(NTF_OPERATE_RESULT, ERR_ZHANLI_NOT_ENOUGH);

  int ret = player->can_transfer_to(scene_cid);
  if (ret != 0)
    return player->send_respond(NTF_OPERATE_RESULT, ret);

  ret = dxc_module::do_enter_dxc_scp(player, scene_cid);
  if (ret != 0)
    return player->send_respond(NTF_OPERATE_RESULT, ret);
  return 0;
}
int dxc_module::clt_exit(player_obj *player, const char *, const int )
{
  if (!clsid::is_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player->do_exit_scp();
  return 0;
}
int dxc_module::clt_finish(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  in_stream is(msg, len);
  char type = -1;
  is >> type;

  ilist_node<dxc_info *> *iter = player->dxc_info_list_.head();
  for (; iter != NULL; iter = iter->next_)
  {
    if (iter->value_->scene_cid_ == player->scene_cid())
      break;
  }
  if (iter == NULL)
  {
    dxc_info *info = new dxc_info();
    info->char_id_ = player->id();
    info->scene_cid_ = player->scene_cid();
    info->grade_   = DXC_NORMAL;
    player->dxc_info_list_.push_back(info);

    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << stream_ostr((const char*)info, sizeof(dxc_info));
    db_proxy::instance()->send_request(player->id(), REQ_INSERT_DXC_INFO, &os);
  }else if (iter->value_->grade_ + 1 == type
            && type < DXC_END)
  {
    iter->value_->grade_ = type;

    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << stream_ostr((const char*)iter->value_, sizeof(dxc_info));
    db_proxy::instance()->send_request(player->id(), REQ_UPDATE_DXC_INFO, &os);
  }

  return 0;
}
int dxc_module::clt_get_info(player_obj *player, const char *, const int)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->dxc_enter_cnt() << player->dxc_info_list_.size();
  for (ilist_node<dxc_info *> *iter = player->dxc_info_list_.head();
       iter != NULL;
       iter = iter->next_)
    os << iter->value_->scene_cid_ << iter->value_->grade_;
  return player->send_respond_ok(RES_GET_DIXIACHENG_INFO, &os);
}

int dxc_module::do_enter_dxc_scp(player_obj *player,
                                 const int scene_cid)
{
  scp_obj *so = scp_module::construct_scp(scene_cid, player->id(), 0, 0);
  if (so == NULL || so->open() != 0)
  {
    delete so;
    return ERR_DXC_SCP_CREATE_FAILED;
  }

  coord_t enter_ct = scp_config::instance()->get_enter_pos(scene_cid);
  if (enter_ct.x_ == -1)
    return ERR_COORD_IS_ILLEGAL;
  int ret = player->do_transfer_to(scene_cid,
                                   so->first_scene_id(),
                                   enter_ct.x_,
                                   enter_ct.y_);
  if (ret != 0) return ret;
  so->enter_scp(player->id(), player->scene_id());

  player->dxc_enter_cnt(player->dxc_enter_cnt() + 1);
  player->db_save_daily_clean_info();
  
  behavior_log::instance()->store(BL_ENTER_SCP,
                                  time_util::now,
                                  "%d|%d",
                                  player->id(), scene_cid);
  return 0;
}

void dxc_module::handle_db_get_dxc_info_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(dxc_info) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    dxc_info *info = (dxc_info *)bf;

    dxc_info *new_info = new dxc_info();
    *new_info = *info;
    player->dxc_info_list_.push_back(new_info);
  }
}
void dxc_module::on_char_logout(player_obj *player)
{
  while (!player->dxc_info_list_.empty())
    delete player->dxc_info_list_.pop_front();
}
