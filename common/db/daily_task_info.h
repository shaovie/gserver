// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-22 21:46
 */
//========================================================================

#ifndef DAILY_TASK_INFO_H_
#define DAILY_TASK_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_daily_task_info[] = "daily_task";

/**
 * @class daily_task_info
 *
 * @brief
 */
class daily_task_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_TASK_CID, IDX_DONE_CNT,
    IDX_END
  };

  daily_task_info() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->task_cid_                = 0;
    this->done_cnt_                = 0;
  }
  static const char *all_col()
  {
    return "char_id,task_cid,done_cnt";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      daily_task_info::all_col(),
                      db_tb_daily_task_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values(%d,%d,%d)",
                      db_tb_daily_task_info,
                      daily_task_info::all_col(),
                      this->char_id_, this->task_cid_, this->done_cnt_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set done_cnt=%d"
                      " where char_id=%d and task_cid=%d",
                      db_tb_daily_task_info,
                      this->done_cnt_,
                      this->char_id_, this->task_cid_);
  }
  int clean_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set done_cnt=%d"
                      " where char_id=%d",
                      db_tb_daily_task_info,
                      this->done_cnt_, this->char_id_);
  }

  int   char_id_;
  int   task_cid_;
  int   done_cnt_;
};
#endif // DAILY_TASK_INFO_H_

