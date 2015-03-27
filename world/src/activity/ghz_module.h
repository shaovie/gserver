// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-20 12:25
 */
//========================================================================

#ifndef GHZ_MODULE_H_
#define GHZ_MODULE_H_

#define GHZ_SHOU_WEI_CNT   6

// Forward declarations
class player_obj;
class out_stream;

/**
 * @class ghz_module
 * 
 * @brief
 */
class ghz_module
{
public:
  static int parse_cfg_param(const char *param);

  static int can_transfer_to(player_obj *player, const int to_scene_cid);
  static int can_buy_item(player_obj *player, const int npc_cid);

  static void on_char_login(player_obj *player);
  static void on_enter_game(player_obj *);
  static void on_char_lvl_up(player_obj *);
  static void on_transfer_scene(player_obj *, const int);
  static void on_kill_somebody(player_obj *);
  static void on_wang_zuo_activated(const int id);
  static void on_wang_zuo_dead(const int killer_id);
  static void on_shou_wei_dead(const int shou_wei_id, const int shou_wei_cid, const int killer_id);
  static void on_shou_wei_live(player_obj *, const int , const int );
  static void on_ghz_open();
  static void on_ghz_over(const int , const int);

  static int  do_enter_ghz(player_obj *);
  static int  do_activate_shou_wei(player_obj *, const int);
  static int  do_obtain_fighting_info(player_obj *, out_stream &);
  static void do_calc_world_arerage_zhan_li(int &hp, int &gong_ji, int &fang_yu);
private:
  static void do_give_winner_award(const int winner_id);
};

#endif // GHZ_MODULE_H_

