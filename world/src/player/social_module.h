// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2013-12-16 11:30
 */
//========================================================================

#ifndef SOCIAL_MODULE_H_
#define SOCIAL_MODULE_H_

// Forward declarations
class player_obj;
class social_relation;
class in_stream;
class out_stream;
class char_brief_info;
class social_info;

#define MAX_ONCE_SOCIAL_OPERATE                 32

/**
 * @class social_module
 *
 * @brief
 */
class social_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void destroy(player_obj *player);

  //= CLIENT REQUEST
  static int clt_obtain_social_list(player_obj *player, const char *, const int );
  static int clt_search_social_by_name(player_obj *player, const char *msg, const int len);

  // friend
  static int clt_add_friend(player_obj *player, const char *msg, const int len);
  static int clt_remove_friend(player_obj *player, const char *msg, const int len);

  // black
  static int clt_add_black(player_obj *player, const char *msg, const int len);
  static int clt_remove_black(player_obj *player, const char *msg, const int len);

  //= OTHER
  static int handle_db_get_social_list_result(player_obj *player, in_stream &is);
  static int handle_db_get_search_list_result(player_obj *player, in_stream &is);
  static int handle_db_get_other_socialer_result(player_obj *player, in_stream &is);

  static void on_enter_game(player_obj *player);
  static void on_char_logout(player_obj *player);
  static void on_char_lvl_up(player_obj *); 
  static void on_char_zhan_li_update(player_obj *); 

  static bool is_friend(player_obj *player, const int char_id);
  static bool is_black(player_obj *player, const int char_id);
private:
  static int do_send_client_all_social_info(player_obj *player);

  static int add_friend_i(player_obj *player, const int inviter_id);
  static int remove_friend_i(player_obj *player, const int friend_id);
  static int agree_friend_invite_i(player_obj *player, const int other_id);
  static int add_black_i(player_obj *player, const int black_id);
  static void do_add_socialer(player_obj *player, const int , const char );
  static void do_remove_socialer(player_obj *player, const int , const char );

  static void do_insert_socialer(player_obj *, social_info *si);
  static void do_erase_socialer(player_obj *, const int socialer_id, const char relation);

  static void on_social_list_load_ok(player_obj *);

  static void do_notify_socialer_lvl_up(player_obj *player, const int socialer_id);
  static void do_notify_socialer_zhan_li_update(player_obj *player, const int socialer_id);
  static void do_notify_socialer_my_state(player_obj *player,
                                          const int socialer_id,
                                          const char state);
};

#endif  // SOCIAL_MODULE_H_
