// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-13 13:40
 */
//========================================================================

#ifndef LTIME_ACT_MODULE_H_
#define LTIME_ACT_MODULE_H_

#include "ilist.h"

// Forward declarations
class ltime_act;
class player_obj;
class in_stream;
class ltime_act_obj;
class ltime_recharge_award;

#define LTIME_ACT_SET_LIMITED_TIME        5

/**
 * @class ltime_act_module
 * 
 * @brief
 */
class ltime_act_module
{
public:
  static void destroy(player_obj *player);
  static int dispatch_msg(player_obj *, const int , const char *, const int );
  static int handle_db_get_ltime_recharge_award_result(player_obj *player, in_stream &is);

  static void do_update_act(const ltime_act *, const bool preload);
  static void do_insert_ltime_recharge_award(player_obj *, ltime_recharge_award *lra);
  static ilist<ltime_recharge_award *> &get_ltime_recharge_award(player_obj *);

  static void on_enter_game(player_obj *player);
  static int on_kill_mst_got_exp(const int );
  static int on_drop_items();
  static void on_recharge_ok(player_obj *player);
  static void on_consume_diamond(player_obj *player);
private:
  static ltime_act_obj *construct(const int act_id,
                                  const int begin_time,
                                  const int end_time);
  static void db_update_act(const ltime_act *lta);

  static int clt_lucky_turn_one(player_obj *, const char *, const int );
  static int clt_lucky_turn_ten(player_obj *, const char *, const int );
  static int clt_obtain_lucky_turn_info(player_obj *, const char *, const int );
  static int clt_obtain_lucky_turn_big_award_log(player_obj *, const char *, const int );
  static int clt_obtain_acc_recharge_info(player_obj *, const char *, const int );
  static int clt_obtain_daily_recharge_info(player_obj *, const char *, const int );
  static int clt_obtain_acc_consume_info(player_obj *, const char *, const int );
  static int clt_obtain_daily_consume_info(player_obj *, const char *, const int );
};

#endif // LTIME_ACT_MODULE_H_

