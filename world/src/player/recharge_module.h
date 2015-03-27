// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-24 10:39
 */
//========================================================================

#ifndef RECHARGE_MODULE_H_
#define RECHARGE_MODULE_H_

// Forward declarations
class player_obj;
class out_stream;

/**
 * @class recharge_module
 * 
 * @brief
 */
class recharge_module
{
public:
  static bool can_buy_mc(player_obj *player, const int now);
  static void do_recharge(player_obj *player,
                          const int diamond,
                          const int rc_type,
                          const int rmb,
                          const char *orderid, const char *platform);

  static void do_send_mc_disabled_mail(player_obj *player);
private:
  static int  do_recharge_rebate(player_obj *player,
                                 const int has_buy_cnt,
                                 const int rc_type);
  static void on_recharge_ok(player_obj *player,
                             const int diamond,
                             const int rc_type,
                             const int rmb,
                             const char *orderid, const char *platform);
};

#endif // RECHARGE_MODULE_H_

