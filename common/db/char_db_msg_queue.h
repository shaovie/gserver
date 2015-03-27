// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-11-29 10:13
 */
//========================================================================

#ifndef CHAR_DB_MSG_QUEUE_H_
#define CHAR_DB_MSG_QUEUE_H_

// Lib header
#include "singleton.h"
#include "obj_pool.h"
#include <string.h>

#define MAX_PARAM_LEN         127

// Forward declarations

const char db_tb_char_db_msg_queue[] = "char_db_msg_queue";

/**
 * @class char_db_msg_queue
 * 
 * @brief
 */
class char_db_msg_queue
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_MSG_ID,
    IDX_PARAM,
    IDX_END
  };
  char_db_msg_queue() { this->reset(); }

  void reset()
  {
    this->char_id_      = 0;
    this->msg_id_       = 0;
    ::memset(this->param_, 0, sizeof(this->param_));
  }
  static const char *all_col() { return "char_id,msg_id,param"; }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values(%d,%d,'%s')",
                      db_tb_char_db_msg_queue,
                      char_db_msg_queue::all_col(),
                      this->char_id_,
                      this->msg_id_,
                      this->param_);
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      char_db_msg_queue::all_col(),
                      db_tb_char_db_msg_queue,
                      this->char_id_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where char_id=%d",
                      db_tb_char_db_msg_queue,
                      this->char_id_);
  }

  int char_id_;                   // 角色id
  int msg_id_;                    // 消息id
  char param_[MAX_PARAM_LEN + 1]; // 参数
};

/**
 * @class char_db_msg_queue_pool
 * 
 * @brief
 */
class char_db_msg_queue_pool : public singleton<char_db_msg_queue_pool>
{
  friend class singleton<char_db_msg_queue_pool>;
public:
  char_db_msg_queue *alloc() { return this->pool_.alloc(); }

  void release(char_db_msg_queue *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  char_db_msg_queue_pool() { }
  obj_pool<char_db_msg_queue, obj_pool_std_allocator<char_db_msg_queue> > pool_;
};

#endif // CHAR_DB_MSG_QUEUE_H_

