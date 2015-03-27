// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-03 20:13
 */
//========================================================================

#ifndef SCP_MODULE_H_
#define SCP_MODULE_H_

// Forward declarations
class player_obj;
class sub_scp_obj;
class scp_obj;
class scp_log;
class in_stream;
class out_stream;

enum
{
  SCP_EV_REMOVE_GUILD        = 101,
  SCP_EV_GUILD_SUMMON_BOSS   = 102,
  SCP_EV_GUILD_CHAR_EXIT     = 103,
  SCP_EV_JING_JI_MST         = 104,
  SCP_EV_CHAR_DEAD           = 106,
  SCP_EV_GUILD_SCP_MST_REACH = 107,
  SCP_EV_GUILD_INIT_SCP      = 108,
  SCP_EV_GUILD_SCP_CAN_ENTER = 109,
  SCP_EV_GUILD_SCP_MST_EXIT  = 110,
  SCP_EV_XSZC_BARRACK_DEAD   = 111,
  SCP_EV_XSZC_MAIN_DEAD      = 112,
  SCP_EV_XSZC_DEFENDER_DEAD  = 113,
  SCP_EV_XSZC_ARM_DEAD       = 114,
  SCP_EV_KILL_SOMEBODY       = 115,
  SCP_EV_LUEDUO_FANPAI       = 116,
};

/**
 * @class scp_module
 *
 * @brief
 */
class scp_module
{
  friend class gm_cmd;
public:
  static void destroy(player_obj *player);
  static int handle_db_get_scp_log_result(player_obj *player, in_stream &is);
  static int dispatch_msg(player_obj *player,
                          const int msg_id,
                          const char *msg,
                          const int len);
private:
  static int clt_enter_scp(player_obj *, const char *, const int );
  static int clt_exit_scp(player_obj *, const char *, const int );
  static int clt_scp_end(player_obj *, const char *, const int );
  static int clt_in_scp_kill_mst(player_obj *player, const char *msg, const int len);
  static int clt_in_scp_relive(player_obj *player, const char *msg, const int len);
  static int clt_mushroom_scp_end(player_obj *player, const char *msg, const int len);
  static int clt_get_total_mstar(player_obj *player, const char *msg, const int len);

public: // 构建一个副本
  static scp_obj *construct_scp(const int scene_cid,
                                const int char_id,
                                const int team_id,
                                const int guild_id);

  static sub_scp_obj *construct_sub_scp(const int scene_cid);

  static void on_char_login(player_obj *player);
  static void on_new_day(player_obj *player, const bool);
  static void on_enter_game(player_obj *player);
  static void on_char_logout(player_obj *player);
  static void on_transfer_scene(player_obj *player,
                                const int from_scene_id,
                                const int from_scene_cid,
                                const int to_scene_id,
                                const int to_scene_cid);
  static void on_char_dead(player_obj *player, const int killer_id);
  static void on_kill_somebody(player_obj *player, const int char_id);
  static int  can_enter_scp(player_obj *player, const int scene_cid);
  static scp_log *find_scp_log(player_obj *, const int scene_cid);

  static void db_insert_scp_log(player_obj *player, scp_log *sl);
  static void db_update_scp_log(player_obj *player, scp_log *sl);
  static int  do_enter_single_scp(player_obj *, const int scene_cid);
private:
  static int  do_enter_team_scp(player_obj *, const int scene_cid);
  static void do_push_all_scp_list(player_obj *player);
};

#endif // SCP_MODULE_H_

