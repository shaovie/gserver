#include "player_obj.h"
#include "recharge_module.h"
#include "recharge_config.h"
#include "recharge_log.h"
#include "sys_log.h"
#include "db_proxy.h"
#include "time_util.h"
#include "mail_module.h"
#include "mail_config.h"
#include "mail_info.h"
#include "behavior_log.h"
#include "all_char_info.h"
#include "time_util.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "zhan_xing_module.h"
#include "order_serial.h"
#include "ltime_act_module.h"
#include "month_card_award_act.h"
#include "package_module.h"
#include "global_param_cfg.h"
#include "clsid.h"
#include "notice_module.h"

// Lib header
#include <json/json.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("recharge");
static ilog_obj *e_log = err_log::instance()->get_ilog("recharge");

void player_obj::handle_db_get_char_recharge_result(char_recharge *info)
{
  if (info == NULL)
  {
    e_log->error("got %s char recharge is null!", this->account());
    this->shutdown();
    return ;
  }

  this->char_recharge_ = new char_recharge();
  ::memcpy(this->char_recharge_, info, sizeof(char_recharge));
}
int player_obj::db_save_char_recharge()
{
  if (this->char_recharge_ == NULL) return 0;
  stream_ostr so((const char *)this->char_recharge_, sizeof(char_recharge));
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << so;
  if (db_proxy::instance()->send_request(this->id_,
                                         REQ_UPDATE_CHAR_RECHARGE,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}

int player_obj::last_buy_mc_time()
{
  if (this->char_recharge_ == NULL)
    return 0;
  return this->char_recharge_->last_buy_mc_time_;
}
void player_obj::last_buy_mc_time(const int v)
{
  if (this->char_recharge_ == NULL)
    this->char_recharge_ = new char_recharge(this->id());
  this->char_recharge_->last_buy_mc_time_ = v;
}
int player_obj::increase_recharge_cnt(const int rc_type)
{
  if (this->char_recharge_ == NULL)
    this->char_recharge_ = new char_recharge(this->id());
  char_recharge *rc = this->char_recharge_;

  Json::Value root;
  Json::Reader j_reader;
  if (!j_reader.parse(rc->data_,
                      rc->data_ + ::strlen(rc->data_),
                      root,
                      false))
  {
    e_log->rinfo("load char recharge data[%s] failed!", rc->data_);
    return 99999999;
  }
  int last_cnt = 0;
  char buff[16] = {0};
  ::snprintf(buff, sizeof(buff), "%d", rc_type);
  if (root.isMember(buff))
    last_cnt = root[buff].asInt();

  root[buff] = Json::Value(last_cnt + 1);

  Json::FastWriter j_writer;
  std::string new_str = j_writer.write(root);
  if (new_str.length() < sizeof(rc->data_))
    ::snprintf(rc->data_, sizeof(rc->data_), "%s", new_str.c_str());
  else
    e_log->rinfo("increase type %d failed! out of space!", rc_type);

  return last_cnt;
}
int player_obj::clt_get_first_recharge_award(const char *, const int )
{
  char_brief_info *cinfo = all_char_info::instance()->get_char_brief_info(this->id());
  if (cinfo == NULL
      || cinfo->recharge_log_.empty())
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (BIT_ENABLED(this->yes_or_no(), char_extra_info::ST_GOT_FIRST_RECHARGE))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_HAD_GOT_AWARD);

  const int ret = package_module::do_insert_award_item_list(this,
                                                            &global_param_cfg::first_recharge_gift,
                                                            blog_t(ITEM_GOT_FIRST_RECHARGE, 0, 0));
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  this->yes_or_no(this->yes_or_no() | char_extra_info::ST_GOT_FIRST_RECHARGE);
  this->db_save_char_extra_info();

  // notice
  ilist_node<item_amount_bind_t> *itor = global_param_cfg::first_recharge_gift.head();
  for (; itor != NULL; itor = itor->next_)
  {
    item_amount_bind_t &io = itor->value_;
    if (clsid::is_char_equip(io.cid_))
    {
      item_obj notice_item;
      notice_item.cid_ = clsid::get_equip_cid_by_career(this->career(), io.cid_);
      notice_item.amount_ = io.amount_;
      notice_item.bind_ = io.bind_;
      notice_module::got_first_recharge_equip_award(this->id(), this->name(), &notice_item);
    }
  }

  return this->send_respond_ok(RES_GET_FIRST_RECHARGE_AWARD);
}

bool recharge_module::can_buy_mc(player_obj *player, const int now)
{
  return (time_util::diff_days(now, player->last_buy_mc_time())
          >= MONTHLY_CARD_REBATE_DAYS);
}
void recharge_module::do_recharge(player_obj *player,
                                  const int diamond,
                                  const int rc_type,
                                  const int rmb,
                                  const char *orderid, const char *platform)
{
  if (rc_type == RECHARGE_MONTHLY_CARD) // 月卡
  {
    player->last_buy_mc_time(time_util::now);
    month_card_award_act::on_buy_month_card_ok(player);
  }
  int has_buy_cnt = player->increase_recharge_cnt(rc_type);
  player->db_save_char_recharge();

  player->do_got_money(diamond, M_DIAMOND, MONEY_GOT_RECHARGE, 0);
  recharge_module::on_recharge_ok(player,
                                  diamond,
                                  rc_type,
                                  rmb,
                                  orderid,
                                  platform);

  recharge_module::do_recharge_rebate(player, has_buy_cnt, rc_type);
}
void recharge_module::on_recharge_ok(player_obj *player,
                                     const int diamond,
                                     const int rc_type,
                                     const int rmb,
                                     const char *orderid, const char *platform)
{
  recharge_log rl;
  rl.char_id_ = player->id();
  rl.value_   = diamond;
  rl.rc_time_ = time_util::now;

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr((const char *)&rl, sizeof(recharge_log));
  if (db_proxy::instance()->send_request(player->id(),
                                         REQ_INSERT_RECHARGE_LOG,
                                         &os) == 0)
    char_brief_info::on_recharge_ok(player->id(), time_util::now, diamond);

  behavior_log::instance()->store(BL_RECHARGE,
                                  time_util::now,
                                  "%s|%d|%d|%s|%s",
                                  player->account(),
                                  diamond,
                                  rmb,
                                  orderid,
                                  platform);

  player->notify_recharge_info();
  vip_module::on_recharge_ok(player, diamond);
  zhan_xing_module::on_recharge_ok(player, diamond);
  ltime_act_module::on_recharge_ok(player);

#define MAKE_REPLACE_STR(INDEX, ORISTR, FORMAT, REPSTR, DATA) \
  ::strncpy(mail_module::replace_str[INDEX], (ORISTR), sizeof(mail_module::replace_str[INDEX]) - 1); \
  ::snprintf(mail_module::replace_value[INDEX], \
             sizeof(mail_module::replace_value[INDEX]), \
             FORMAT, REPSTR, DATA);
  // 确认邮件
  const mail_obj *mo = NULL;
  if (rc_type == RECHARGE_MONTHLY_CARD)
    mo = mail_config::instance()->get_mail_obj(mail_config::RECHARGE_MC_INFORM);
  else
    mo = mail_config::instance()->get_mail_obj(mail_config::RECHARGE_INFORM);
  if (mo != NULL)
  {
    int size = 2;
    char datetime[24] = {0};
    date_time(time_util::now).to_str(datetime, sizeof(datetime));
    MAKE_REPLACE_STR(0, MAIL_P_DATE_TIME, "%s(%s)", STRING_DATE_TIME, datetime);
    MAKE_REPLACE_STR(1, MAIL_P_NUMBER, "%s(%d)", STRING_NUMBER, diamond);
    if (rc_type == RECHARGE_MONTHLY_CARD)
    {
      recharge_cfg_obj *rco = recharge_config::instance()->get_recharge_obj(rc_type);
      int b_diamond = (rco == NULL ? 0 : rco->b_diamond_1_);
      size = 4;
      MAKE_REPLACE_STR(2, MAIL_P_NUMBER, "%s(%d)", STRING_NUMBER, b_diamond);
      date_time(time_util::now + 3600 * 24 * (MONTHLY_CARD_REBATE_DAYS - 1))
        .date_to_str(datetime, sizeof(datetime));
      MAKE_REPLACE_STR(3, MAIL_P_DATE, "%s(%s)", STRING_DATE, datetime);
    }
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   size,
                                   content);
    mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                              mo->sender_name_, mo->title_, content,
                              mail_info::MAIL_TYPE_GM,
                              0, 0, 0, 0, NULL,
                              player->db_sid(), time_util::now);
    mail_module::do_notify_haved_new_mail(player, 1);
  }
}
int recharge_module::do_recharge_rebate(player_obj *player,
                                        const int has_buy_cnt,
                                        const int rc_type)
{
  if (rc_type == RECHARGE_MONTHLY_CARD
      || rc_type == 0)
    return 0;

  recharge_cfg_obj *rco = recharge_config::instance()->get_recharge_obj(rc_type);
  if (rco == NULL)
  {
    e_log->rinfo("rebate failed! player=%d rc_type=%d", player->id(), rc_type);
    return -1;
  }

  int b_diamond = rco->b_diamond_2_;
  if (has_buy_cnt < rco->limit_cnt_)
    b_diamond = rco->b_diamond_1_;
  if (b_diamond > 0)
  {
    player->do_got_money(b_diamond, M_BIND_DIAMOND, MONEY_GOT_RECHARGE_REBATE, 0);

    const mail_obj *mo
      = mail_config::instance()->get_mail_obj(mail_config::RECHARGE_REBATE);
    if (mo != NULL)
    {
      MAKE_REPLACE_STR(0, MAIL_P_NUMBER, "%s(%d)", STRING_NUMBER, b_diamond);
      char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
      mail_module::replace_mail_info(mo->content_,
                                     mail_module::replace_str,
                                     mail_module::replace_value,
                                     1,
                                     content);
      mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                                mo->sender_name_, mo->title_, content,
                                mail_info::MAIL_TYPE_GM,
                                0, 0, 0, 0, NULL,
                                player->db_sid(), time_util::now);
      mail_module::do_notify_haved_new_mail(player, 1);
    }
  }
  return 0;
}
void recharge_module::do_send_mc_disabled_mail(player_obj *player)
{
  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::MC_DISABLED);
  if (mo != NULL)
  {
    char datetime[24] = {0};
    date_time d_time(player->last_buy_mc_time() + 3600 * 24 * MONTHLY_CARD_REBATE_DAYS);
    d_time.hour(0), d_time.min(0), d_time.sec(0);
    d_time.to_str(datetime, sizeof(datetime));
    MAKE_REPLACE_STR(0, MAIL_P_DATE_TIME, "%s(%s)", STRING_DATE_TIME, datetime);
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   1,
                                   content);
    mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                              mo->sender_name_, mo->title_, content,
                              mail_info::MAIL_TYPE_GM,
                              0, 0, 0, 0, NULL,
                              player->db_sid(), time_util::now);
    mail_module::do_notify_haved_new_mail(player, 1);
  }
}

int player_obj::notify_recharge_info()
{
  out_stream os(client::send_buf, client::send_buf_len);
  if (this->char_recharge_ == NULL)
    os << char(1) << stream_ostr("{}", sizeof("{}") - 1);
  else
  {
    os << (recharge_module::can_buy_mc(this, time_util::now) ? char(1) : char(0))
      << stream_ostr(this->char_recharge_->data_,
                     ::strlen(this->char_recharge_->data_));
  }
  os << (BIT_ENABLED(this->yes_or_no(), char_extra_info::ST_GOT_FIRST_RECHARGE) ? char(1) : char(0));
  return this->send_request(NTF_RECHARGE_INFO, &os);
}
int player_obj::clt_get_trade_no(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  const char *trade_no = order_serial::instance()->alloc(this->id());
  os << stream_ostr(trade_no, ::strlen(trade_no));
  return this->send_respond_ok(RES_GET_TRADE_NO, &os);
}
