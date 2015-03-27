// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-05-04 16:31
 */
//========================================================================

#ifndef HUO_YUE_DU_MODULE_H_
#define HUO_YUE_DU_MODULE_H_

#define HUO_YUE_SCORE_CHANGE_ZX_VALUE         1000

class player_obj;
class in_stream;
class huo_yue_du_info;

/**
 * @class huo_yue_du_module
 *
 * @brief
 */
class huo_yue_du_module
{
public:
  static void init(player_obj *player);
  static void destroy(player_obj *player);

  static void on_enter_game(player_obj *player);
  static void on_new_day(player_obj *player);
  static void on_login_award(player_obj *player);
  static void on_equip_qh(player_obj *player);
  static void on_equip_fen_jie(player_obj *player);
  static void on_equip_xi_lian(player_obj *player);
  static void on_upgrade_bao_shi(player_obj *player);
  static void on_upgrade_passive_skill(player_obj *player);
  static void on_enter_scp(player_obj *player, const int scene_cid);
  static void on_jing_ji(player_obj *player);
  static void on_guild_ju_bao(player_obj *player);
  static void on_mo_bai(player_obj *player);
  static void on_water_tree(player_obj *player);

  static int handle_db_get_huo_yue_du_result(player_obj *player, in_stream &is);
  static void update_huo_yue_du_2_db(player_obj *player, huo_yue_du_info *hydi);
private:
  static void add_huo_yue_du_score(player_obj *player,
                                   huo_yue_du_info *hydi,
                                   const int cid);
};

#endif  // HUO_YUE_DU_MODULE_H_
