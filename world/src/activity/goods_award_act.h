// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-18 17:57
 */
//========================================================================

#ifndef GOODS_AWARD_ACT_H_
#define GOODS_AWARD_ACT_H_

#include "activity_obj.h"

// Forward declarations
class player_obj;

/**
 * @class goods_award_act
 * 
 * @brief
 */
class goods_award_act : public activity_obj
{
  typedef activity_obj super;
public:
  goods_award_act();
  virtual ~goods_award_act() { }

  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *player);
  static int do_lucky_goods_draw(player_obj *player);
};

#endif // GOODS_AWARD_ACT_H_

