#include "player_obj.h"
#include "behavior_log.h"
#include "account_info.h"
#include "global_param_cfg.h"
#include "all_char_info.h"
#include "time_util.h"
#include "char_info.h"
#include "mblock_pool.h"
#include "consume_log.h"
#include "istream.h"
#include "sys_log.h"
#include "message.h"
#include "error.h"
#include "def.h"
#include "db_proxy.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "ltime_act_module.h"

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

int player_obj::is_money_enough(const int money_type, const int value)
{
  if (money_type == M_DIAMOND)
  {
    if (this->diamond() < value)
      return ERR_DIAMOND_NOT_ENOUGH;
  }else if (money_type == M_BIND_DIAMOND)
  {
    if (this->b_diamond() < value)
      return ERR_B_DIAMOND_NOT_ENOUGH;
  }else if (money_type == M_COIN)
  {
    if (this->coin() < value)
      return ERR_COIN_NOT_ENOUGH;
  }else if (money_type == M_BIND_UNBIND_DIAMOND)
  {
    if (this->b_diamond() +  this->diamond() < value)
      return ERR_DIAMOND_NOT_ENOUGH;
  }else if (money_type == V_JING_JI_SCORE)
  {
    if (this->jing_ji_score() < value)
      return ERR_JING_JI_SCORE_NOT_ENOUGH;
  }else if (money_type == V_XSZC_HONOR)
  {
    if (this->xszc_honor() < value)
      return ERR_XSZC_HONOR_NOT_ENOUGH;
  }else
    return -1;
  return 0;
}
bool player_obj::is_money_upper_limit(const int money_type, const int value)
{
  if (money_type == M_DIAMOND)
  {
    if (this->diamond() + value > MONEY_UPPER_LIMIT)
      return true;
  }else if (money_type == M_BIND_DIAMOND)
  {
    if (this->b_diamond() + value > MONEY_UPPER_LIMIT)
      return true;
  }else if (money_type == M_COIN)
  {
    if (this->coin() + value > MONEY_UPPER_LIMIT)
      return true;
  }else
    return true;
  return false;
}
void player_obj::do_got_money(const int value,
                              const int money_type,
                              const int behavior_sub_type,
                              const int src_id)
{
  if (value <= 0) return ;
  if (this->is_money_upper_limit(money_type, value))
  {
    e_log->rinfo("forget check money! who !!!!!!!!!!!!!!!%d", behavior_sub_type);
    this->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);
    return ;
  }
  int sum_value = 0;
  switch (money_type)
  {
  case M_DIAMOND:
    this->ac_info_->diamond_ += value;
    this->db_save_account_info();
    sum_value = this->ac_info_->diamond_;
    break;
  case M_BIND_DIAMOND:
    this->char_info_->b_diamond_ += value;
    this->db_save_char_info();
    sum_value = this->char_info_->b_diamond_;
    break;
  case M_COIN:
    this->char_info_->coin_ += value;
    this->db_save_char_info();
    sum_value = this->char_info_->coin_;
    cheng_jiu_module::on_coin_value(this, this->char_info_->coin_);
    break;
  default:
    e_log->rinfo("unknow got money_type %d behavior %d", money_type, behavior_sub_type);
    return ;
  }

  this->do_notify_money_to_clt();

  behavior_log::instance()->store(BL_MONEY_GOT,
                                  time_util::now,
                                  "%d|%d|%d|%d|%d|%d",
                                  this->id(),
                                  sum_value,
                                  money_type,
                                  value,
                                  behavior_sub_type,
                                  src_id);
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
  *mb << money_type << value;
  this->post_aev(AEV_GOT_MONEY, mb);
}
void player_obj::do_got_item_money(const int cid,
                                   const int amount,
                                   const int behavior_sub_type,
                                   const int src)
{
  if (cid == global_param_cfg::item_coin_cid)
    this->do_got_money(amount, M_COIN, behavior_sub_type, src);
  else if (cid == global_param_cfg::item_diamond_cid)
    this->do_got_money(amount, M_DIAMOND, behavior_sub_type, src);
  else if (cid == global_param_cfg::item_bind_diamond_cid)
    this->do_got_money(amount, M_BIND_DIAMOND, behavior_sub_type, src);
}
void player_obj::do_exchange_item_money(const int cid,
                                        const int amount,
                                        int &coin,
                                        int &diamond,
                                        int &b_diamond)
{
  if (cid == global_param_cfg::item_coin_cid)
    coin += amount;
  else if (cid == global_param_cfg::item_diamond_cid)
    diamond += amount;
  else if (cid == global_param_cfg::item_bind_diamond_cid)
    b_diamond += b_diamond;
}
void player_obj::do_lose_money(const int value,
                               const int money_type,
                               const int behavior_sub_type,
                               const int src_id_1,
                               const int src_id_2,
                               const int src_id_2_cnt)
{
  if (value <= 0) return ;
  int left_value = 0;
  switch (money_type)
  {
  case M_DIAMOND:
    this->ac_info_->diamond_ -= value;
    this->db_save_account_info();
    left_value = this->ac_info_->diamond_;
    this->do_record_lose_diamond(value, behavior_sub_type);
    break;
  case M_BIND_DIAMOND:
    this->char_info_->b_diamond_ -= value;
    this->db_save_char_info();
    left_value = this->char_info_->b_diamond_;
    break;
  case M_COIN:
    this->char_info_->coin_ -= value;
    this->db_save_char_info();
    left_value = this->char_info_->coin_;
    break;
  case M_BIND_UNBIND_DIAMOND:
    {
      const int c = this->b_diamond() - value;
      if (c < 0)
      {
        this->do_lose_money(this->b_diamond(),
                            M_BIND_DIAMOND,
                            behavior_sub_type,
                            src_id_1,
                            src_id_2,
                            src_id_2_cnt);
        this->do_lose_money((-c),
                            M_DIAMOND,
                            behavior_sub_type,
                            src_id_1,
                            src_id_2,
                            src_id_2_cnt);
      }else
        this->do_lose_money(value,
                            M_BIND_DIAMOND,
                            behavior_sub_type,
                            src_id_1,
                            src_id_2,
                            src_id_2_cnt);
      return ;
    }
  case V_JING_JI_SCORE:
    this->char_extra_info_->jing_ji_score_ -= value;
    this->db_save_char_extra_info();
    this->do_notify_jing_ji_score_to_clt();
    return ;
  case V_XSZC_HONOR:
    this->char_extra_info_->xszc_honor_ -= value;
    this->db_save_char_extra_info();
    this->do_notify_xszc_honor_to_clt();
    return ;
  default:
    e_log->rinfo("unknow lose money_type %d behavior %d", money_type, behavior_sub_type);
    return ;
  }

  this->do_notify_money_to_clt();
  vip_module::on_lose_money(this, money_type, behavior_sub_type, value);

  behavior_log::instance()->store(BL_MONEY_LOSE,
                                  time_util::now,
                                  "%d|%d|%d|%d|%d|%d|%d|%d",
                                  this->id(),
                                  left_value,
                                  money_type,
                                  value,
                                  behavior_sub_type,
                                  src_id_1,
                                  src_id_2,
                                  src_id_2_cnt);
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
  *mb << money_type << value;
  this->post_aev(AEV_LOSE_MONEY, mb);
}
void player_obj::do_notify_money_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->diamond()
    << this->b_diamond()
    << this->coin();
  this->do_delivery(NTF_UPDATE_MONEY, &mb);
}
void player_obj::do_record_lose_diamond(const int value, const int behavior_sub_type)
{
  if (global_param_cfg::diamond_not_cs_type_set.find(behavior_sub_type))
    return ;

  consume_log cl;
  cl.char_id_ = this->id();
  cl.cs_type_ = behavior_sub_type;
  cl.cs_time_ = time_util::now;
  cl.value_   = value;

  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr so((char *)&cl, sizeof(consume_log));
  os << this->db_sid() << so;
  db_proxy::instance()->send_request(this->id(), REQ_INSERT_CONSUME_LOG, &os);

  char_brief_info::on_consume_diamond(cl.char_id_,
                                      behavior_sub_type,
                                      time_util::now,
                                      value);

  ltime_act_module::on_consume_diamond(this);
}
