// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-31 16:25
 */
//========================================================================

#ifndef MALL_MODULE_H_
#define MALL_MODULE_H_

// Forward declarations
class player_obj;

/**
 * @class mall_module
 * 
 * @brief
 */
class mall_module
{
public:
  static void destroy(player_obj *player);
  static void on_enter_game(player_obj *player);
  static void on_buy_item_ok(player_obj *player,
                             const int npc_cid,
                             const int item_cid,
                             const int amt,
                             const int param);
  static int can_buy_item(player_obj *, const int, const int , const int , const int );
  static int do_insert_mall_buy_log(player_obj *player,
                                    const int npc_cid,
                                    const int item_cid,
                                    const int amount,
                                    const int buy_time);
  static void baowu_mall_refresh(player_obj *player);
private:
  static int do_check_mall_limit_buy(player_obj *player, const int, const int );
};

#endif // MALL_MODULE_H_

