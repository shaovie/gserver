#include "player_obj.h"
#include "all_char_info.h"
#include "account_info.h"
#include "global_param_cfg.h"
#include "daily_clean_info.h"
#include "skill_module.h"
#include "package_module.h"
#include "forbid_opt_module.h"
#include "attr_module.h"
#include "char_info.h"
#include "db_proxy.h"
#include "player_mgr.h"
#include "time_util.h"
#include "scene_config.h"
#include "svc_config.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "error.h"
#include "clsid.h"
#include "util.h"
#include "sys.h"
#include "guild_module.h"
#include "name_storage_cfg.h"
#include "rank_module.h"
#include "bao_shi_module.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

int player_obj::do_login_module_msg(const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:       \
  ret = this->FUNC(msg, len);               \
  break
#define SHORT_DEFAULT default:              \
  e_log->error("unknow msg id %d", msg_id); \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_HELLO_WORLD,   clt_hello_world);
    SHORT_CODE(REQ_CREATE_CHAR,   clt_create_char);
    SHORT_CODE(REQ_START_GAME,    clt_start_game);
    SHORT_CODE(REQ_ENTER_GAME,    clt_enter_game);
    SHORT_CODE(REQ_REENTER_GAME,  clt_reenter_game);
    SHORT_CODE(REQ_GET_HOT_NAME,  clt_get_hot_name);
    SHORT_DEFAULT;
  }
  return ret;
}
int player_obj::clt_hello_world(const char *msg, const int len)
{
  char channel[MAX_CHANNEL_LEN + 1] = {0};
  in_stream is(msg, len);
  stream_istr ac_si(this->account_, sizeof(this->account_));
  stream_istr channel_si(channel, sizeof(channel));
  is >> ac_si >> channel_si;

  util::to_lower(this->account_);

  if (this->clt_state_ != CLT_NULL) return ERR_CLIENT_STATE_ERROR;
  s_log->rinfo("account [%s - %s] want login! h:%d db_sid:%d",
               this->account(), channel, this->get_handle(), this->db_sid_);

  if (ac_si.str_len() < 3
      || !util::verify_account(this->account_)
      || !util::verify_account(channel))
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_LOGIN_AC_IS_ILLEGAL);
    return ERR_LOGIN_AC_IS_ILLEGAL;
  }

  if (!forbid_opt_module::can_login_for_account(this->account()))
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_FORBID_LOGIN);
    return ERR_FORBID_LOGIN;
  }

  // repead login
  {
    player_obj *po = player_mgr::instance()->find(this->account());
    if (po != NULL)
      po->do_kick_out();
  }

  player_mgr::instance()->insert(this->account(), this);

  s_log->rinfo("account [%s][%p] login!", this->account(), this);

  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL)
  {
    int ret = this->to_create_account(this->account(), channel);
    if (ret != 0)
      this->send_respond_err(NTF_PLAYER_LOGOUT, ret);
    return ret;
  }else
    this->on_get_account_info_ok(abi);
  return 0;
}
int player_obj::to_create_account(const char *account, const char *channel)
{
  out_stream ac_os(client::send_buf, client::send_buf_len);
  account_info ac_info;
  ::strncpy(ac_info.account_, account, sizeof(ac_info.account_) - 1);
  ::strncpy(ac_info.channel_, channel, sizeof(ac_info.channel_) - 1);
  ac_info.c_time_ = time_util::now;
  stream_ostr ac_so((const char *)&ac_info, sizeof(account_info));
  ac_os << this->db_sid_ << ac_so;
  if (db_proxy::instance()->send_request(::abs(this->account_[0]),
                                         REQ_INSERT_ACCOUNT_INFO,
                                         &ac_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;

  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr so(account, ::strlen(account));
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(::abs(this->account_[0]),
                                         REQ_GET_ACCOUNT_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  this->clt_state_ = CLT_TO_CREATE_ACCOUNT;
  return 0;
}
void player_obj::handle_db_get_account_info_result(account_info *ai)
{
  if (ai == NULL)
  {
    e_log->error("got account %s info is null!", this->account());
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }
  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL)
  {
    abi = new account_brief_info();
    all_char_info::instance()->insert_account_info(this->account(), abi);
  }
  *(abi->ac_info_) = *ai;
  this->on_get_account_info_ok(abi);
}
void player_obj::on_get_account_info_ok(account_brief_info *abi)
{
  this->ac_info_ = abi->ac_info_;
  this->clt_state_ = CLT_GET_ACCOUNT_OK;

  if (abi->char_list_.size() > 0)
    this->clt_state_ = CLT_GET_CHAR_OK;
  this->do_send_client_char_list();

  if (abi->char_list_.size() < 3)
    this->do_ntf_clt_hot_name();
}
int player_obj::do_kick_out()
{
  s_log->rinfo("kick out %s %p", this->account(), this);
  this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_LOGIN_REPEADED);
  this->shutdown();
  return 0;
}
int player_obj::do_send_client_char_list()
{
  account_brief_info *abi
    = all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL) return -1;

  s_log->rinfo("account [%s] has %d chars!", this->account(), abi->char_list_.size());
  out_stream os(client::send_buf, client::send_buf_len);
  os << sys::assign_career() << this->ac_info_->char_id_ << abi->char_list_.size();
  for (ilist_node<char_brief_info *> *itor = abi->char_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    char_brief_info *cbi = itor->value_;
    this->do_build_char_login_show_info(os, cbi);
  }
  return this->send_respond_ok(RES_HELLO_WORLD, &os);
}
int player_obj::do_ntf_clt_hot_name()
{
  out_stream os(client::send_buf, client::send_buf_len);
  std::string new_name = name_storage_cfg::instance()->get_name();
  os << stream_ostr(new_name.c_str(), new_name.length());
  return this->send_request(NTF_CLT_HOT_NAME, &os);
}
void player_obj::do_build_char_login_show_info(out_stream &os, char_brief_info *cbi)
{
  os << cbi->char_id_
    << stream_ostr(cbi->name_, ::strlen(cbi->name_))
    << cbi->career_
    << cbi->lvl_
    << cbi->zhu_wu_cid_ << cbi->fu_wu_cid_
    << cbi->scene_cid_;
}
int player_obj::clt_create_char(const char *msg, const int len)
{
  if (this->clt_state_ != CLT_GET_ACCOUNT_OK
      && this->clt_state_ != CLT_GET_CHAR_OK)
    return ERR_CLIENT_STATE_ERROR;

  char career = 0;
  char name_bf[MAX_NAME_LEN + 1] = {0};
  stream_istr name_si(name_bf, sizeof(name_bf));

  in_stream is(msg, len);
  is >> name_si >> career;

  if (name_si.str_len() < 2
      || !util::is_career_ok(career)
      || name_si.str_len() > MAX_PURE_NAME_LEN
      || ::strcmp("null", name_si.str()) == 0
      || !util::verify_name(name_si.str()))
    return this->send_respond_err(RES_CREATE_CHAR,
                                  ERR_INPUT_IS_ILLEGAL);

  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL
      || abi->char_list_.size() >= 3)
    return this->send_respond_err(RES_CREATE_CHAR, ERR_CLIENT_OPERATE_ILLEGAL);
  if (all_char_info::instance()->get_char_brief_info(name_bf) != NULL)
    return this->send_respond_err(RES_CREATE_CHAR, ERR_CHAR_NAME_EXIST);

  int ret = this->to_create_char(name_bf, career);
  if (ret != 0)
    return this->send_respond_err(RES_CREATE_CHAR, ret);
  return 0;
}
int player_obj::to_create_char(const char *name, const char career)
{
  // init char_info
  char_info cinfo;
  ::strncpy(cinfo.name_, name, sizeof(cinfo.name_) - 1);
  ::strncpy(cinfo.account_, this->account(), sizeof(cinfo.account_) - 1);
  cinfo.career_ = career;
  cinfo.char_id_ = sys::assign_char_id();
  if (cinfo.char_id_  == -1)
  {
    e_log->error("assign char id failed! cur max char id = %d", sys::last_char_id);
    return ERR_CAN_NOT_CREATE_CHAR;
  }

  int ret = this->do_init_daily_clean_info(&cinfo);
  if (ret != 0) return ret;

  ret = this->do_init_char_extra_info(&cinfo);
  if (ret != 0) return ret;

  this->do_init_birth_equip(&cinfo);

  // 最后再创建角色，万一前面失败了也不影响再次创建角色
  ret = this->do_init_char_info(&cinfo);
  if (ret != 0) return ret;

  // 从数据库中获取新创建的角色，以保证绝对创建成功
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << cinfo.char_id_;
  if (db_proxy::instance()->send_request(cinfo.char_id_,
                                         REQ_GET_NEW_CHAR_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  this->clt_state_ = CLT_TO_CREATE_CHAR;
  return 0;
}
int player_obj::do_init_char_info(char_info *cinfo)
{
  cinfo->c_time_ = time_util::now;
  cinfo->scene_id_  = global_param_cfg::birth_coord.cid_;
  cinfo->scene_cid_ = global_param_cfg::birth_coord.cid_;
  cinfo->coord_x_   = global_param_cfg::birth_coord.coord_.x_;
  cinfo->coord_y_   = global_param_cfg::birth_coord.coord_.y_;
  cinfo->last_scene_cid_ = cinfo->scene_cid_;
  cinfo->last_coord_x_   = cinfo->coord_x_;
  cinfo->last_coord_y_   = cinfo->coord_y_;
  cinfo->lvl_ = 1;

  cinfo->pkg_cap_   = global_param_cfg::pkg_cap;
  cinfo->storage_cap_ = global_param_cfg::storage_cap;
  char bf[32] = {0};
  this->remote_addr_.get_host_addr(bf, sizeof(bf));
  ::strncpy(cinfo->ip_, bf, sizeof(cinfo->ip_) - 1);

  stream_ostr char_so((const char *)cinfo, sizeof(char_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << this->db_sid_ << char_so;
  if (db_proxy::instance()->send_request(cinfo->char_id_,
                                         REQ_INSERT_CHAR_INFO,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::do_init_char_extra_info(char_info *cinfo)
{
  char_extra_info v;
  v.char_id_ = cinfo->char_id_;
  v.ti_li_   = global_param_cfg::tili_val;
  v.ti_li_time_ = time_util::now;
  stream_ostr v_so((const char *)&v, sizeof(char_extra_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << this->db_sid_ << v_so;
  if (db_proxy::instance()->send_request(cinfo->char_id_,
                                         REQ_INSERT_CHAR_EXTRA_INFO,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::do_init_daily_clean_info(char_info *cinfo)
{
  daily_clean_info v;
  v.char_id_ = cinfo->char_id_;
  v.clean_time_ = time_util::now;
  v.ol_award_start_time_ = time_util::now;

  stream_ostr v_so((const char *)&v, sizeof(daily_clean_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << this->db_sid_ << v_so;
  if (db_proxy::instance()->send_request(cinfo->char_id_,
                                         REQ_INSERT_DAILY_CLEAN_INFO,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
void player_obj::do_init_birth_equip(char_info *ci)
{
  int item_id = 1;
  for (ilist_node<int> *itor = global_param_cfg::birth_equip_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    int equip_cid = clsid::get_equip_cid_by_career(ci->career_, itor->value_);
    if (item_config::instance()->get_equip(equip_cid) == NULL)
      continue;
    item_obj *equip_obj = package_module::alloc_new_item(ci->char_id_,
                                                         equip_cid,
                                                         1,
                                                         BIND_TYPE);
    equip_obj->pkg_ = PKG_EQUIP;
    equip_obj->id_  = item_id++;
    out_stream os(client::send_buf, client::send_buf_len);
    os << this->db_sid() << equip_obj;
    db_proxy::instance()->send_request(ci->char_id_, REQ_INSERT_ITEM, &os);
    package_module::release_item(equip_obj);
  }
}
void player_obj::handle_db_get_new_char_info_result(char_info *cinfo)
{
  if (cinfo == NULL)
  {
    e_log->error("got %s new char info is null!", this->account());
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }
  char_brief_info *p_cbi = new char_brief_info();
  ::strncpy(p_cbi->name_, cinfo->name_, sizeof(p_cbi->name_) - 1);
  p_cbi->char_id_     = cinfo->char_id_;
  p_cbi->career_      = cinfo->career_;
  p_cbi->lvl_         = cinfo->lvl_;
  p_cbi->scene_cid_   = global_param_cfg::birth_mission_cid;
  all_char_info::instance()->insert_char_info(p_cbi);
  sys::select_career(cinfo->career_);
  name_storage_cfg::instance()->on_create_char(p_cbi->name_);

  rank_module::on_char_get_exp(cinfo->char_id_, cinfo->lvl_, cinfo->exp_,
                               cinfo->lvl_, cinfo->exp_);

  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  char_brief_info *cbi =
    all_char_info::instance()->get_char_brief_info(cinfo->char_id_);
  if (abi == NULL || cbi == NULL)
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }
  abi->char_list_.push_back(cbi);

  // save equip show info
  for (ilist_node<int> *itor = global_param_cfg::birth_equip_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    int equip_cid = clsid::get_equip_cid_by_career(cinfo->career_, itor->value_);
    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_cid);
    if (eco == NULL) continue;
    if (eco->part_ == PART_ZHU_WU)
      cbi->zhu_wu_cid_ = equip_cid;
    else if (eco->part_ == PART_FU_WU)
      cbi->fu_wu_cid_  = equip_cid;
  }

  this->clt_state_ = CLT_GET_CHAR_OK;

  out_stream os(client::send_buf, client::send_buf_len);
  this->do_build_char_login_show_info(os, cbi);
  this->send_respond_ok(RES_CREATE_CHAR, &os);

  if (this->ac_info_->track_st_ == account_info::TK_CREATE_AC_OK)
  {
    this->ac_info_->track_st_ = account_info::TK_CREATE_CHAR_OK;
    this->db_save_account_info();
  }
}
int player_obj::clt_start_game(const char *msg, const int len)
{
  if (this->clt_state_ != CLT_GET_CHAR_OK)
    return ERR_CLIENT_STATE_ERROR;

  if (player_mgr::instance()->size() >= svc_config::instance()->max_online())
    return this->send_respond_err(RES_START_GAME, ERR_OVER_MAX_PAYLOAD);

  int char_id = 0;
  char has_resource = 1;
  in_stream is(msg, len);
  is >> char_id >> has_resource;
  if (has_resource == 0)
    this->has_resource_ = false;

  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL
      || !abi->is_char_ok(char_id))
  {
    this->send_respond_err(RES_START_GAME, ERR_CLIENT_OPERATE_ILLEGAL);
    return ERR_CLIENT_OPERATE_ILLEGAL;
  }

  if (!forbid_opt_module::can_login_for_char(char_id))
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_FORBID_LOGIN);
    return ERR_FORBID_LOGIN;
  }

  int ret = this->to_start_game(char_id);
  if (ret != 0)
    this->send_respond_err(RES_START_GAME, ret);
  return ret;
}
int player_obj::to_start_game(const int char_id)
{
  return this->to_load_all_db_info(char_id);
}
void player_obj::handle_db_get_char_info_result(char_info *cinfo)
{
  if (cinfo == NULL)
  {
    e_log->error("got %s char info is null!", this->account());
    this->shutdown();
    return ;
  }

  ::memcpy(this->char_info_, cinfo, sizeof(char_info));

  this->clt_state_ = CLT_LOAD_ALL_DB_OK;
  this->on_load_all_db_info_ok();
}
void player_obj::handle_db_get_char_extra_info_result(char_extra_info *info)
{
  if (info == NULL)
  {
    e_log->error("got %s char extra info is null!", this->account());
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }

  ::memcpy(this->char_extra_info_, info, sizeof(char_extra_info));
}
void player_obj::handle_db_get_daily_clean_info_result(daily_clean_info *info)
{
  if (info == NULL)
  {
    e_log->error("got %s char daily clean info is null!", this->account());
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }

  ::memcpy(this->daily_clean_info_, info, sizeof(daily_clean_info));
}
void player_obj::on_load_all_db_info_ok()
{
  if (this->do_init_char_info() != 0
      || this->do_calc_all_attr() != 0)
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_GET_PLAYER_DATA_FAILED);
    this->shutdown();
    return ;
  }

  this->clt_state_ = CLT_LOGIN_OK;
  player_mgr::instance()->insert(this->id(), this);
  this->on_char_login();

  // do respond client
  out_stream os(client::send_buf, client::send_buf_len);
  int zhu_wu_cid = 0;
  int fu_wu_cid = 0;
  package_module::get_equip_for_view(this, zhu_wu_cid, fu_wu_cid);
  os << stream_ostr(this->char_info_->name_, ::strlen(this->char_info_->name_))
    << this->char_info_->career_
    << this->char_info_->lvl_
    << this->char_info_->exp_
    << this->scene_cid_
    << this->dir_
    << this->coord_x_
    << this->coord_y_
    << this->char_info_->sin_val_
    << this->char_info_->pk_mode_
    << this->ac_info_->diamond_
    << this->char_info_->b_diamond_
    << this->char_info_->coin_
    << this->char_extra_info_->ti_li_
    << this->char_extra_info_->jing_li_
    << this->char_info_->cur_title_
    << zhu_wu_cid
    << fu_wu_cid
    << this->xszc_honor()
    << this->free_relive_cnt();
  guild_module::do_fetch_char_guild_info(this->guild_id(), os);
  os << guild_module::get_char_guild_pos(this->guild_id(), this->id_)
    << bao_shi_module::get_sum_bao_shi_lvl(this);

  if (this->send_respond_ok(RES_START_GAME, &os) != 0)
  {
    this->shutdown();
    return ;
  }

  // login ok
  s_log->rinfo("char %s:%d login ok %d:%d %d:%d!",
               this->account(), this->id(),
               this->scene_cid_, this->scene_id_, this->coord_x_, this->coord_y_);
  this->on_after_login();
}
int player_obj::do_init_char_info()
{
  this->id_              = this->char_info_->char_id_;
  this->cid_             = this->char_info_->char_id_;
  this->scene_id_        = this->char_info_->scene_id_;
  this->scene_cid_       = this->char_info_->scene_cid_;
  this->dir_             = this->char_info_->dir_;
  this->coord_x_         = this->char_info_->coord_x_;
  this->coord_y_         = this->char_info_->coord_y_;
  this->move_speed_      = PLAYER_DEFAULT_SPEED;
  this->prev_move_speed_ = this->move_speed_;
  if (this->char_info_->hp_ <= 0)
    SET_BITS(this->unit_status_, OBJ_DEAD);

  if (this->to_set_coord_ok() != 0) return -1;
  return 0;
}
int player_obj::do_calc_all_attr()
{
  this->do_calc_attr_affected_by_basic();
  this->do_calc_attr_affected_by_equip();
  this->do_calc_attr_affected_by_bao_shi();
  this->do_calc_attr_affected_by_passive_skill();
  this->do_calc_attr_affected_by_guild_skill();
  this->do_calc_attr_affected_by_title();

  if (this->char_info_->in_time_ == 0) // for new char
  {
    this->char_info_->hp_ = this->obj_attr_.total_hp();
    this->char_info_->mp_ = this->obj_attr_.total_mp();
  }
  // 在计算属性的过程中，会不断调整hp，上限也不断在变化
  if (this->char_info_->hp_ > this->obj_attr_.total_hp())
    this->char_info_->hp_ = this->obj_attr_.total_hp();
  if (this->char_info_->mp_ > this->obj_attr_.total_mp())
    this->char_info_->mp_ = this->obj_attr_.total_mp();
  this->obj_attr_.hp_ = this->char_info_->hp_;
  this->obj_attr_.mp_ = this->char_info_->mp_;

  obj_attr::reset_attr_v_add();
  this->obj_attr_.calc_zhan_li(obj_attr::attr_v_add);
  this->char_info_->zhan_li_ = attr_module::do_calc_zhan_li(this);
  char_brief_info::on_char_zhan_li_update(this->id_, this->char_info_->zhan_li_);
  return 0;
}
int player_obj::to_set_coord_ok()
{
  // 前端分包用的
  if (!this->has_resource_)
  {
    this->scene_cid_  = global_param_cfg::capital_relive_coord.cid_;
    this->scene_id_   = global_param_cfg::capital_relive_coord.cid_;
    this->coord_x_    = global_param_cfg::capital_relive_coord.coord_.x_;
    this->coord_y_    = global_param_cfg::capital_relive_coord.coord_.y_;
    return 0;
  }

  // 首张推图是不用判断的
  if (clsid::is_tui_tu_scp_scene(this->scene_cid_)
      || clsid::is_scp_scene(this->scene_cid_))
    return 0;

  if (scene_config::instance()->can_move(this->scene_cid_,
                                         this->coord_x_,
                                         this->coord_y_))
    return 0;
  coord_t random_pos = scene_config::instance()->get_scene_random_pos(this->scene_cid_,
                                                                      this->coord_x_,
                                                                      this->coord_y_);
  if (!scene_config::instance()->can_move(this->scene_cid_,
                                          random_pos.x_,
                                          random_pos.y_))
  {
    e_log->rinfo("mpt modified! but not found movable coord! "
                 "x=%d, y=%d",
                 random_pos.x_,
                 random_pos.y_);
    this->coord_x_ = 0;
    this->coord_y_ = 0;
    return -1;
  }

  this->coord_x_ = random_pos.x_;
  this->coord_y_ = random_pos.y_;
  return 0;
}
int player_obj::clt_enter_game(const char *, const int )
{
  if (this->clt_state_ != CLT_LOGIN_OK)
    return ERR_CLIENT_STATE_ERROR;

  if (this->do_enter_scene() != 0)
    return -1;

  this->clt_state_ = CLT_ENTER_GAME_OK;
  this->on_enter_game();
  this->enter_scene_ok_ = true;

  if (this->ac_info_->track_st_ == account_info::TK_CREATE_CHAR_OK)
  {
    this->ac_info_->track_st_ = account_info::TK_ENTER_GAME_OK;
    this->db_save_account_info();
  }
  return this->do_scan_snap(time_value::gettimeofday());
}
int player_obj::clt_reenter_game(const char *msg, const int len)
{
  if (this->clt_state_ != CLT_NULL) return ERR_CLIENT_STATE_ERROR;

  if (player_mgr::instance()->size() >= svc_config::instance()->max_online())
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_OVER_MAX_PAYLOAD);

  int char_id = 0;
  stream_istr ac_si(this->account_, sizeof(this->account_));
  in_stream is(msg, len);
  is >> ac_si >> char_id;

  util::to_lower(this->account_);
  s_log->rinfo("account [%s:%d] want to relogin!", this->account(), char_id);

  this->is_reenter_game_ = true;

  account_brief_info *abi =
    all_char_info::instance()->get_account_brief_info(this->account());
  if (abi == NULL
      || abi->char_list_.empty()
      || !abi->is_char_ok(char_id))
  {
    this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
    return ERR_CLIENT_OPERATE_ILLEGAL;
  }

  if (!forbid_opt_module::can_login_for_char(char_id))
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_FORBID_LOGIN);
    return ERR_FORBID_LOGIN;
  }

  this->ac_info_ = abi->ac_info_;  // !!!

  // repead login
  player_obj *po = player_mgr::instance()->find(this->account());
  if (po != NULL)
    po->do_kick_out();

  player_mgr::instance()->insert(this->account(), this);

  s_log->rinfo("account [%s] relogin!", this->account());

  this->clt_state_ = CLT_GET_CHAR_OK;

  int ret = this->to_start_game(char_id);
  if (ret != 0)
    this->send_respond_err(NTF_OPERATE_RESULT, ret);
  return ret;
}
int player_obj::clt_get_hot_name(const char *, const int )
{ return this->do_ntf_clt_hot_name(); }
