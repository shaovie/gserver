#include "ltime_act_obj.h"
#include "ltime_act_mgr.h"
#include "ltime_act_module.h"
#include "ltime_recharge_award.h"
#include "ltime_act_cfg.h"
#include "all_char_info.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "activity_mgr.h"
#include "db_proxy.h"
#include "mail_module.h"
#include "mail_config.h"
#include "mail_info.h"
#include "time_util.h"
#include "sys_log.h"
#include "error.h"
#include "def.h"
#include "sys.h"

// Lib header
#include "date_time.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("lact");
static ilog_obj *e_log = err_log::instance()->get_ilog("lact");

ltime_act_obj::ltime_act_obj(const int act_id,
                             const int begin_time,
                             const int end_time) :
  act_id_(act_id),
  status_(ltime_act_obj::ST_CLOSED),
  begin_time_(begin_time),
  end_time_(end_time)
{ }
ltime_act_obj::~ltime_act_obj()
{ }
void ltime_act_obj::on_time_update(const int begin_time, const int end_time)
{
  this->begin_time_ = begin_time;
  this->end_time_   = end_time;
}
void ltime_act_obj::on_enter_game(player_obj *player)
{
  if (!this->is_opened()) return ;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->act_id_ << this->begin_time() << this->end_time();
  player->do_delivery(NTF_LTIME_ACT_OPENED, &mb);
}
bool ltime_act_obj::do_check_if_opened(const int now)
{ return now > this->begin_time_ && now < this->end_time_; }
int ltime_act_obj::left_time()
{ return util::max(0, this->end_time_ - time_util::now); }
void ltime_act_obj::run(const int now)
{
  if (this->status_ == ltime_act_obj::ST_CLOSED)
  {
    if (this->do_check_if_opened(now))
    {
      if (this->open() == 0)
      {
        this->on_activity_opened();
        this->status_ = ltime_act_obj::ST_OPENED;
      }else
        e_log->rinfo("act %d open failed!", this->act_id_);
    }
  }else if (this->status_ == ltime_act_obj::ST_OPENED)
  {
    if (this->do_ai(now) != 0
        || !this->do_check_if_opened(now))
    {
      this->close();
      this->on_activity_closed();
      this->status_ = ltime_act_obj::ST_CLOSED;
    }
  }
}
void ltime_act_obj::on_activity_opened()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->act_id_ << this->begin_time() << this->end_time();
  player_mgr::instance()->broadcast_to_world(NTF_LTIME_ACT_OPENED, &mb);
}
void ltime_act_obj::on_activity_closed()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->act_id_;
  player_mgr::instance()->broadcast_to_world(NTF_LTIME_ACT_CLOSED, &mb);
}
void ltime_act_obj::do_send_mail_when_opened(const int mail_id)
{
  if (time_util::now - sys::svc_launch_time < LTIME_ACT_SET_LIMITED_TIME)
    return ;
  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_id);
  if (mo == NULL) return ;

  char time_bf[64] = {0};
  int size = 2;

  date_time(this->begin_time_).to_str(time_bf, sizeof(time_bf));
  ::strncpy(mail_module::replace_str[0], MAIL_P_DATE_TIME, sizeof(mail_module::replace_str[0]) - 1);
  ::snprintf(mail_module::replace_value[0],
             sizeof(mail_module::replace_value[0]),
             "%s(%s)",
             STRING_DATE_TIME, time_bf);

  date_time(this->end_time_).to_str(time_bf, sizeof(time_bf));
  ::strncpy(mail_module::replace_str[1], MAIL_P_DATE_TIME, sizeof(mail_module::replace_str[1]) - 1);
  ::snprintf(mail_module::replace_value[1],
             sizeof(mail_module::replace_value[1]),
             "%s(%s)",
             STRING_DATE_TIME, time_bf);

  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);
  mail_module::do_send_mail_2_all(mo->sender_name_, mo->title_, content,
                                  0, 0, 0,
                                  "", 0);
}
// --------------------------------------------------------------------------------
void ltime_exp_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_TIMES_EXP_OPENED);
}
// --------------------------------------------------------------------------------
void ltime_drop_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_TIMES_DROP_OPENED);
}
// --------------------------------------------------------------------------------
static void do_give_recharge_act_award(player_obj *player,
                                       const int mail_id,
                                       const int act_id,
                                       const int sub_id,
                                       const int value,
                                       ilist<item_amount_bind_t> &award_list)
{
  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_id);
  if (mo == NULL) return ;

  int size = 1;
  ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
  ::snprintf(mail_module::replace_value[0],
             sizeof(mail_module::replace_value[0]),
             "%s(%d)",
             STRING_NUMBER, value);
  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);
  mail_module::do_send_mail(player->id(),
                            player->career(),
                            player->db_sid(),
                            mail_info::MAIL_TYPE_GM,
                            mo->sender_name_, mo->title_, content,
                            award_list);

  ltime_recharge_award *lra = NULL;
  ilist_node<ltime_recharge_award *> *itor = ltime_act_module::get_ltime_recharge_award(player).head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_->act_id_ == act_id
        && itor->value_->sub_id_ == sub_id)
    {
      itor->value_->award_time_ = time_util::now;
      lra = itor->value_;
      break;
    }
  } 

  if (lra == NULL)
  {
    lra = new ltime_recharge_award();
    lra->char_id_ = player->id();
    lra->act_id_ = act_id;
    lra->sub_id_ = sub_id;
    lra->award_time_ = time_util::now;
    ltime_act_module::do_insert_ltime_recharge_award(player, lra);
  }
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << stream_ostr((char *)lra, sizeof(ltime_recharge_award));
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_LTIME_RECHARGE_AWARD, &os);
}
void ltime_acc_recharge_act_obj::on_recharge_ok(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_ACC_RECHARGE);
  if (lao == NULL
      || !lao->is_opened())
    return ;
  ilist<ltime_recharge_cfg_obj *> *rcfg =
    ltime_recharge_cfg::instance()->get_recharge_cfg(LTIME_ACT_ACC_RECHARGE);
  if (rcfg == NULL) return ;

  int value = char_brief_info::acc_recharge(player->id(), lao->begin_time(), lao->end_time());
  for (ilist_node<ltime_recharge_cfg_obj *> *itor = rcfg->head();
       itor != NULL;
       itor = itor->next_)
  {
    if (value >= itor->value_->param_1_
        && !ltime_acc_recharge_act_obj::if_got_award(player,
                                                     ltime_act_module::get_ltime_recharge_award(player),
                                                     LTIME_ACT_ACC_RECHARGE,
                                                     itor->value_->sub_id_,
                                                     lao->begin_time(),
                                                     lao->end_time()))
    {
      do_give_recharge_act_award(player,
                                 mail_config::LTIME_ACC_RECHARGE,
                                 LTIME_ACT_ACC_RECHARGE,
                                 itor->value_->sub_id_,
                                 itor->value_->param_1_,
                                 itor->value_->award_list_);
    }
  }
}
bool ltime_acc_recharge_act_obj::if_got_award(player_obj *,
                                              ilist<ltime_recharge_award *> &award_l,
                                              const int act_id,
                                              const int sub_id,
                                              const int begin_time,
                                              const int end_time)
{
  for (ilist_node<ltime_recharge_award *> *itor = award_l.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->act_id_ == act_id
        && itor->value_->sub_id_ == sub_id
        && itor->value_->award_time_ >= begin_time
        && itor->value_->award_time_ <= end_time)
      return true;
  } 
  return false;
}
int ltime_acc_recharge_act_obj::acc_recharge(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_ACC_RECHARGE);
  if (lao == NULL
      || !lao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  return char_brief_info::acc_recharge(player->id(), lao->begin_time(), lao->end_time());
}
void ltime_acc_recharge_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_ACC_RECHARGE_OPENED);
}
// --------------------------------------------------------------------------------
void ltime_daily_recharge_act_obj::on_recharge_ok(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_DAILY_RECHARGE);
  if (lao == NULL
      || !lao->is_opened())
    return ;
  ilist<ltime_recharge_cfg_obj *> *rcfg =
    ltime_recharge_cfg::instance()->get_recharge_cfg(LTIME_ACT_DAILY_RECHARGE);
  if (rcfg == NULL) return ;

  int value = char_brief_info::daily_recharge(player->id(), lao->begin_time(), lao->end_time());
  for (ilist_node<ltime_recharge_cfg_obj *> *itor = rcfg->head();
       itor != NULL;
       itor = itor->next_)
  {
    if (value >= itor->value_->param_1_
        && !ltime_daily_recharge_act_obj::if_got_award(player,
                                                       ltime_act_module::get_ltime_recharge_award(player),
                                                       LTIME_ACT_DAILY_RECHARGE,
                                                       itor->value_->sub_id_,
                                                       lao->begin_time(),
                                                       lao->end_time()))
    {
      do_give_recharge_act_award(player,
                                 mail_config::LTIME_DAILY_RECHARGE,
                                 LTIME_ACT_DAILY_RECHARGE,
                                 itor->value_->sub_id_,
                                 itor->value_->param_1_,
                                 itor->value_->award_list_);
    }
  }
}
bool ltime_daily_recharge_act_obj::if_got_award(player_obj *,
                                                ilist<ltime_recharge_award *> &award_l,
                                                const int act_id,
                                                const int sub_id,
                                                const int begin_time,
                                                const int end_time)
{
  for (ilist_node<ltime_recharge_award *> *itor = award_l.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->act_id_ == act_id
        && itor->value_->sub_id_ == sub_id
        && itor->value_->award_time_ >= begin_time
        && itor->value_->award_time_ <= end_time
        && time_util::diff_days(itor->value_->award_time_) == 0)
      return true;
  } 
  return false;
}
int ltime_daily_recharge_act_obj::daily_recharge(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_DAILY_RECHARGE);
  if (lao == NULL
      || !lao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  return char_brief_info::daily_recharge(player->id(), lao->begin_time(), lao->end_time());
}
void ltime_daily_recharge_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_DAILY_RECHARGE_OPENED);
}
// --------------------------------------------------------------------------------
void ltime_acc_consume_act_obj::on_consume_diamond(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_ACC_CONSUME);
  if (lao == NULL
      || !lao->is_opened())
    return ;
  ilist<ltime_recharge_cfg_obj *> *rcfg =
    ltime_recharge_cfg::instance()->get_recharge_cfg(LTIME_ACT_ACC_CONSUME);
  if (rcfg == NULL) return ;

  int value = char_brief_info::acc_consume(player->id(), lao->begin_time(), lao->end_time());
  for (ilist_node<ltime_recharge_cfg_obj *> *itor = rcfg->head();
       itor != NULL;
       itor = itor->next_)
  {
    if (value >= itor->value_->param_1_
        && !ltime_acc_recharge_act_obj::if_got_award(player,
                                                     ltime_act_module::get_ltime_recharge_award(player),
                                                     LTIME_ACT_ACC_CONSUME,
                                                     itor->value_->sub_id_,
                                                     lao->begin_time(),
                                                     lao->end_time()))
    {
      do_give_recharge_act_award(player,
                                 mail_config::LTIME_ACC_CONSUME,
                                 LTIME_ACT_ACC_CONSUME,
                                 itor->value_->sub_id_,
                                 itor->value_->param_1_,
                                 itor->value_->award_list_);
    }
  }
}
bool ltime_acc_consume_act_obj::if_got_award(player_obj *,
                                             ilist<ltime_recharge_award *> &award_l,
                                             const int act_id,
                                             const int sub_id,
                                             const int begin_time,
                                             const int end_time)
{
  for (ilist_node<ltime_recharge_award *> *itor = award_l.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->act_id_ == act_id
        && itor->value_->sub_id_ == sub_id
        && itor->value_->award_time_ >= begin_time
        && itor->value_->award_time_ <= end_time)
      return true;
  } 
  return false;
}
int ltime_acc_consume_act_obj::acc_consume(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_ACC_CONSUME);
  if (lao == NULL
      || !lao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  return char_brief_info::acc_consume(player->id(), lao->begin_time(), lao->end_time());
}
void ltime_acc_consume_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_ACC_CONSUME_OPENED);
}
// --------------------------------------------------------------------------------
void ltime_daily_consume_act_obj::on_consume_diamond(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_DAILY_CONSUME);
  if (lao == NULL
      || !lao->is_opened())
    return ;
  ilist<ltime_recharge_cfg_obj *> *rcfg =
    ltime_recharge_cfg::instance()->get_recharge_cfg(LTIME_ACT_DAILY_CONSUME);
  if (rcfg == NULL) return ;

  int value = char_brief_info::daily_consume(player->id(), lao->begin_time(), lao->end_time());
  for (ilist_node<ltime_recharge_cfg_obj *> *itor = rcfg->head();
       itor != NULL;
       itor = itor->next_)
  {
    if (value >= itor->value_->param_1_
        && !ltime_daily_consume_act_obj::if_got_award(player,
                                                      ltime_act_module::get_ltime_recharge_award(player),
                                                      LTIME_ACT_DAILY_CONSUME,
                                                      itor->value_->sub_id_,
                                                      lao->begin_time(),
                                                      lao->end_time()))
    {
      do_give_recharge_act_award(player,
                                 mail_config::LTIME_DAILY_CONSUME,
                                 LTIME_ACT_DAILY_CONSUME,
                                 itor->value_->sub_id_,
                                 itor->value_->param_1_,
                                 itor->value_->award_list_);
    }
  }
}
bool ltime_daily_consume_act_obj::if_got_award(player_obj *,
                                               ilist<ltime_recharge_award *> &award_l,
                                               const int act_id,
                                               const int sub_id,
                                               const int begin_time,
                                               const int end_time)
{
  for (ilist_node<ltime_recharge_award *> *itor = award_l.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->act_id_ == act_id
        && itor->value_->sub_id_ == sub_id
        && itor->value_->award_time_ >= begin_time
        && itor->value_->award_time_ <= end_time
        && time_util::diff_days(itor->value_->award_time_) == 0)
      return true;
  } 
  return false;
}
int ltime_daily_consume_act_obj::daily_consume(player_obj *player)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(LTIME_ACT_DAILY_CONSUME);
  if (lao == NULL
      || !lao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  return char_brief_info::daily_consume(player->id(), lao->begin_time(), lao->end_time());
}
void ltime_daily_consume_act_obj::on_activity_opened()
{
  super::on_activity_opened();
  this->do_send_mail_when_opened(mail_config::LTIME_DAILY_CONSUME_OPENED);
}
