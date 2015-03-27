// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-06 17:37
 */
//========================================================================

#ifndef JING_JI_MODULE_H_
#define JING_JI_MODULE_H_

#include "ilist.h"

// Forward declarations
class player_obj;
class in_stream;
class out_stream;
class jing_ji_rank;
class coord_t;

/**
 * @class jing_ji_module
 * 
 * @brief
 */
class jing_ji_module
{
public:
  static void destroy(player_obj *player);
  static int  handle_db_get_other_skill_list_result(player_obj *player, in_stream &is);
  static int  handle_db_get_jing_ji_log_result(player_obj *player, in_stream &is);
  static int  dispatch_msg(player_obj *, const int , const char *msg, const int len);

  static void on_after_login(player_obj *);
  static void on_load_on_rank(jing_ji_rank *jjr);
  static void on_char_lvl_up(player_obj *);
  static void on_jing_ji_end(const int char_id, const bool win);
  static void on_enter_scene(player_obj *);

  static void do_timeout(const int now);
  static void do_send_jing_ji_award(const int char_id, const int rank);
  static void do_build_n_rank_info(out_stream &os);
  static void do_notify_can_jing_ji(player_obj *player);

  static void get_char_list_before_rank(ilist<int> &char_list, const int n);
  static int  create_competitor(player_obj *player, const int competitor_id, coord_t &);
private:
  static void db_update_rank(const int , const int, const int );

  static int  clt_obtain_jing_ji_info(player_obj *, const char *, const int );
  static int  clt_refresh_competitor(player_obj *, const char *, const int );
  static int  clt_jing_ji_challenge(player_obj *, const char *, const int );
  static int  clt_jing_ji_clean_time(player_obj *, const char *, const int );
  static int  clt_get_jing_ji_log_list(player_obj *, const char *, const int );

  static void get_refresh_competitor_result(const int my_rank, int refresh_rank_result[4]);
  static void get_max_rank_award(player_obj *player);
  static int  can_jing_ji(player_obj *player, const int rank, const int competitor_id);
  static int  can_jing_ji(player_obj *player);
  static void insert_jing_ji_log_2_db(player_obj *player,
                                      const int competitor_id,
                                      const int rank_change);
private:
  static bool today_send_award;
};

#endif // JING_JI_MODULE_H_

