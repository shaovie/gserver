// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-15 19:43
 */
//========================================================================

#ifndef MIS_AWARD_MODULE_H_
#define MIS_AWARD_MODULE_H_

#include "ilist.h"

// Forward declarations
class player_obj;
class item_amount_bind_t;

/**
 * @class mis_award_module
 *
 * @brief
 */
class mis_award_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );
  static void on_new_day(player_obj *player, const bool when_login);
  static void on_enter_game(player_obj *);
  static void on_char_lvl_up(player_obj *);
  static void do_notify_online_award_get_info(player_obj *);
private:
  static int clt_obtain_online_award(player_obj *, const char *, const int );
  static int clt_get_online_award(player_obj *, const char *, const int );
  static int clt_get_login_award(player_obj *, const char *, const int );
  static int clt_get_lvl_award(player_obj *, const char *, const int );
  static int clt_get_seven_day_login_award(player_obj *, const char *, const int );

  static void  do_notify_login_award_get_info(player_obj *);
  static void  do_notify_lvl_award_info(player_obj *);
  static short to_get_lvl_award_min_lvl(const short get_lvl);
  static void  do_gen_lvl_award(const short lvl, ilist<item_amount_bind_t> &award_list);
  static int   to_get_online_award_left_time(player_obj *);
  static void  do_notify_seven_day_login_info(player_obj *);
  static void  do_set_seven_day_login(player_obj *, const char day, const char st);
  static char  to_get_seven_day_login(player_obj *, const char day);
  static bool  if_need_notify_seven_day_login_to_clt(player_obj *);
};

#endif // MIS_AWARD_MODULE_H_
