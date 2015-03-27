// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-20 10:54
 */
//========================================================================

#ifndef ATTR_MODULE_H_
#define ATTR_MODULE_H_

// Forward declarations
class player_obj;

/**
 * @class attr_module
 * 
 * @brief
 */
class attr_module
{
public:
  static int do_calc_zhan_li(player_obj *player);
  static void on_all_qh_update(player_obj *player);
  static void on_all_equip_color_update(player_obj *player);

  static short get_equip_all_qh_lvl(player_obj *player);
  static int get_equip_all_color(player_obj *player);
};

#endif // ATTR_MODULE_H_

