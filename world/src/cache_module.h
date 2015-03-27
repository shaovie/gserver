// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-04-17 11:23
 */
//========================================================================

#ifndef CACHE_MODULE_H_
#define CACHE_MODULE_H_

#include "ilist.h"

// Forward declarations
class player_obj;
class item_obj;

/**
 * @class cache_module
 *
 * @brief
 */
class cache_module
{
public:
  static void equip_cache_add_player(const int, ilist<item_obj*> *);
  static void equip_cache_update_player(player_obj *);
  static ilist<item_obj*> * get_player_equip(const int);

public:
  static void on_char_logout(player_obj *);
};

#endif // CACHE_MODULE_H_

