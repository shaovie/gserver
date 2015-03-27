// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-08-05 15:01
 */
//========================================================================

#ifndef WATER_TREE_MODULE_H_
#define WATER_TREE_MODULE_H_

class water_tree_info;
class player_obj;
class water_tree_obj;

/**
 * @class water_tree_module
 *
 * @brief
 */
class water_tree_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static int clt_get_water_time(player_obj *, const char *, const int );
  static int clt_water_tree(player_obj *, const char *, const int );

  static void on_enter_game(player_obj *player);
private:
  static void tree_level_up(player_obj *player, water_tree_obj *wto);
public:
  static water_tree_info wti;
};

#endif  // WATER_TREE_MODULE_H_
