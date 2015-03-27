// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-24 18:18
 */
//========================================================================

#ifndef ACTIVITY_MODULE_H_
#define ACTIVITY_MODULE_H_

// Forward declarations
class activity_obj;
class player_obj;

enum
{
  ACT_EV_GHZ_WANG_ZUO_ACTIVATED      = 100, // 
  ACT_EV_GHZ_WANG_ZUO_DEAD           = 101, // 
  ACT_EV_GHZ_SHOU_WEI_DEAD           = 102, // 
  ACT_EV_GHZ_SHOU_WEI_LIVE           = 103, // 
  ACT_EV_GHZ_GET_SHOU_WEI_INFO       = 104, // 
  ACT_EV_GHZ_OBTAIN_FIGHTING_INFO    = 105, // 

  ACT_EV_XSZC_ENTER_SCP              = 106, //
  ACT_EV_XSZC_ACT_INFO               = 107, //
  ACT_EV_XSZC_GROUP_POS              = 108, //
  ACT_EV_XSZC_EXIT_SCP               = 109, //
  ACT_EV_XSZC_END                    = 110, //
};
/**
 * @class activity_module
 * 
 * @brief
 */
class activity_module
{
public:
  static int init();
  static void destroy(player_obj *);
  static activity_obj *construct(const int act_id);

  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void on_char_login(player_obj *player);
  static void on_new_day(player_obj *player, const bool when_login);
  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *player);

  static int can_transfer_to(player_obj *, const int to_scene_cid);
  static void on_transfer_scene(player_obj *player,
                                const int from_scene_id,
                                const int from_scene_cid,
                                const int to_scene_id,
                                const int to_scene_cid);
private:
  static int clt_got_ti_li_award(player_obj *, const char *, const int);
  static int clt_got_month_card_award(player_obj *, const char *, const int);
  static int clt_enter_xszc(player_obj *, const char *, const int);
  static int clt_obtain_xszc_act_info(player_obj *, const char *, const int);
  static int clt_obtain_xszc_group_pos(player_obj *, const char *, const int);
  static int clt_get_daily_lucky_goods_draw_cnt(player_obj *, const char *, const int);
  static int clt_do_lucky_goods_draw(player_obj *, const char *, const int);
  static int clt_obtain_kai_fu_act_rank(player_obj *, const char *, const int);
  static int clt_obtain_kai_fu_act_state(player_obj *, const char *, const int);
};

#endif // ACTIVITY_MODULE_H_

