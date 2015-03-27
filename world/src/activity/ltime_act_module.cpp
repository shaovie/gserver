#include "ltime_act_module.h"
#include "lucky_turn_module.h"
#include "lucky_turn_act.h"
#include "ltime_act.h"
#include "ltime_act_cfg.h"
#include "ltime_act_mgr.h"
#include "ltime_act_obj.h"
#include "ltime_recharge_award.h"
#include "player_obj.h"
#include "time_util.h"
#include "db_proxy.h"
#include "message.h"
#include "istream.h"
#include "sys_log.h"
#include "client.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("act");
static ilog_obj *e_log = err_log::instance()->get_ilog("act");

void ltime_act_module::destroy(player_obj *player)
{
  while (!player->ltime_recharge_award_list_.empty())
    delete player->ltime_recharge_award_list_.pop_front();
}
ltime_act_obj *ltime_act_module::construct(const int act_id,
                                           const int begin_time,
                                           const int end_time)
{
  ltime_act_obj *lao = NULL;
  if (act_id == LTIME_ACT_EXP)
    lao = new ltime_exp_act_obj(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_DROP)
    lao = new ltime_drop_act_obj(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_LUCKY_TURN)
    lao = new lucky_turn_act(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_ACC_RECHARGE)
    lao = new ltime_acc_recharge_act_obj(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_DAILY_RECHARGE)
    lao = new ltime_daily_recharge_act_obj(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_ACC_CONSUME)
    lao = new ltime_acc_consume_act_obj(act_id, begin_time, end_time);
  else if (act_id == LTIME_ACT_DAILY_CONSUME)
    lao = new ltime_daily_consume_act_obj(act_id, begin_time, end_time);
  return lao;
}
void ltime_act_module::db_update_act(const ltime_act *lta)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << stream_ostr((char *)lta, sizeof(ltime_act));
  db_proxy::instance()->send_request(0, REQ_UPDATE_LIMIT_TIME_ACT, &os);
}
void ltime_act_module::do_update_act(const ltime_act *lta, const bool preload)
{
  ltime_act_obj *lao = ltime_act_mgr::instance()->find(lta->act_id_);
  if (lao == NULL)
  {
    lao = ltime_act_module::construct(lta->act_id_,
                                      lta->begin_time_,
                                      lta->end_time_);
    if (lao == NULL) return ;
    ltime_act_mgr::instance()->insert(lta->act_id_, lao);
  }else
    lao->on_time_update(lta->begin_time_, lta->end_time_);

  if (!preload)
    ltime_act_module::db_update_act(lta);
}
int ltime_act_module::on_kill_mst_got_exp(const int exp)
{
  if (ltime_act_mgr::instance()->is_opened(LTIME_ACT_EXP))
    return 2*exp;
  return exp;
}
int ltime_act_module::on_drop_items()
{
  if (ltime_act_mgr::instance()->is_opened(LTIME_ACT_DROP))
    return 2;
  return 1;
}
void ltime_act_module::on_enter_game(player_obj *player)
{
  ltime_act_mgr::instance()->on_enter_game(player);

  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << player->id();
    db_proxy::instance()->send_request(player->id(), REQ_GET_LTIME_RECHARGE_AWARD, &os);
  }
}
void ltime_act_module::on_recharge_ok(player_obj *player)
{
  ltime_acc_recharge_act_obj::on_recharge_ok(player);
  ltime_daily_recharge_act_obj::on_recharge_ok(player);
}
void ltime_act_module::on_consume_diamond(player_obj *player)
{
  ltime_acc_consume_act_obj::on_consume_diamond(player);
  ltime_daily_consume_act_obj::on_consume_diamond(player);
}
int ltime_act_module::handle_db_get_ltime_recharge_award_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(ltime_recharge_award) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    ltime_recharge_award *p = (ltime_recharge_award *)bf;

    ltime_recharge_award *lra = new ltime_recharge_award();
    ::memcpy(lra, p, sizeof(ltime_recharge_award));
    ltime_act_module::do_insert_ltime_recharge_award(player, lra);
  }
  return 0;
}
void ltime_act_module::do_insert_ltime_recharge_award(player_obj *player, ltime_recharge_award *lra)
{ player->ltime_recharge_award_list_.push_back(lra); }
ilist<ltime_recharge_award *> &ltime_act_module::get_ltime_recharge_award(player_obj *player)
{ return player->ltime_recharge_award_list_; }
int ltime_act_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = ltime_act_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_LUCKY_TURN_ONE,           clt_lucky_turn_one);
    SHORT_CODE(REQ_LUCKY_TURN_TEN,           clt_lucky_turn_ten);
    SHORT_CODE(REQ_OBTAIN_LUCKY_TURN_INFO,   clt_obtain_lucky_turn_info);
    SHORT_CODE(REQ_OBTAIN_LUCKY_TURN_BIG_AWARD_LOG, clt_obtain_lucky_turn_big_award_log);
    SHORT_CODE(REQ_LTIME_ACC_RECHARGE_INFO,  clt_obtain_acc_recharge_info);
    SHORT_CODE(REQ_LTIME_DAILY_RECHARGE_INFO,clt_obtain_daily_recharge_info);
    SHORT_CODE(REQ_LTIME_ACC_CONSUME_INFO,   clt_obtain_acc_consume_info);
    SHORT_CODE(REQ_LTIME_DAILY_CONSUME_INFO, clt_obtain_daily_consume_info);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int ltime_act_module::clt_lucky_turn_one(player_obj *player, const char *, const int )
{
  if (!ltime_act_mgr::instance()->is_opened(LTIME_ACT_LUCKY_TURN))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ACTIVITY_NOT_OPENED);
  int ret = lucky_turn_module::do_turn(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int ltime_act_module::clt_lucky_turn_ten(player_obj *player, const char *, const int )
{
  if (!ltime_act_mgr::instance()->is_opened(LTIME_ACT_LUCKY_TURN))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ACTIVITY_NOT_OPENED);
  int ret = lucky_turn_module::do_ten_turn(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int ltime_act_module::clt_obtain_lucky_turn_info(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  int ret = lucky_turn_module::do_fetch_lucky_turn_info(player, os);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return player->send_respond_ok(RES_OBTAIN_LUCKY_TURN_INFO, &os);
}
int ltime_act_module::clt_obtain_lucky_turn_big_award_log(player_obj *player, const char *, const int )
{
  int ret = lucky_turn_module::do_fetch_big_award_log(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int ltime_act_module::clt_obtain_acc_recharge_info(player_obj *player, const char *, const int )
{
  int ret = ltime_acc_recharge_act_obj::acc_recharge(player);
  if (ret < 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  out_stream os(client::send_buf, client::send_buf_len);
  os << ret;
  return player->send_respond_ok(RES_LTIME_ACC_RECHARGE_INFO, &os);
}
int ltime_act_module::clt_obtain_daily_recharge_info(player_obj *player, const char *, const int )
{
  int ret = ltime_daily_recharge_act_obj::daily_recharge(player);
  if (ret < 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  out_stream os(client::send_buf, client::send_buf_len);
  os << ret;
  return player->send_respond_ok(RES_LTIME_DAILY_RECHARGE_INFO, &os);
}
int ltime_act_module::clt_obtain_acc_consume_info(player_obj *player, const char *, const int )
{
  int ret = ltime_acc_consume_act_obj::acc_consume(player);
  if (ret < 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  out_stream os(client::send_buf, client::send_buf_len);
  os << ret;
  return player->send_respond_ok(RES_LTIME_ACC_CONSUME_INFO, &os);
}
int ltime_act_module::clt_obtain_daily_consume_info(player_obj *player, const char *, const int )
{
  int ret = ltime_daily_consume_act_obj::daily_consume(player);
  if (ret < 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  out_stream os(client::send_buf, client::send_buf_len);
  os << ret;
  return player->send_respond_ok(RES_LTIME_DAILY_CONSUME_INFO, &os);
}
