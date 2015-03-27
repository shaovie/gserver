// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-18 15:37
 */
//========================================================================

#ifndef GOODS_AWARD_MODULE_H_
#define GOODS_AWARD_MODULE_H_

// Forward declarations
class player_obj;
class water_tree_obj;

/**
 * @class goods_award_module
 * 
 * @brief
 */
class goods_award_module
{
public:
  static void on_preload_ok(const int, const int);

  static void on_guan_gai(player_obj *, water_tree_obj *);
};

#endif // GOODS_AWARD_MODULE_H_

