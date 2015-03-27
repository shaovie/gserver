// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-03-20 10:52
 */
//========================================================================

#ifndef VIP_MODULE_H_
#define VIP_MODULE_H_

#include "vip_config.h"

// Forward declarations
class player_obj;
class in_stream;
class out_stream;

/**
 * @class vip_module
 *
 * @brief
 */
class vip_module
{
public:
  static int handle_db_get_vip_result(player_obj *player, in_stream &is);
  static int dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len);
  static void destroy(player_obj *player);
public:
  static char vip_lvl(player_obj *);
  static void on_after_login(player_obj *);
  static void on_char_lvl_up(player_obj *);
  static void on_lose_money(player_obj *, const char , const int , const int);
  static void on_recharge_ok(player_obj *, const int val);
  static void on_new_day(player_obj *, const bool when_login);

  static int  can_buy_item(player_obj *, const int );
  static int  to_get_effect_left(player_obj *, const char vip_effect_id);
  static bool do_cost_effect(player_obj *, const char vip_effect_id, const int val = 1);
  static void do_add_vip_exp(player_obj *player, const int exp);
public:
  static void do_cost_guild_ju_bao_cnt(player_obj *);
  static void do_cost_buy_ti_li_cnt(player_obj *);
  static void do_cost_exchange_coin(player_obj *);
  static void do_cost_free_relive(player_obj *);
  static int  to_get_left_guild_ju_bao_cnt(player_obj *);
  static int  to_get_passive_skill_add_rate(player_obj *);
  static int  to_get_tili_add_upper_limit(player_obj *);
  static int  to_get_tui_tu_add_exp(player_obj *);
  static int  to_get_tui_tu_free_turn_cnt(player_obj *);
  static int  to_get_equip_strengthen_add_rate(player_obj *);
  static int  to_get_add_zhan_xing_value(player_obj *);
  static int  to_get_add_friend_amt(player_obj *);
  static int  to_get_bao_shi_exp_per(player_obj *);
  static int  to_get_day_jing_ji_cnt(player_obj *);
  static int  to_get_add_worship_cnt(player_obj *);
  static int  to_get_left_buy_ti_li_cnt(player_obj *);
  static int  to_get_had_buy_ti_li_cnt(player_obj *);
  static int  to_get_left_exchange_coin_cnt(player_obj *);
  static int  to_get_had_exchange_coin_cnt(player_obj *);
  static int  to_get_left_free_relive_cnt(player_obj *);
  static int  to_get_had_free_relive_cnt(player_obj *);
  static int  to_get_jingli_add_upper_limit(player_obj *);
private:
  static bool if_vip_lvl_award_get(player_obj *, const char vip_lvl);
  static void do_set_vip_lvl_award_get(player_obj *, const char vip_lvl);
  static void do_notify_vip_2_client(player_obj *player);
  static void do_fetch_vip_lvl_award(player_obj *player, out_stream &);
  static void do_insert_vip_2_db(player_obj *player);
  static void do_update_vip_2_db(player_obj *player);
  static void do_be_vip(player_obj *player);
  static void do_award_title(player_obj *player, const char vip_lvl);
private:
  static void on_be_vip(player_obj *);
  static void on_vip_lvl_up(player_obj *);
private:
  static int clt_get_lvl_award(player_obj *, const char *, const int );
  static int clt_exchange_coin(player_obj *, const char *, const int );
};
#endif  // VIP_MODULE_H_
