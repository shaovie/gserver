#include "trade_module.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "ilist.h"
#include "char_info.h"
#include "package_module.h"
#include "item_obj.h"
#include "behavior_id.h"
#include "db_proxy.h"
#include "istream.h"
#include "global_macros.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "util.h"
#include "all_char_info.h"
#include "client.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("trade");
static ilog_obj *e_log = err_log::instance()->get_ilog("trade");

trade_info::trade_info()
: state_(trade_info::TRADE_ST_NULL),
  opp_id_(0),
  coin_(0),
  diamond_(0)
{ }
trade_info::~trade_info()
{
  while (!this->item_list_.empty())
  {
    trade_item *ti = this->item_list_.pop_front();
    if (ti != NULL) delete ti;
  }
}
void trade_info::modify_item(const int item_id, const int amt)
{
  if (amt <= 0) return ;
  trade_item *ti = this->find_item(item_id);
  if (ti == NULL)
  {
    ti = new trade_item();
    this->item_list_.push_back(ti);
    ti->item_id_ = item_id;
  }
  ti->item_amt_ = amt;
}
int trade_info::item_amt(const int item_id)
{
  trade_item *ti = this->find_item(item_id);
  if (ti == NULL) return 0;
  return ti->item_amt_;
}
trade_info::trade_item *trade_info::find_item(const int item_id)
{
  ilist_node<trade_item *> *iter = this->item_list_.head();
  for (; iter != NULL; iter = iter->next_)
  {
    if (iter->value_->item_id_ == item_id)
      return iter->value_;
  }
  return NULL;
}
int trade_module::dispatch_msg(player_obj *player,
                               const int msg_id,
                               const char *msg,
                               const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = trade_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_TRADE_INVITE,       clt_trade_invite);
    SHORT_CODE(REQ_TRADE_AGREE,        clt_trade_agree);
    SHORT_CODE(REQ_TRADE_REFUSE,       clt_trade_refuse);
    SHORT_CODE(REQ_TRADE_MODIFY_ITEM,  clt_trade_modify_item);
    SHORT_CODE(REQ_TRADE_MODIFY_MONEY, clt_trade_modify_money);
    SHORT_CODE(REQ_TRADE_LOCK,         clt_trade_lock);
    SHORT_CODE(REQ_TRADE_CONFIRM,      clt_trade_confirm);
    SHORT_CODE(REQ_TRADE_CANCEL,       clt_trade_cancel);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int trade_module::clt_trade_invite(player_obj *player, const char *msg, const int len)
{
  int target_id = 0;
  in_stream is(msg, len);
  is >> target_id;

  // check
  if (player->trade_info_ != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_SELF_STATE_IN);

  player_obj *target = player_mgr::instance()->find(target_id);
  if (target == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);

  if (target->trade_info_ != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_OPP_STATE_IN);

  // send
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id() << stream_ostr(player->name(), ::strlen(player->name()));
  target->send_request(NTF_TRADE_INVITE, &os);
  return 0;
}
int trade_module::clt_trade_agree(player_obj *player, const char *msg, const int len)
{
  int target_id = 0;
  in_stream is(msg, len);
  is >> target_id;

  // check
  if (player->trade_info_ != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_SELF_STATE_IN);

  player_obj *target = player_mgr::instance()->find(target_id);
  if (target == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);

  if (target->trade_info_ != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_OPP_STATE_IN);

  // ok
  player->trade_info_ = new trade_info();
  player->trade_info_->state_ = trade_info::TRADE_ST_TRADE;
  player->trade_info_->opp_id_ = target_id;
  target->trade_info_ = new trade_info();
  target->trade_info_->state_ = trade_info::TRADE_ST_TRADE;
  target->trade_info_->opp_id_ = player->id();

  out_stream t_os(client::send_buf, client::send_buf_len);
  t_os << target_id
    << stream_ostr(target->name(), ::strlen(target->name()))
    << target->lvl()
    << target->career();
  target->send_request(NTF_TRADE_BEGIN, &t_os);

  out_stream p_os(client::send_buf, client::send_buf_len);
  p_os << player->id()
    << stream_ostr(player->name(), ::strlen(player->name()))
    << player->lvl()
    << player->career();
  player->send_request(NTF_TRADE_BEGIN, &p_os);
  return 0;
}
int trade_module::clt_trade_refuse(player_obj *player, const char *msg, const int len)
{
  int target_id = 0;
  in_stream in(msg, len);
  in >> target_id;

  // notice
  player_obj *target = player_mgr::instance()->find(target_id);
  if (target == NULL) return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  os << stream_ostr(target->name(), ::strlen(target->name()));
  player->send_request(NTF_TRADE_REFUSE, &os);
  return 0;
}
int trade_module::clt_trade_modify_item(player_obj *player, const char *msg, const int len)
{
  int amt = 0;
  int item_id = 0;
  in_stream in(msg, len);
  in >> item_id >> amt;

  // check
  if (player->trade_info_ == NULL
      || player->trade_info_->state_ != trade_info::TRADE_ST_TRADE)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  item_obj *item = package_module::find_item(player, item_id);
  if (item == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  if (amt <= 0 || item->amount_ < amt)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_AMOUNT_NOT_ENOUGH);

  if (item->bind_ == BIND_TYPE
      || item->pkg_ != PKG_PACKAGE)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_ITEM_NEED_IN_PACKAGE);

  // ok
  player->trade_info_->modify_item(item_id, amt);

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id();
  int old_amt = item->amount_;
  item->amount_ = amt;
  package_module::do_build_item_info(item, os);
  item->amount_ = old_amt;
  player->send_request(NTF_TRADE_MODIFY_ITEM, &os);

  player_obj *target = player_mgr::instance()->find(player->trade_info_->opp_id_);
  if (target != NULL) target->send_request(NTF_TRADE_MODIFY_ITEM, &os);
  return 0;
}
int trade_module::clt_trade_modify_money(player_obj *player, const char *msg, const int len)
{
  int coin = 0;
  int diamond = 0;
  in_stream in(msg, len);
  in >> diamond >> coin;

  // check
  if (player->trade_info_ == NULL
      || player->trade_info_->state_ != trade_info::TRADE_ST_TRADE
      || coin < 0 || diamond < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player_obj *target = player_mgr::instance()->find(player->trade_info_->opp_id_);
  if (target != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);

  int ret = player->is_money_enough(M_COIN, coin);
  if (ret != 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  ret = player->is_money_enough(M_DIAMOND, diamond);
  if (ret != 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  //
  player->trade_info_->coin_ = coin;
  player->trade_info_->diamond_ = diamond;

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id() << diamond << coin;
  target->send_request(NTF_TRADE_MODIFY_MONEY, &os);
  return player->send_request(NTF_TRADE_MODIFY_MONEY, &os);
}
int trade_module::clt_trade_lock(player_obj *player, const char *, const int )
{
  // check
  if (player->trade_info_ == NULL
      || player->trade_info_->state_ != trade_info::TRADE_ST_TRADE)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  //
  player->trade_info_->state_ = trade_info::TRADE_ST_LOCK;

  player_obj *target = player_mgr::instance()->find(player->trade_info_->opp_id_);
  if (target != NULL) target->send_request(NTF_TRADE_LOCK, NULL);
  return player->send_respond_ok(RES_TRADE_LOCK);
}
int trade_module::clt_trade_confirm(player_obj *player, const char *, const int )
{
  // check
  if (player->trade_info_ == NULL
      || player->trade_info_->state_ != trade_info::TRADE_ST_LOCK)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player_obj *target = player_mgr::instance()->find(player->trade_info_->opp_id_);
  if (target == NULL
      || target->trade_info_ == NULL
      || (target->trade_info_->state_ != trade_info::TRADE_ST_LOCK
          && target->trade_info_->state_ != trade_info::TRADE_ST_CONFIRM))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TRADE_OPP_NO_LOCK);

  //
  if (target->trade_info_->state_ == trade_info::TRADE_ST_CONFIRM)
  {
    const int ret = trade_module::do_complete_trade(player);
    if (ret != 0) trade_module::do_cancel_trade(player);
    return 0;
  }
  player->trade_info_->state_ = trade_info::TRADE_ST_CONFIRM;
  target->send_request(NTF_TRADE_LOCK, NULL);
  return 0;
}
int trade_module::do_complete_trade(player_obj *player)
{
  // check
  if (player->trade_info_ == NULL) return ERR_UNKNOWN;
  player_obj *target = player_mgr::instance()->find(player->trade_info_->opp_id_);
  if (target == NULL || target->trade_info_ == NULL) return ERR_UNKNOWN;

  // maybe target's send coin - player's recv coin  to judge limit
  int ret = player->is_money_enough(M_COIN, player->trade_info_->coin_);
  if (ret != 0) return ret;
  ret = player->is_money_enough(M_DIAMOND, player->trade_info_->diamond_);
  if (ret != 0) return ret;
  ret = player->is_money_enough(M_COIN, target->trade_info_->coin_);
  if (ret != 0) return ret;
  ret = player->is_money_enough(M_DIAMOND, target->trade_info_->diamond_);
  if (ret != 0) return ret;
  if (player->is_money_upper_limit(M_COIN, target->trade_info_->coin_)
      || player->is_money_upper_limit(M_DIAMOND, target->trade_info_->diamond_)
      || target->is_money_upper_limit(M_COIN, player->trade_info_->coin_)
      || target->is_money_upper_limit(M_DIAMOND, player->trade_info_->diamond_))
    return ERR_MONEY_UPPER_LIMIT;

  int insert_item_num = 0;
  for (ilist_node<trade_info::trade_item *> *iter = player->trade_info_->item_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    trade_info::trade_item *ti = iter->value_;
    if (ti == NULL) return ERR_UNKNOWN;
    item_obj *io = package_module::find_item(player, ti->item_id_);
    if (io == NULL || io->amount_ < ti->item_amt_) return ERR_UNKNOWN;
    g_item_amount_bind[0][insert_item_num] = io->cid_;
    g_item_amount_bind[1][insert_item_num] = ti->item_amt_;
    g_item_amount_bind[2][insert_item_num] = io->bind_;
    insert_item_num++;
  }
  if (package_module::would_be_full(target,
                                    PKG_PACKAGE,
                                    g_item_amount_bind[0],
                                    g_item_amount_bind[1],
                                    g_item_amount_bind[2],
                                    insert_item_num))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;
  insert_item_num = 0;
  for (ilist_node<trade_info::trade_item *> *iter = target->trade_info_->item_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    trade_info::trade_item *ti = iter->value_;
    if (ti == NULL) return ERR_UNKNOWN;
    item_obj *io = package_module::find_item(target, ti->item_id_);
    if (io == NULL || io->amount_ < ti->item_amt_) return ERR_UNKNOWN;
    g_item_amount_bind[0][insert_item_num] = io->cid_;
    g_item_amount_bind[1][insert_item_num] = ti->item_amt_;
    g_item_amount_bind[2][insert_item_num] = io->bind_;
    insert_item_num++;
  }
  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    g_item_amount_bind[0],
                                    g_item_amount_bind[1],
                                    g_item_amount_bind[2],
                                    insert_item_num))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  // ok
  while (!player->trade_info_->item_list_.empty())
  {
    trade_info::trade_item *ti = player->trade_info_->item_list_.pop_front();
    item_obj *item = package_module::find_item(player, ti->item_id_);
    int old_amt = item->amount_;
    item->amount_ = ti->item_amt_;
    package_module::do_insert_item(target, PKG_PACKAGE, item, ITEM_GOT_TRADE, player->id(), 0);
    item->amount_ = old_amt;
    package_module::do_remove_item(player, item, ti->item_amt_, ITEM_LOSE_TRADE, player->id());
  }
  while (!target->trade_info_->item_list_.empty())
  {
    trade_info::trade_item *ti = target->trade_info_->item_list_.pop_front();
    item_obj *item = package_module::find_item(target, ti->item_id_);
    int old_amt = item->amount_;
    item->amount_ = ti->item_amt_;
    package_module::do_insert_item(player, PKG_PACKAGE, item, ITEM_GOT_TRADE, player->id(), 0);
    item->amount_ = old_amt;
    package_module::do_remove_item(target, item, ti->item_amt_, ITEM_LOSE_TRADE, player->id());
  }
  if (player->trade_info_->coin_ > 0)
  {
    player->do_lose_money(player->trade_info_->coin_, M_COIN, MONEY_LOSE_TRADE, target->id(), 0, 0);
    target->do_got_money(player->trade_info_->coin_, M_COIN, MONEY_GOT_TRADE, player->id());
  }
  if (player->trade_info_->diamond_ > 0)
  {
    player->do_lose_money(player->trade_info_->diamond_, M_DIAMOND, MONEY_LOSE_TRADE, target->id(), 0, 0);
    target->do_got_money(player->trade_info_->diamond_, M_DIAMOND, MONEY_GOT_TRADE, player->id());
  }
  if (target->trade_info_->coin_ > 0)
  {
    target->do_lose_money(target->trade_info_->coin_, M_COIN, MONEY_LOSE_TRADE, player->id(), 0, 0);
    player->do_got_money(target->trade_info_->coin_, M_COIN, MONEY_GOT_TRADE, target->id());
  }
  if (target->trade_info_->diamond_ > 0)
  {
    target->do_lose_money(target->trade_info_->diamond_, M_DIAMOND, MONEY_LOSE_TRADE, player->id(), 0, 0);
    player->do_got_money(target->trade_info_->diamond_, M_DIAMOND, MONEY_GOT_TRADE, target->id());
  }
  return 0;
}
int trade_module::clt_trade_cancel(player_obj *player, const char *, const int )
{
  trade_module::do_cancel_trade(player);
  return 0;
}
int trade_module::do_cancel_trade(player_obj* player)
{
  if (player->trade_info_ == NULL) return 0;
  const int opp_id = player->trade_info_->opp_id_;
  delete player->trade_info_;
  player->trade_info_ = NULL;
  player_obj *target = player_mgr::instance()->find(opp_id);
  if (target == NULL || target->trade_info_ == NULL) return 0;
  delete target->trade_info_;
  target->trade_info_ = NULL;
  return 0;
}
void trade_module::on_char_logout(player_obj *player)
{ trade_module::do_cancel_trade(player); }
