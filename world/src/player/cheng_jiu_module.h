// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-21 11:23
 */
//========================================================================

#ifndef CHENG_JIU_MODULE_H_
#define CHENG_JIU_MODULE_H_

// Forward declarations

class player_obj;
class cheng_jiu_info;
class in_stream;

enum
{
  CJ_LEVEL                   = 100,
  CJ_FIGHT_SCORE             = 101,

  CJ_ZHU_WU_QH               = 102,
  CJ_FU_WU_QH                = 103,
  CJ_YI_FU_QH                = 104,
  CJ_KU_ZI_QH                = 105,
  CJ_XIE_ZI_QH               = 106,
  CJ_SHOU_TAO_QH             = 107,
  CJ_XIANG_LIAN_QH           = 108,
  CJ_JIE_ZHI_QH              = 109,

  CJ_P_SK_HP_LVL             = 110,
  CJ_P_SK_GONG_JI_LVL        = 111,
  CJ_P_SK_FANG_YU_LVL        = 112,
  CJ_P_SK_MING_ZHONG_LVL     = 113,
  CJ_P_SK_SHAN_BI_LVL        = 114,
  CJ_P_SK_BAO_JI_LVL         = 115,
  CJ_P_SK_KANG_BAO_LVL       = 116,
  CJ_P_SK_SHANG_MIAN_LVL     = 117,

  CJ_FRIEND_COUNT            = 118,
  CJ_EQUIP_FEN_JIE           = 119,
  CJ_EQUIP_XI_LIAN           = 120,
  CJ_COMPLETE_TUI_TU         = 121,
  CJ_ZHAN_XING               = 122,
  CJ_GUILD_CONTRIB           = 123,
  CJ_KILL_CHAR               = 124,
  CJ_COIN_VALUE              = 125,
  CJ_VIP_LEVEL               = 126,
  CJ_ONLINE_TIME             = 127,
  CJ_CONTINUE_LOGIN_DAY      = 128,

  CJ_ZHU_WU_BAO_SHI          = 129,
  CJ_FU_WU_BAO_SHI           = 130,
  CJ_YI_FU_BAO_SHI           = 131,
  CJ_KU_ZI_BAO_SHI           = 132,
  CJ_XIE_ZI_BAO_SHI          = 133,
  CJ_SHOU_TAO_BAO_SHI        = 134,
  CJ_XIANG_LIAN_BAO_SHI      = 135,
  CJ_JIE_ZHI_BAO_SHI         = 136,

  CJ_SCP_3101                = 137,
  CJ_SCP_3102                = 138,
  CJ_SCP_3103                = 139,
  CJ_SCP_3104                = 140,

  CJ_JING_JI                 = 141,
  CJ_GUILD_JU_BAO            = 142,

  CJ_MO_BAI                  = 143,
  CJ_WATER_TREE              = 144,

  CJ_END
};

/**
 * @class cheng_jiu_module
 *
 * @brief
 */
class cheng_jiu_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void handle_db_get_cheng_jiu_result(player_obj *player, in_stream &is);
  static void init(player_obj *player);
  static void destroy(player_obj *player);

  static void on_enter_game(player_obj *player);

  static void on_char_lvl_up(player_obj *player);
  static void on_zhan_li_update(player_obj *player, const int score);
  static void on_equip_qh(player_obj *player, const int part, const int qh);
  static void on_passive_skill_level_up(player_obj *player, const int effect_id, const int lvl);
  static void on_friend_count(player_obj *player, const int count);
  static void on_equip_fen_jie(player_obj *player);
  static void on_equip_xi_lian(player_obj *player);
  static void on_complete_tui_tu(player_obj *player);
  static void on_zhan_xing(player_obj *player, const int count);
  static void on_guild_contrib(player_obj *player, const int contrib);
  static void on_kill_char(player_obj *player);
  static void on_coin_value(player_obj *player, const int coin);
  static void on_vip_level(player_obj *player, const int vip);
  static void on_online_time(player_obj *player, const int t);
  static void on_continue_login_day(player_obj *player, const int days);
  static void on_bao_shi_lvl(player_obj *player, const int part, const int lvl);
  static void on_enter_scp(player_obj *player, const int scene_cid);
  static void on_jing_ji(player_obj *player);
  static void on_guild_ju_bao(player_obj *player);
  static void on_mo_bai(player_obj *player);
  static void on_water_tree(player_obj *player);
private:
  static int clt_get_cheng_jiu_list(player_obj *player, const char *msg, const int len);
  static int clt_get_cheng_jiu_award(player_obj *player, const char *msg, const int len);

  static bool check_cheng_jiu_complete(player_obj *player, cheng_jiu_info *cji);
  static void notify_cheng_jiu_complete(player_obj *player, const int id);
  static void give_cheng_jiu_title(player_obj *, const int id);
  static cheng_jiu_info *find_cj_info(player_obj *player, const int type);
  static void update_cheng_jiu_2_db(player_obj *player, cheng_jiu_info *cji);
  static cheng_jiu_info *new_cheng_jiu(player_obj *player, const int type);
};

#endif // CHENG_JIU_MODULE_H_

