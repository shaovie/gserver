#include "player_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "char_info.h"
#include "account_info.h"
#include "daily_clean_info.h"
#include "db_session_mgr.h"
#include "skill_info.h"
#include "all_char_info.h"
#include "global_macros.h"
#include "global_param_cfg.h"
#include "mblock_pool.h"
#include "player_mgr.h"
#include "fighter_mgr.h"
#include "time_util.h"
#include "db_proxy.h"
#include "error.h"
#include "util.h"
#include "sys.h"
#include "social_module.h"
#include "team_module.h"
#include "task_module.h"
#include "chat_module.h"
#include "package_module.h"
#include "buff_module.h"
#include "skill_module.h"
#include "passive_skill_module.h"
#include "tianfu_skill_module.h"
#include "mail_module.h"
#include "trade_module.h"
#include "scene_config.h"
#include "iu_module.h"
#include "monster_mgr.h"
#include "guild_module.h"
#include "mall_buy_log.h"
#include "mall_module.h"
#include "market_module.h"
#include "scp_module.h"
#include "rank_module.h"
#include "tui_tu_module.h"
#include "mis_award_module.h"
#include "vip_module.h"
#include "cache_module.h"
#include "cheng_jiu_module.h"
#include "bao_shi_module.h"
#include "huo_yue_du_module.h"
#include "jing_ji_module.h"
#include "char_msg_queue_module.h"
#include "activity_module.h"
#include "ltime_act_module.h"
#include "cross_module.h"
#include "behavior_log.h"
#include "mobai_module.h"
#include "water_tree_module.h"
#include "dxc_module.h"
#include "lue_duo_module.h"
#include "fa_bao_module.h"
#include "attr_module.h"

// Lib header
#include "reactor.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

player_obj::player_obj() :
  client(CLIENT_RECV_BUFF_SIZE, "game client"),
  is_reenter_game_(false),
  enter_scene_ok_(false),
  notify_jing_ji_(false),
  has_resource_(true),
  group_(0),
  move_speed_(PLAYER_DEFAULT_SPEED),
  prev_move_speed_(PLAYER_DEFAULT_SPEED),
  all_qh_lvl_(0),
  sum_bao_shi_lvl_(0),
  db_sid_(-1),
  clt_state_(CLT_NULL),
  team_id_(0),
  guild_id_(0),
  last_check_1_sec_time_(time_util::now),
  last_check_3_sec_time_(time_util::now),
  last_clt_heart_beat_time_(time_util::now),
  last_fight_time_(0),
  scp_award_exp_(0),
  enter_scp_time_(0),
  ac_info_(NULL),
  char_info_(NULL),
  char_extra_info_(NULL),
  daily_clean_info_(NULL),
  char_recharge_(NULL),
  baowu_mall_info_(NULL),
  msg_buff_chunk_(NULL),
  social_relation_(NULL),
  pack_info_(NULL),
  task_data_(NULL),
  trade_info_(NULL),
  vip_info_(NULL),
  cheng_jiu_data_(NULL),
  huo_yue_du_data_(NULL)
{
  ::memset(this->account_, '\0', sizeof(this->account_));

  this->db_sid_ = db_session_mgr::instance()->alloc_session_id();
  db_session_mgr::instance()->insert(this->db_sid_, this);
  player_mgr::instance()->attach_timer(this);

  this->char_info_ = char_info_pool::instance()->alloc();
  this->char_extra_info_ = char_extra_info_pool::instance()->alloc();
  this->daily_clean_info_ = daily_clean_info_pool::instance()->alloc();

  this->old_snap_unit_list_ = new ilist<pair_t<int> >();
  this->new_snap_unit_list_ = new ilist<pair_t<int> >();

  // module
  task_module::init(this);
  package_module::init(this);
  cheng_jiu_module::init(this);
  huo_yue_du_module::init(this);

  this->do_attach_sev();
}
player_obj::~player_obj()
{
  this->do_logout();

  db_session_mgr::instance()->remove(this->db_sid_);
  player_mgr::instance()->remove(this->account());
  player_mgr::instance()->remove(this->id_);
  player_mgr::instance()->deattach_timer(this);

  // destroy
  this->ac_info_ = NULL;
  char_info_pool::instance()->release(this->char_info_);
  char_extra_info_pool::instance()->release(this->char_extra_info_);
  daily_clean_info_pool::instance()->release(this->daily_clean_info_);
  delete this->char_recharge_;
  for (skill_map_itor itor = this->skill_map_.begin();
       itor != this->skill_map_.end();
       ++itor)
    skill_info_pool::instance()->release(itor->second);

  while (!this->mall_buy_list_.empty())
    delete this->mall_buy_list_.pop_front();

  delete this->old_snap_unit_list_;
  delete this->new_snap_unit_list_;
  this->do_release_msg_buff();

  // module
  social_module::destroy(this);
  task_module::destroy(this);
  package_module::destroy(this);
  passive_skill_module::destroy(this);
  guild_module::destroy(this);
  tui_tu_module::destroy(this);
  scp_module::destroy(this);
  cheng_jiu_module::destroy(this);
  bao_shi_module::destroy(this);
  huo_yue_du_module::destroy(this);
  jing_ji_module::destroy(this);
  vip_module::destroy(this);
  mall_module::destroy(this);
  ltime_act_module::destroy(this);
  lue_duo_module::destroy(this);
  activity_module::destroy(this);

  this->do_deattach_sev();
}
int player_obj::open(void *arg)
{
  if (client::open(arg) != 0)
    return -1;

  return 0;
}
void player_obj::do_delivery(mblock *mb)
{
  if (mb->length() >= TO_CLIENT_MSG_SLICE_BUF_SIZE)
  {
    e_log->rinfo("char[%d]: length of mb is[%d] in %s",
                 this->id_, mb->length(), __func__);
    return ;
  }
  if (this->msg_buff_chunk_ == NULL)
    this->msg_buff_chunk_ = mblock_pool::instance()->alloc(TO_CLIENT_MSG_SLICE_BUF_SIZE);
  else if (this->msg_buff_chunk_->space() < mb->length())
  {
    this->msg_buff_chunk_list_.push_back(this->msg_buff_chunk_);
    this->msg_buff_chunk_ = mblock_pool::instance()->alloc(TO_CLIENT_MSG_SLICE_BUF_SIZE);
  }
  this->msg_buff_chunk_->copy(mb->rd_ptr(), mb->length());
}
void player_obj::do_delivery(const int msg_id, mblock *mb)
{
  proto_head *ph = (proto_head *)mb->rd_ptr();
  ph->set(0, msg_id, 0, mb->length());
  this->do_delivery(mb);
}
int player_obj::do_send_msg_buff_chunk()
{
  if (this->msg_buff_chunk_ == NULL
      && this->msg_buff_chunk_list_.empty())
    return 0;

  mblock *mb = NULL;
  if (!this->msg_buff_chunk_list_.empty())
    mb = this->msg_buff_chunk_list_.pop_front();
  else
  {
    mb = this->msg_buff_chunk_;
    this->msg_buff_chunk_ = NULL;
  }

  int ret = this->send_data(mb->rd_ptr(), mb->length());
  mblock_pool::instance()->release(mb);
  return ret;
}
void player_obj::do_release_msg_buff()
{
  if (this->msg_buff_chunk_ != NULL)
    mblock_pool::instance()->release(this->msg_buff_chunk_);
  while (!this->msg_buff_chunk_list_.empty())
  {
    mblock *mb = this->msg_buff_chunk_list_.pop_front();
    mblock_pool::instance()->release(mb);
  }
  while (!this->snap_slice_list_.empty())
  {
    mblock *mb = this->snap_slice_list_.pop_front();
    mblock_pool::instance()->release(mb);
  }
}
int player_obj::dispatch_msg(const int msg_id,
                             const int ,
                             const char *msg,
                             const int len)
{
  this->handle_aev_queue();

#ifndef PUBLISH
  s_log->rinfo("[%s]  message %d h=%d", this->name(), msg_id, this->get_handle());
#endif
  int ret = 0;
  if (msg_id == REQ_CLT_HEART_BEAT) // 心跳随时都可以处理
    ret = this->clt_heart_beat(msg, len);
  else
  {
    this->last_clt_heart_beat_time_ = time_util::now;
    int major_id = MAJOR_ID(msg_id);
    if (major_id == MJ_LOGIN_MODULE)
      ret = this->do_login_module_msg(msg_id, msg, len);
    else
    {
      if (this->clt_state_ != CLT_ENTER_GAME_OK)
        ret = ERR_CLIENT_STATE_ERROR;
      else
      {
        switch (major_id)
        {
        case MJ_BASIC_MODULE:
          ret = this->do_basic_module_msg(msg_id, msg, len);
          break;
        case MJ_PACKAGE_MODULE:
          ret = package_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_SOCIAL_MODULE:
          ret = social_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_TASK_MODULE:
          ret = task_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_TEAM_MODULE:
          ret = team_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_CHAT_MODULE:
          ret = chat_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_SKILL_MODULE:
          ret = skill_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_MAIL_MODULE:
          ret = mail_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_PASSIVE_SKILL_MODULE:
          ret = passive_skill_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_TIANFU_SKILL_MODULE:
          ret = tianfu_skill_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_IU_MODULE:
          ret = iu_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_GUILD_MODULE:
          ret = guild_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_MARKET_MODULE:
          ret = market_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_SCP_MODULE:
          ret = scp_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_RANK_MODULE:
          ret = rank_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_TUI_TU_MODULE:
          ret = tui_tu_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_MIS_AWARD_MODULE:
          ret = mis_award_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_VIP_MODULE:
          ret = vip_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_CHENG_JIU_MODULE:
          ret = cheng_jiu_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_BAO_SHI_MODULE:
          ret = bao_shi_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_JING_JI_MODULE:
          ret = jing_ji_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_ACTIVITY_MODULE:
          ret = activity_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_LTIME_ACT_MODULE:
          ret = ltime_act_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_CROSS_MODULE:
          ret = cross_module::dispatch_msg(this, msg_id, msg, len, 0);
          break;
        case MJ_MOBAI_MODULE:
          ret = mobai_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_WATER_TREE_MODULE:
          ret = water_tree_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_DI_XIA_CHENG_MODULE:
          ret = dxc_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_LUE_DUO_MODULE:
          ret = lue_duo_module::dispatch_msg(this, msg_id, msg, len);
          break;
        case MJ_FA_BAO_MODULE:
          ret = fa_bao_module::dispatch_msg(this, msg_id, msg, len);
          break;
        default:
          e_log->error("%s unknow major id %d %d", __func__, major_id, msg_id);
          break;
        }
      }
    }
  }

  if (ret != 0)
    e_log->wning("handle msg %d return %d", msg_id, ret);
  return ret;
}
int player_obj::check_package_seq(const int seq)
{
#ifdef FOR_IROBOT
  return 0;
#endif
  if (this->seq_ != seq)
  {
    e_log->wning("client %s h:%d package seq %d:%d invalid!",
                 this->account(),
                 this->get_handle(),
                 this->seq_,
                 seq);
    return -1;
  }
  return 0;
}
int player_obj::do_timeout(const time_value &now)
{
  int now_sec = now.sec();
  // handle pending events at first.
  this->handle_aev_queue();

  if (this->clt_state_ != CLT_ENTER_GAME_OK)
    return 0;

  if (this->do_something_every_1_sec(now_sec) != 0) return -1;
  if (this->do_something_every_3_sec(now_sec) != 0) return -1;

  this->do_update_all_buff(now);
  this->do_resume_energy(now);

  if (this->do_scan_snap(now) != 0) return -1;

  // scan的逻辑要在前面，因为比如广播之类的消息，前端肯定是先有这个对象才对处理广播消息
  if (this->do_send_msg_buff_chunk() != 0)
    return -1;
  return 0;
}
int player_obj::do_something_every_1_sec(const int now)
{
  if (now - this->last_check_1_sec_time_ <= 0) return 0;
  this->last_check_1_sec_time_ = now;

  this->do_check_fight_status(now);
  this->do_check_online_award(now);
  fa_bao_module::do_timeout(this, now);

  return 0;
}
int player_obj::do_something_every_3_sec(const int now)
{
  int diff = now - this->last_check_3_sec_time_;
  if (diff <= 2) return 0;
  this->last_check_3_sec_time_ = now;

  if (this->do_check_clt_heart_beat() != 0) return -1;

  this->char_info_->ol_time_ += 3;
  if (this->char_info_->ol_time_ % 300 == 0) // 每5分钟更新一次成就信息
    cheng_jiu_module::on_online_time(this, this->char_info_->ol_time_);

  jing_ji_module::do_notify_can_jing_ji(this);

  // sin val
  if (this->char_info_->sin_val_ > 0)
  {
    this->char_info_->sin_ol_time_ += diff; // 这个就不用保存DB了，logout的时候自然会保存
    if (this->char_info_->sin_ol_time_ > global_param_cfg::sin_val_decrease_time)
    {
      this->char_info_->sin_ol_time_ = 0;
      this->char_info_->sin_val_ -= 1;
      this->db_save_char_info();
      char_brief_info::on_char_sin_val_update(this->id(), this->char_info_->sin_val_);
      this->broadcast_sin_val();
    }
  }

  this->do_timing_add_ti_li_value(now, false);
  this->do_timing_add_jing_li_value(now, false);
  return 0;
}
int player_obj::do_check_clt_heart_beat()
{
  if (time_util::now - this->last_clt_heart_beat_time_ > 600)
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_CLIENT_NETWORK_UNSTABLE);
    e_log->rinfo("char %s:%d heart beat timeout!", this->account(), this->id_);
    return -1;
  }
  if (this->clt_state_ < CLT_ENTER_GAME_OK
      && time_util::now - this->last_check_1_sec_time_ > 300) // 借用一下这个时间: 登录过程不能停留太久
  {
    this->send_respond_err(NTF_PLAYER_LOGOUT, ERR_LOGIN_TIMEOUT);
    e_log->rinfo("char %s:%d stay too long in login state!", this->account(), this->id_);
    return -1;
  }
  return 0;
}
void player_obj::on_new_day(const bool when_login, const int diff_days)
{
  this->daily_clean_info_->clean_time_ = time_util::now;
  this->daily_clean_info_->guild_free_jb_cnt_ = 0;
  this->daily_clean_info_->guild_cost_jb_cnt_ = 0;
  this->daily_clean_info_->guild_promote_skill_cnt_ = 0;
  this->daily_clean_info_->ol_award_get_idx_ = 0;
  this->daily_clean_info_->login_award_if_get_ = 0;
  this->daily_clean_info_->daily_task_done_cnt_ = 0;
  this->daily_clean_info_->guild_task_done_cnt_ = 0;
  this->daily_clean_info_->jing_ji_cnt_ = 0;
  this->daily_clean_info_->ol_award_start_time_ = time_util::now;
  this->daily_clean_info_->if_mc_rebate_ = 0;
  this->daily_clean_info_->worship_cnt_ = 0;
  this->daily_clean_info_->goods_lucky_turn_cnt_ = 0;
  this->daily_clean_info_->daily_wx_shared_ = 0;
  this->daily_clean_info_->if_got_goods_lucky_draw_ = 0;
  this->daily_clean_info_->free_relive_cnt_ = 0;
  this->daily_clean_info_->dxc_enter_cnt_ = 0;
  if (diff_days > 1)
    this->daily_clean_info_->con_login_days_ = 1;
  else
  {
    this->daily_clean_info_->con_login_days_ += 1;
    if (!when_login)
      cheng_jiu_module::on_continue_login_day(this,
                                              this->daily_clean_info_->con_login_days_);
  }
  this->db_save_daily_clean_info();

  task_module::on_new_day(this, when_login);
  mis_award_module::on_new_day(this, when_login);
  huo_yue_du_module::on_new_day(this);
  scp_module::on_new_day(this, when_login);
  vip_module::on_new_day(this, when_login);
  activity_module::on_new_day(this, when_login);
}
int player_obj::do_logout()
{
  if (this->char_info_->char_id_ == 0) return 0;

  if (this->clt_state_ == CLT_ENTER_GAME_OK)
    this->do_exit_scene();

  this->on_char_logout();

  this->char_info_->out_time_ = time_util::now;
  this->db_save_char_info();

  return 0;
}
void player_obj::on_char_login()
{
  int last_cln_time = this->daily_clean_info_->clean_time_;
  int diff_days = time_util::diff_days(last_cln_time);
  if (diff_days > 0)
    this->on_new_day(true, diff_days);

  if (this->ac_info_->char_id_ != this->id())
  {
    this->ac_info_->char_id_ = this->id();
    this->db_save_account_info();
  }

  this->do_timing_add_ti_li_value(time_util::now, true);
  this->do_timing_add_jing_li_value(time_util::now, true);
  guild_module::on_char_login(this);
  team_module::on_char_login(this);
  scp_module::on_char_login(this);
  activity_module::on_char_login(this);
  tui_tu_module::on_char_login(this);
  char_msg_queue_module::on_char_login(this);

  behavior_log::instance()->store(BL_CHAR_LOGIN, time_util::now, "%d", this->id_);
}
void player_obj::on_after_login()
{
  vip_module::on_after_login(this);
  jing_ji_module::on_after_login(this);
  this->notify_recharge_info();
}
void player_obj::on_enter_game()
{
  this->next_resume_energy_time_.set(time_util::now + 3, 0);

  this->all_qh_lvl_ = attr_module::get_equip_all_qh_lvl(this);
  this->sum_bao_shi_lvl_ = bao_shi_module::get_sum_bao_shi_lvl(this);
  this->do_notify_attr_to_clt();
  this->do_notify_zhan_xing_v_to_clt();
  this->do_notify_goods_lucky_turn_info_to_clt();
  this->do_notify_fa_bao_info_to_clt();
  package_module::on_enter_game(this);
  social_module::on_enter_game(this);
  team_module::on_enter_game(this);
  task_module::on_enter_game(this);
  skill_module::on_enter_game(this);
  passive_skill_module::on_enter_game(this);
  buff_module::on_enter_game(this);
  mail_module::on_enter_game(this);
  mall_module::on_enter_game(this);
  mis_award_module::on_enter_game(this);
  huo_yue_du_module::on_enter_game(this);
  scp_module::on_enter_game(this);
  activity_module::on_enter_game(this);
  ltime_act_module::on_enter_game(this);
  cheng_jiu_module::on_enter_game(this);
  guild_module::on_enter_game(this);
  tianfu_skill_module::on_enter_game(this);
  mobai_module::on_enter_game(this);
  water_tree_module::on_enter_game(this);
  fa_bao_module::on_enter_game(this);

  // last one
  chat_module::on_enter_game(this);

  this->char_info_->in_time_ = time_util::now;
  char bf[32] = {0};
  this->remote_addr_.get_host_addr(bf, sizeof(bf));
  if (::strcmp(this->char_info_->ip_, bf) != 0)
    ::strncpy(this->char_info_->ip_, bf, sizeof(this->char_info_->ip_) - 1);
  this->db_save_char_info();
}
void player_obj::on_char_logout()
{
  social_module::on_char_logout(this);
  team_module::on_char_logout(this);
  chat_module::on_char_logout(this);
  buff_module::on_char_logout(this);
  trade_module::on_char_logout(this);
  char_brief_info::on_char_logout(this);
  cache_module::on_char_logout(this);
  tui_tu_module::on_char_logout(this);
  scp_module::on_char_logout(this);
  lue_duo_module::on_char_logout(this);
  dxc_module::on_char_logout(this);

  behavior_log::instance()->store(BL_CHAR_LOGOUT, time_util::now, "%d", this->id_);
}
void player_obj::on_attack_somebody(const int target_id,
                                    const int obj_type,
                                    const time_value &/*now*/,
                                    const int /*skill_cid*/,
                                    const int /*real_hurt*/)
{
  if (obj_type == scene_unit::PLAYER)
  {
    this->add_status(OBJ_FIGHTING);
    this->last_fight_time_ = time_util::now;
  }

  if (target_id != 0
      && this->char_info_->career_ == CAREER_LI_LIANG
      && !this->zhuzai_fen_shen_.empty())
  {
    int ls = this->zhuzai_fen_shen_.size();
    for (int i = 0; i < ls; ++i)
    {
      int fen_shen_id = this->zhuzai_fen_shen_.pop_front();
      char_obj *fen_shen = fighter_mgr::instance()->find(fen_shen_id);
      if (fen_shen != NULL)
      {
        mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
        *mb << target_id;
        fen_shen->post_aev(AEV_NOTIFY_ATTACK_TARGET_ID, mb);
        this->zhuzai_fen_shen_.push_back(fen_shen_id);
      }
    }
  }
}
void player_obj::on_be_attacked(char_obj *attacker)
{
  if (attacker->unit_type() == scene_unit::PLAYER)
  {
    this->add_status(OBJ_FIGHTING);
    this->last_fight_time_ = time_util::now;
  }
}
void player_obj::do_check_fight_status(const int now)
{
  if (BIT_DISABLED(this->unit_status_, OBJ_FIGHTING)
      || now - this->last_fight_time_ < 3)
    return ;
  this->del_status(OBJ_FIGHTING);
}
int player_obj::db_save_char_info()
{
  this->char_info_->scene_id_ = this->scene_id_;
  this->char_info_->scene_cid_ = this->scene_cid_;
  this->char_info_->dir_ = this->dir_;
  this->char_info_->coord_x_ = this->coord_x_;
  this->char_info_->coord_y_ = this->coord_y_;
  this->char_info_->hp_ = this->obj_attr_.hp_;
  this->char_info_->mp_ = this->obj_attr_.mp_;

  stream_ostr so((const char *)this->char_info_, sizeof(char_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(this->id_,
                                         REQ_UPDATE_CHAR_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::db_save_char_extra_info()
{
  stream_ostr so((const char *)this->char_extra_info_, sizeof(char_extra_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(this->id_,
                                         REQ_UPDATE_CHAR_EXTRA_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::db_save_daily_clean_info()
{
  stream_ostr so((const char *)this->daily_clean_info_, sizeof(daily_clean_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(this->id_,
                                         REQ_UPDATE_DAILY_CLEAN_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::db_save_account_info()
{
  stream_ostr so((const char *)this->ac_info_, sizeof(account_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(::abs(this->account_[0]),
                                         REQ_UPDATE_ACCOUNT_INFO,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
void player_obj::do_broadcast(mblock *mb, const bool to_self)
{
  if (to_self) this->do_delivery(mb);
  ilist_node<pair_t<int> > *itor = this->old_snap_unit_list_->head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_.first_ == itor->value_.second_ // is a player
        && itor->value_.first_ != this->id_)
    {
      player_obj *player = player_mgr::instance()->find(itor->value_.first_);
      if (player == NULL) continue;
      player->do_delivery(mb);
    }
  }
}
void player_obj::do_post_aev_in_view_area(const int aev_id,
                                          mblock *ev_mb,
                                          const int include_unit_type)
{
  ilist_node<pair_t<int> > *itor = this->old_snap_unit_list_->head();
  for (; itor != NULL; itor = itor->next_)
  {
    char_obj *co = fighter_mgr::instance()->find(itor->value_.first_);
    if (co == NULL
        || BIT_DISABLED(co->unit_type(), include_unit_type))
      continue;
    mblock *mb = NULL;
    if (ev_mb != NULL)
    {
      mb = mblock_pool::instance()->alloc(ev_mb->length());
      mb->copy(ev_mb->rd_ptr(), ev_mb->length());
    }
    co->post_aev(aev_id, mb);
  }
  if (ev_mb != NULL)
    mblock_pool::instance()->release(ev_mb);
}
bool player_obj::is_in_my_view(const int target_id)
{
  ilist_node<pair_t<int> > *itor = this->old_snap_unit_list_->head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_.first_ == target_id)
      return true;
  }
  return false;
}
int player_obj::can_attack(char_obj *target)
{
  if (this->scene_cid_ != global_param_cfg::jing_ji_map
      && this->scene_cid_ != global_param_cfg::lue_duo_map
      && this->scene_cid_ != global_param_cfg::battle_scene_cid)
  {
    if (target->unit_type() == scene_unit::PLAYER)
    {
      if (this->char_info_->pk_mode_ == PK_MODE_PEACE)
        return ERR_CAN_NOT_FIGHT_IN_PEACE_MODE;
      if (target->lvl() < global_param_cfg::pk_lvl_limit)
        return ERR_CAN_NOT_ATTACK_TARGET;
      if (!scene_config::instance()->can_pk(this->scene_cid_))
        return ERR_CAN_NOT_FIGHT_IN_PEACE_SCENE;
    }
    if (this->team_id() != 0
        && this->team_id() == target->team_id())
      return ERR_TEAM_MEMBER_CAN_NOT_ATTACK;
    if (this->guild_id() != 0
        && this->guild_id() == target->guild_id())
      return ERR_GUILD_MEMBER_CAN_NOT_ATTACK;
  }
  return char_obj::can_attack(target);
}
