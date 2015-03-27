// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-16 18:19
 */
//========================================================================

#ifndef AWARD_TI_LI_ACT_H_
#define AWARD_TI_LI_ACT_H_

#include "activity_obj.h"

// Forward declarations
class player_obj;

/**
 * @class award_ti_li_act
 * 
 * @brief
 */
class award_ti_li_act : public activity_obj
{
  typedef activity_obj super;
public:
  award_ti_li_act();
  virtual ~award_ti_li_act() { }

  static int parse_cfg_param(const char *param);
  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *player);

  static int do_got_ti_li_award(player_obj *player);
public:
  static int award_ti_li;
};

#endif // AWARD_TI_LI_ACT_H_

