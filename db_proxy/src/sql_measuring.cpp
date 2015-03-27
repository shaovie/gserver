#include "sql_measuring.h"
#include "db_async_store.h"
#include "istream.h"

// Lib header
#include "guard.h"
#include "thread_mutex.h"
#include "date_time.h"
#include "time_value.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @class sql_measuring_impl
 * 
 * @brief implement of sql_measuring
 */
class sql_measuring_impl
{
public:
  sql_measuring_impl() :
    insert_cnt_(0),
    max_insert_cnt_(0),
    update_cnt_(0),
    max_update_cnt_(0),
    select_cnt_(0),
    max_select_cnt_(0),
    delete_cnt_(0),
    max_delete_cnt_(0),
    last_measure_time_(0)
  {
    last_measure_time_ = time_value::gettimeofday().sec();
  }

  void do_sql(const int v)
  {
    guard<thread_mutex> g(this->mtx_);

    if (v == sql_opt::SQL_SELECT)
      ++this->select_cnt_;
    else if (v == sql_opt::SQL_INSERT)
      ++this->insert_cnt_;
    else if (v == sql_opt::SQL_UPDATE)
      ++this->update_cnt_;
    else if (v == sql_opt::SQL_DELETE)
      ++this->delete_cnt_;
  }
  void do_measure(out_stream &os)
  {
    guard<thread_mutex> g(this->mtx_);

    int now = time_value::gettimeofday().sec();
    int interval = now - this->last_measure_time_;
    if (interval <= 0) interval = 1;
    os << this->last_measure_time_ << now;
    this->last_measure_time_ = now;

    int insert_c = this->insert_cnt_ / interval;
    this->insert_cnt_ = 0;
    if (insert_c > this->max_insert_cnt_)
      this->max_insert_cnt_ = insert_c;
    os << insert_c << this->max_insert_cnt_;

    int update_c = this->update_cnt_ / interval;
    this->update_cnt_ = 0;
    if (update_c > this->max_update_cnt_)
      this->max_update_cnt_ = update_c;
    os << update_c << this->max_update_cnt_;

    int select_c = this->select_cnt_ / interval;
    this->select_cnt_ = 0;
    if (select_c > this->max_select_cnt_)
      this->max_select_cnt_ = select_c;
    os << select_c << this->max_select_cnt_;

    int delete_c = this->delete_cnt_ / interval;
    this->delete_cnt_ = 0;
    if (delete_c > this->max_delete_cnt_)
      this->max_delete_cnt_ = delete_c;
    os << delete_c << this->max_delete_cnt_;
  }

  int insert_cnt_;
  int max_insert_cnt_;
  int update_cnt_;
  int max_update_cnt_;
  int select_cnt_;
  int max_select_cnt_;
  int delete_cnt_;
  int max_delete_cnt_;
  int last_measure_time_;
  thread_mutex mtx_;
};
sql_measuring::sql_measuring() : impl_(new sql_measuring_impl()) { }
void sql_measuring::do_sql(const int t) { this->impl_->do_sql(t); }
void sql_measuring::do_measure(out_stream &os) { this->impl_->do_measure(os); }

