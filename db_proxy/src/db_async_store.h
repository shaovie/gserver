// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-14 19:59
 */
//========================================================================

#ifndef DB_ASYNC_STORE_H_
#define DB_ASYNC_STORE_H_

// Lib header
#include "mysql/mysql.h"
#include "singleton.h"
#include "ev_handler.h"

#include "ilist.h"
#include "mblock_pool.h"

// Forward declarations
class reactor;
class proxy_obj;
class async_result;
class db_async_store_impl;

/**
 * @class sql_opt
 * 
 * @brief
 */
class sql_opt
{
public:
  enum
  {
    SQL_SELECT = 1,
    SQL_INSERT,
    SQL_UPDATE,
    SQL_DELETE,
    DB_PING
  };
  virtual ~sql_opt() { }

  virtual int opt_type() = 0;
  virtual int id() = 0;
  virtual int build_sql(char * /*bf*/, const int /*bf_len*/) = 0;
  virtual void read_sql_result(MYSQL_RES *, async_result *) { return ; }
};
/**
 * @class async_result
 * 
 * @brief
 */
class async_result
{
public:
  async_result();
  ~async_result();

  void reset()
  {
    this->arg_          = 0;
    this->result_       = -1;
    this->db_sid_       = 0;
    this->resp_id_      = 0;
    this->result_cnt_   = 0;
    this->result_head_  = NULL;
    this->result_tail_  = NULL;
  }
  void push_back(mblock *mb)
  {
    if (this->result_tail_ == NULL)
    {
      this->result_head_ = mb;
      this->result_tail_ = mb;
    }else
    {
      this->result_tail_->next(mb);
      this->result_tail_ = mb;
    }
    this->result_cnt_++;
  }
  void destroy()
  {
    if (this->result_head_ == NULL) return ;
    for (mblock *itor = this->result_head_; itor != NULL; )
    {
      mblock *mb = itor;
      itor = itor->next();
      mblock_pool::instance()->release(mb);
    }
    this->result_cnt_   = 0;
    this->result_head_  = NULL;
    this->result_tail_  = NULL;
  }

  int arg_;               //
  int result_;            // result
  int db_sid_;     // uuid in proxy obj
  int resp_id_;           // for respond client 
  int result_cnt_;
  mblock *result_head_;
  mblock *result_tail_;
};

/**
 * @class db_async_store
 * 
 * @brief
 */
class db_async_store : public ev_handler
                       , public singleton<db_async_store>
{
  friend class singleton<db_async_store>;
public:
  int open(reactor *r,
           const int conn_num, 
           const short port, 
           const char *host, 
           const char *user, 
           const char *passwd, 
           const char *db_name);

  // on failed return -1 (db is busy), on succ return 0
  int do_sql(const int proxy_id, 
             const int db_session_id,
             const int arg,
             const int resp_id, 
             sql_opt *so);

  virtual int handle_timeout(const time_value &);
private:
  db_async_store();

  int task_num_;
  db_async_store_impl *impl_;
};

#endif // DB_ASYNC_STORE_H_

