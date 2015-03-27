// -*- C++ -*-

//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-03 11:10
 */
//========================================================================

#ifndef TRADE_MODULE_H_
#define TRADE_MODULE_H_

#include "ilist.h"
#include "global_macros.h"

// Forward declarations
class player_obj;
class out_stream;
class in_stream;
class item_obj;

/**
 * @class trade_info
 *
 * @brief
 */
class trade_info
{
public:
  class trade_item
  {
  public:
    trade_item() :
      item_amt_(0),
      item_id_(0)
    { }
  public:
    int item_amt_;
    int item_id_;
  };
  enum trade_state
  {
    TRADE_ST_NULL    = 0, // NULL
    TRADE_ST_TRADE   = 1, // 交易中
    TRADE_ST_LOCK    = 2, // 锁定中
    TRADE_ST_CONFIRM = 3, // 确认中
  };
public:
  trade_info();
  ~trade_info();
  void modify_item(const int item_id, const int amt);
  int item_amt(const int item_id);
private:
  trade_item *find_item(const int item_id);
public:
  char state_;
  int  opp_id_;
  int  coin_;
  int  diamond_;
  ilist<trade_item *> item_list_;
};
/**
 * @class trade_module
 *
 * @brief player and player trade
 */
class trade_module
{
public:
  static int  dispatch_msg(player_obj *, const int , const char *, const int );
  static void on_char_logout(player_obj *);
private:
  // = client
  static int clt_trade_invite(player_obj *, const char *msg, const int len);
  static int clt_trade_agree(player_obj *, const char *msg, const int len);
  static int clt_trade_refuse(player_obj *, const char *msg, const int len);
  static int clt_trade_modify_item(player_obj *, const char *msg, const int len);
  static int clt_trade_modify_money(player_obj *, const char *msg, const int len);
  static int clt_trade_lock(player_obj *, const char *msg, const int len);
  static int clt_trade_confirm(player_obj *, const char *msg, const int len);
  static int clt_trade_cancel(player_obj *, const char *msg, const int len);
private:
  static int do_complete_trade(player_obj *);
  static int do_cancel_trade(player_obj *);
};

#endif // TRADE_MODULE_H_
