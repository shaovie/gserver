// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2013-12-13 17:20:00
 */
//========================================================================

#ifndef TEAM_MODULE_H_
#define TEAM_MODULE_H_

// Forward declarations
class player_obj;
class team_info;
class out_stream;

/**
 * @class team_module
 * 
 * @brief
 */
class team_module
{
public:
  // handle client msg
  static int dispatch_msg(player_obj *,
                          const int msg_id,
                          const char *msg,
                          const int len);

  static int clt_create_team(player_obj *, const char *, const int);
  static int clt_kick_member(player_obj *, const char *, const int);
  static int clt_quit_team(player_obj *, const char *, const int);
  static int clt_invite_be_a_team(player_obj *, const char *, const int);
  static int clt_agree_invite(player_obj *, const char *, const int);
  static int clt_refuse_invite(player_obj *, const char *, const int);

  static void on_char_login(player_obj *);
  static void on_char_logout(player_obj *);
  static void on_enter_game(player_obj *);

  static void on_join_team(player_obj *, const int );
  static void on_exit_team(player_obj *, const int );
private:
  static bool build_one_member_info(const int, const char, out_stream &);
  static void build_all_member_info(team_info *, out_stream &);
  static team_info* do_create_team(player_obj *, player_obj *);
  static void do_join_team(team_info *, player_obj *);
  static void do_disband_team(team_info *);
  static int  do_change_leader(team_info *);

  static void broadcast_to_members(team_info *, const int, const int);
};

#endif // TEAM_MODULE_H_

