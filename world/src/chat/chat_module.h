// -*- C++ -*-

//========================================================================
/**
 * Author   : jsonwu 
 * Date     : 2013-06-12 15:09
 */
//========================================================================
#ifndef CHAT_MODULE_H_
#define CHAT_MODULE_H_

#include "def.h"

// Forward declarations

class out_stream;
class player_obj;
class time_value;

class chat_module
{
public:
  static int  do_timeout(const time_value &now);

  static int  dispatch_msg(player_obj *, const int , const char *, const int );

  static void on_enter_game(player_obj *player);
  static void on_char_logout(player_obj *player);

  static void on_join_team(const int char_id, const int team_id);
  static void on_exit_team(const int char_id, const int team_id);

  static void on_join_guild(const int char_id, const int guild_id);
  static void on_exit_guild(const int char_id, const int guild_id);
private:
  static int clt_world_chat(player_obj *player, const char *msg, const int len);
  static int clt_guild_chat(player_obj *player, const char *msg, const int len);
  static int clt_team_chat(player_obj *player, const char *msg, const int len);
  static int clt_private_chat(player_obj *player, const char *msg, const int len);
  static int clt_flaunt_item(player_obj *player, const char *msg, const int len);
  static int clt_get_flaunt_item_info(player_obj *player, const char *msg, const int len);
private:
  static int channel_chat(player_obj *player,
                          const char *msg,
                          const int len,
                          const int channel_id,
                          const int arg,
                          const int ntf_msg_id);

  static int can_chat(player_obj *player, const int channel_id);
  static void on_chat_ok(player_obj *player, const int channel_id);

  static int send_to_char(const int, const int, out_stream &);
};

#endif 
