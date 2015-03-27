// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-17 14:37
 */
//========================================================================

#ifndef MONTH_CARD_AWARD_ACT_H_
#define MONTH_CARD_AWARD_ACT_H_

// Forward declarations
class player_obj;

/**
 * @class month_card_award_act
 * 
 * @brief
 */
class month_card_award_act
{
public:
  static int parse_cfg_param(const char *param);
  static void on_enter_game(player_obj *player);
  static void on_new_day(player_obj *player, const bool when_login);
  static void on_buy_month_card_ok(player_obj *player);

  static int do_got_month_card_award(player_obj *player);
public:
  static int daily_award;
};

#endif // MONTH_CARD_AWARD_ACT_H_

