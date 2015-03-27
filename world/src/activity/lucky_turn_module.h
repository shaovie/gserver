// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-23 17:39
 */
//========================================================================

#ifndef LUCKY_TURN_MODULE_H_
#define LUCKY_TURN_MODULE_H_

// Forward declarations
class player_obj;
class in_stream;
class out_stream;
class item_amount_bind_t;

/**
 * @class lucky_turn_module
 * 
 * @brief
 */
class lucky_turn_module
{
public:
  static int do_turn(player_obj *);
  static int do_ten_turn(player_obj *);
  static int do_fetch_lucky_turn_info(player_obj *, out_stream &);
  static int do_fetch_big_award_log(player_obj *);

  static void on_load_lucky_turn_score(const int char_id,
                                       const int score,
                                       const int t,
                                       const bool preload);
  static void on_load_lucky_turn(const int v);
  static void on_act_open(const int);
  static void on_act_close();

  static int handle_db_get_lucky_turn_big_award_list_result(player_obj *player, in_stream &is);
private:
  static void one_turn_award(bool &, item_amount_bind_t &);
  static void do_acc_award_pool(const int cnt);
  static void do_acc_score(const int char_id,
                           const int score,
                           const int t,
                           const bool preload);
  static void on_got_big_award(player_obj *player);
  static void db_update_acc_award();
  static void on_char_lucky_turn_score_change(const int char_id,
                                              const int org_score,
                                              const int org_time,
                                              const int cur_score,
                                              const int cur_time,
                                              const bool preload);
};

#endif // LUCKY_TURN_MODULE_H_

