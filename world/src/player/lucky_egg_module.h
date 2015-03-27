// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-16 12:02
 */
//========================================================================

#ifndef LUCKY_EGG_MODULE_H_
#define LUCKY_EGG_MODULE_H_

#include "ilist.h"

// Forward declarations
class item_obj;
class player_obj;
class item_amount_bind_t;
class bao_shi_info;

/**
 * @class lucky_egg_module
 *
 * @brief
 */
class lucky_egg_module
{
public:
  static void on_cheng_jiu_egg(player_obj *, ilist<item_amount_bind_t> &award);
  static void on_equip_strengthen_egg(player_obj *, const item_obj *, const int );
  static void on_passive_skill_egg(player_obj *, const int skill_cid, const int);
  static void on_bao_shi_egg(player_obj *);
  static void on_bao_shi_upgrade_lucky_egg(player_obj *player, const char pos, const short lvl);
};

#endif // LUCKY_EGG_MODULE_H_

