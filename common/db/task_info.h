// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-10-31 13:58
 */
//========================================================================

#ifndef TASK_INFO_H_
#define TASK_INFO_H_

#include "def.h"
#include "obj_pool.h"

// Lib header
#include <cstring>
#include <cstdio>
#include "singleton.h"

// Forward declarations

#define MAX_TASK_VALUE  3

const char db_tb_task_info[] = "task";

/**
 * @class task_info
 * 
 * @brief
 */
class task_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_TASK_CID, IDX_STATE,
    IDX_VALUE_0, IDX_VALUE_1,
    IDX_VALUE_2,
    IDX_EXTRA_VALUE,
    IDX_ACCEPT_TIME,
    IDX_END
  };
  task_info() { this->reset(); }
  task_info(const int char_id, const int task_cid, const char st)
  {
    this->reset();
    this->char_id_           = char_id;
    this->task_cid_          = task_cid;
    this->state_             = st;
  }
  
  void reset()
  {
    this->dirty_             = false;
    this->char_id_           = 0;
    this->task_cid_          = 0;
    this->state_             = 0;
    this->extra_value_       = 0;
    this->accept_time_       = 0;
    memset(this->value_, 0, sizeof(this->value_));
  }
  static const char *all_col()
  { return "char_id,task_cid,state,value_0,value_1,value_2,extra_value,accept_time"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where char_id=%d",
                      task_info::all_col(),
                      db_tb_task_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "%d,%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d"
                      ")",
                      db_tb_task_info,
                      task_info::all_col(),
                      this->char_id_, this->task_cid_, this->state_,
                      this->value_[0], this->value_[1], this->value_[2],
                      this->extra_value_, this->accept_time_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "update %s set "
                      "state=%d,"
                      "value_0=%d,value_1=%d,value_2=%d,extra_value=%d,"
                      "accept_time=%d"
                      " where char_id=%d and task_cid=%d",
                      db_tb_task_info,
                      this->state_,
                      this->value_[0], this->value_[1], this->value_[2], this->extra_value_,
                      this->accept_time_,
                      this->char_id_, this->task_cid_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "delete from %s"
                      " where char_id=%d and task_cid=%d",
                      db_tb_task_info,
                      this->char_id_, this->task_cid_);
  }
  //
  bool dirty() const { return this->dirty_; }
  void dirty(const bool v) { this->dirty_ = v; }

  char state() const { return this->state_; }
  void state(const char v) { this->state_ = v; this->dirty_ = true; }

  int value(const int idx) const { return this->value_[idx]; }
  void value(const int idx, const int v)
  { this->value_[idx] = v; this->dirty_ = true; }

public:
  bool    dirty_;                     // 内存变量 do not store to db
  char    state_;                     // 任务状态
  int     char_id_;                   // 所属角色ID
  int     task_cid_;                  // 任务id
  int     value_[MAX_TASK_VALUE];     // 计数器
  int     extra_value_;               // 计数器
  int     accept_time_;               // 接受任务时间
};

/**
 * @class task_info_pool
 * 
 * @brief
 */
class task_info_pool : public singleton<task_info_pool>
{
  friend class singleton<task_info_pool>;
public:
  task_info *alloc() { return this->pool_.alloc(); }

  void release(task_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  task_info_pool() { }

  obj_pool<task_info, obj_pool_std_allocator<task_info> > pool_;
};

#endif // TASK_INFO_H_

