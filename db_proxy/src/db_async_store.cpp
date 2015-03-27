#include "db_async_store.h"
#include "sql_measuring.h"
#include "global_macros.h"
#include "mblock_pool.h"
#include "proxy_mgr.h"
#include "mysql_db.h"
#include "sys_log.h"
#include "def.h"

// Lib header
#include <errno.h>
#include "task.h"
#include "time_value.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("base");

class async_arg
{
public:
  async_arg();
  ~async_arg();
  void reset()
  {
    this->proxy_id_       = -1;
    this->db_sid_         = -1;
    this->arg_            = 0;
    this->resp_id_        = 0;
    this->sql_opt_        = NULL;
  }

  void destroy()
  {
    delete this->sql_opt_;
    this->sql_opt_ = NULL;
  }

  int proxy_id_;          // for look up proxy obj
  int db_sid_;            // uuid in proxy obj
  int arg_;               // for proxy obj
  int resp_id_;           // for response client
  sql_opt *sql_opt_;      // for execute sql
};

class db_async_store_impl : public task
{
public:
  db_async_store_impl() : db_(NULL) { }

  virtual int open(const short port, 
                   const char *host, 
                   const char *user, 
                   const char *passwd, 
                   const char *db_name)
  { 
    this->db_ = new mysql_db();
    if (this->db_->open(port, host, user, passwd, db_name) != 0)
    {
      delete this->db_;
      this->db_ = NULL;
      return -1;
    }

    this->shared_buffer_ = new char[MAX_SQL_LEN + 1];
    this->shared_buffer_[MAX_SQL_LEN] = '\0';

    return this->activate(1, 8192);
  }
  int do_sql(const int proxy_id, 
             const int db_sid,
             const int arg,
             const int resp_id, 
             sql_opt *so)
  {
    mblock *mb = mblock_pool::instance()->alloc(sizeof(async_arg));
    async_arg *aa = (async_arg *)mb->data();
    aa->reset();
    aa->proxy_id_ = proxy_id;
    aa->db_sid_   = db_sid;
    aa->arg_      = arg;
    aa->resp_id_  = resp_id;
    aa->sql_opt_  = so;

    if (this->putq(mb) == -1)
    {
      char bf[MAX_SQL_LEN + 1] = {0};
      aa->sql_opt_->build_sql(bf, MAX_SQL_LEN);
      e_log->error("do sql failed! mq is busy [%s]", bf);
      aa->destroy();
      mblock_pool::instance()->release(mb);
      return -1;
    }
    return 0;
  }
protected:
  virtual int svc()
  {
    while (true)
    {
      mblock *mb = NULL;
      if (this->getq_n(mb) <= 0)
        continue;

      for (mblock *itor = mb; itor != NULL; )
      {
        mblock *t = itor;
        itor = itor->next();
        this->process_sql(t);
        mblock_pool::instance()->release(t);
      }
    }
    return 0;
  }
  void process_sql(mblock *mb)
  {
    async_arg *aa = (async_arg *)mb->data();
    sql_measuring::instance()->do_sql(aa->sql_opt_->opt_type());
    int len = aa->sql_opt_->build_sql(this->shared_buffer_, MAX_SQL_LEN);
    if (aa->sql_opt_->opt_type() == sql_opt::SQL_SELECT)
    {
      MYSQL_RES *res = this->db_->query(this->shared_buffer_, len);
      this->post_result_i(res, aa, this->shared_buffer_, len);
    }else if (aa->sql_opt_->opt_type() == sql_opt::DB_PING)
    {
      this->db_->ping();
    }else // update delete insert ...
    {
      if (this->db_->store(this->shared_buffer_, len) != 0)
        this->post_error_i(aa, this->shared_buffer_, len);
    }
    aa->destroy();
  }
  void post_result_i(MYSQL_RES *res,
                     async_arg *aa,
                     const char *sql,
                     const int sql_len)
  {
    if (res == NULL)
    {
      this->post_error_i(aa, sql, sql_len);
      return ;
    }

    mblock *mb = mblock_pool::instance()->alloc(sizeof(async_result));
    async_result *ar = (async_result *)mb->data();
    ar->reset();
    ar->arg_ = aa->arg_;
    ar->result_ = DB_ASYNC_OK;
    ar->db_sid_ = aa->db_sid_;
    ar->resp_id_ = aa->resp_id_;

    aa->sql_opt_->read_sql_result(res, ar);

    mysql_free_result(res); // !!!

    if (proxy_mgr::instance()->post_result(aa->proxy_id_, mb) != 0)
    {
      ar->destroy();
      mblock_pool::instance()->release(mb);
    }
  }
  void post_error_i(async_arg *aa, const char *sql, const int sql_len)
  {
    mblock *mb = mblock_pool::instance()->alloc(sizeof(async_result));
    async_result *ar = (async_result *)mb->data();
    ar->reset();
    ar->result_ = this->db_->mysql_err();
    ar->db_sid_ = aa->db_sid_;
    ar->resp_id_ = aa->resp_id_;

    mblock *sql_mb = mblock_pool::instance()->alloc(sql_len + 1/*\0*/);
    sql_mb->copy(sql, sql_len);
    sql_mb->set_eof();
    ar->push_back(sql_mb);
    if (proxy_mgr::instance()->post_result(aa->proxy_id_, mb) != 0)
    {
      ar->destroy();
      mblock_pool::instance()->release(mb);
    }
  }
private:
  mysql_db *db_;
  char *shared_buffer_;
};

db_async_store::db_async_store() :
  task_num_(3),
  impl_(NULL)
{ }
int db_async_store::open(reactor *r,
                         const int task_num,
                         const short port, 
                         const char *host, 
                         const char *user, 
                         const char *passwd, 
                         const char *db_name)
{
  this->task_num_ = task_num;
  this->impl_ = new db_async_store_impl[this->task_num_]();
  for (int i = 0; i < this->task_num_; ++i)
  {
    if (this->impl_[i].open(port,
                            host,
                            user,
                            passwd,
                            db_name) != 0)
      return -1;
  }
  if (r->schedule_timer(this, time_value(3, 0), time_value(30, 0)) == -1)
    return -1;
  return 0;
}
int db_async_store::do_sql(const int proxy_id, 
                           const int db_sid,
                           const int arg,
                           const int resp_id, 
                           sql_opt *so)
{
  return this->impl_[::abs(so->id() % this->task_num_)].do_sql(proxy_id,
                                                               db_sid,
                                                               arg,
                                                               resp_id,
                                                               so);
}
class ping_sql_opt : public sql_opt
{
public:
  virtual int id() { return 0; }
  virtual int opt_type() { return sql_opt::DB_PING; }
  virtual int build_sql(char *, const int ) { return 0; }
};
int db_async_store::handle_timeout(const time_value &)
{
  for (int i = 0; i < this->task_num_; ++i)
    this->impl_[i].do_sql(0, 0, 0, 0, new ping_sql_opt());
  return 0;
}
