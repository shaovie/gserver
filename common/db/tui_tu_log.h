// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-22 21:46
 */
//========================================================================

#ifndef TUI_TU_LOG_H_
#define TUI_TU_LOG_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "obj_pool.h"

// Forward declarations

const char db_tb_tui_tu_log[] = "tui_tu_log";

/**
 * @class tui_tu_log
 *
 * @brief
 */
class tui_tu_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID,
    IDX_USED_TIME, IDX_FIRST_AWARD, IDX_STATE,
    IDX_END
  };
  enum
  {
    ST_NO_OPENED    = 0,
    ST_INPROGRESS   = 1,
    ST_FINISHED     = 2,
  };

  tui_tu_log() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->cid_                     = 0;
    this->used_time_               = 0;
    this->first_award_             = 0;
    this->state_                   = ST_NO_OPENED;
    this->free_turn_cnt_           = 0;
    this->begin_time_              = 0;
    this->is_perfect_              = false;
  }
  static const char *all_col()
  {
    return "char_id,cid,used_time,first_award,state";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      tui_tu_log::all_col(),
                      db_tb_tui_tu_log,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,"
                      "%d,%d,%d"
                      ")",
                      db_tb_tui_tu_log,
                      tui_tu_log::all_col(),
                      this->char_id_, this->cid_,
                      this->used_time_, this->first_award_, this->state_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "used_time=%d,first_award=%d,state=%d"
                      " where char_id=%d and cid=%d",
                      db_tb_tui_tu_log,
                      this->used_time_,
                      this->first_award_, this->state_,
                      this->char_id_, this->cid_);
  }

  bool is_perfect_;   // do not save db
  char state_;
  char first_award_;
  char free_turn_cnt_;     // 使用免费转盘次数
  int cid_;
  int char_id_;
  int used_time_;
  int begin_time_;    // do not save db
};
/**
 * @class tui_tu_log_pool
 * 
 * @brief
 */
class tui_tu_log_pool : public singleton<tui_tu_log_pool>
{
  friend class singleton<tui_tu_log_pool>;
public:
  tui_tu_log *alloc() { return this->pool_.alloc(); }

  void release(tui_tu_log *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  tui_tu_log_pool() { }
  obj_pool<tui_tu_log, obj_pool_std_allocator<tui_tu_log> > pool_;
};
#endif // TUI_TU_LOG_H_

