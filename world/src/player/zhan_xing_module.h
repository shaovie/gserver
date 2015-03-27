// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-19 13:32
 */
//========================================================================

#ifndef ZHAN_XING_MODULE_H_
#define ZHAN_XING_MODULE_H_

// Forward declarations
class player_obj;

/**
 * @class zhan_xing_module
 * 
 * @brief
 */
class zhan_xing_module
{
public:
  static void on_recharge_ok(player_obj *, const int);
  static void on_vip_lvl_up(player_obj *);
};

#endif // ZHAN_XING_MODULE_H_

