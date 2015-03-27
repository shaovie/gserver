// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-03-01 10:11
 */
//========================================================================

#ifndef TASK_MODULE_H_
#define TASK_MODULE_H_

#include "task_config.h"

// Forward declarations
class task_cfg_obj;
class task_info;
class player_obj;
class in_stream;
class out_stream;
class task_bit_array;

enum
{
  TASK_ADD                 = 1,
  TASK_DEL,
  TASK_UPDATE
};

//任务状态
enum
{
  TASK_ST_ACCEPTABLE       = 1, // 任务可接
  TASK_ST_INPROGRESS       = 2, // 任务正在进行
  TASK_ST_COMPLETED        = 3, // 任务已经完成,尚未提交
};

/**
 * @class task_module
 * 
 * @brief
 */
class task_module
{
public:
  static int init_module();

  static void init(player_obj *);
  static void destroy(player_obj *);

  //= db_proxy message
  static int handle_db_get_task_list_result(player_obj *player, in_stream &);
  static int handle_db_get_task_bit_array_result(player_obj *player, in_stream &);

  static int dispatch_msg(player_obj *,
                          const int msg_id,
                          const char *msg,
                          const int len);
  //= clt message
  static int clt_accept_task(player_obj *, const char *msg, const int len);
  static int clt_submit_task(player_obj *, const char *msg, const int len);
  static int clt_abandon_task(player_obj *, const char *msg, const int len);
  static int clt_do_guide(player_obj *, const char *msg, const int len);

  //= on event
  static void on_enter_game(player_obj *player);
  static void on_new_day(player_obj *, const bool);
  static void on_char_lvl_up(player_obj *player);
  static void on_char_dead(player_obj *player);
  static void on_load_task_list_ok(player_obj *player);
  static void on_kill_mst(player_obj *, const int mst_cid, const int);
  static void on_kill_char(player_obj *);
  static void on_join_guild(player_obj *);
  static void on_exit_guild(player_obj *);
  static void on_finish_tui_tu(player_obj *, const int scene_cid, const int star);
  static void on_finish_task(player_obj *player, task_info *tinfo);
  static void on_equip_strengthen(player_obj *player);
  static void on_upgrade_pskill(player_obj *player);
  static void on_guan_gai(player_obj *player);
  static void on_mo_bai(player_obj *player);
  static void on_jing_ji(player_obj *player);
  static void on_add_friends(player_obj *player);

  //= gm
  static int  gm_add_task(player_obj *, const int task_cid);
  static int  gm_submit_task(player_obj *, const int task_cid);
  static int  gm_rm_task(player_obj *, const int task_cid);
protected:
  static void do_build_all_task_todo_info(player_obj *player);
  static void do_build_acceptable_task_list(player_obj *player);
  static int  do_accept_common_task(player_obj *player, const task_cfg_obj *);
  static int  do_accept_task(player_obj *player, const task_cfg_obj *tco, task_info *tinfo);
  static int  do_submit_task(player_obj *player, task_info *, const task_cfg_obj *tco);
  static int  do_abandon_task(player_obj *player, const task_cfg_obj *tco, task_info *tinfo);
  static int  do_send_task_list_2_client(player_obj *player);
  static int  do_update_task_bit_array_2_db(player_obj *player);
  static int  do_update_task_info_2_db(player_obj *player, const int op, task_info *tinfo);
  static int  do_update_task_info_2_client(player_obj *player, const int task_op, task_info *ti);
  static void do_fetch_task_info(player_obj *, const int task_op, task_info *ti, out_stream &os);
  static int  do_update_acceptable_task_list_to_clt(player_obj *player);
  static int  do_update_acceptable_task_list(player_obj *player, out_stream &os);
  static void do_notify_to_clt_if_no_task(player_obj *player);

  static bool is_in_acceptable_list(player_obj *player, const int task_cid);
  static bool can_accept(player_obj* player, const task_cfg_obj *tco);
  static bool is_package_full(player_obj* player, task_ai_list_t *p_award_ai_list);
  static task_info *find_task_by_type(player_obj *, const int t);

  //= ai
  static bool do_task_if_ai(player_obj *, task_ai_list_t *);
  static int  do_task_do_ai(player_obj *, task_info *, task_ai_list_t *);
  static int  do_task_to_ai(player_obj *, const int task_cid, task_ai_list_t *);
  static int  do_check_task_todo_list(player_obj *, task_ai_list_t *, task_info *);
  static int  do_update_task_todo_info(player_obj *,
                                       task_ai_list_t *,
                                       task_info *tinfo,
                                       const int type,
                                       const int arg_0,
                                       const int arg_1,
                                       const int arg_2 = 0);
  static int  do_update_task_todo_list(player_obj *player,
                                       const int type,
                                       const int arg_0,
                                       const int arg_1,
                                       const int arg_2 = 0);
  static int  do_add_task_todo_info(player_obj *player,
                                    const int task_cid,
                                    const int type,
                                    const int arg_0,
                                    const int arg_1);
  static void do_del_task_todo_info(player_obj *, const int task_cid);
  static void on_update_task_todo_info_ok(player_obj *, task_info *, const task_cfg_obj *);

private:
  static int  do_give_item(player_obj *, const int , const int , const int , const char);
  static void do_random_accept_task(player_obj *, const int type);
};
#endif // TASK_MODULE_H_
