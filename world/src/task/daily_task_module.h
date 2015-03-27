// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-19 16:36
 */
//========================================================================

#ifndef DAILY_TASK_MODULE_H_
#define DAILY_TASK_MODULE_H_

// Forward declarations
class in_stream;
class player_obj;
class daily_task_info;

/**
 * @class daily_task_module
 * 
 * @brief
 */
class daily_task_module
{
public:
  static void handle_db_get_daily_task_list_result(player_obj *, in_stream &);

  static void on_new_day(player_obj *);
  static void on_finish_task(player_obj *, const int task_cid);

  static int task_done_cnt(player_obj *, const int task_cid);
private:
  static daily_task_info *get_daily_task_info(player_obj *, const int task_cid);
};

#endif // DAILY_TASK_MODULE_H_

