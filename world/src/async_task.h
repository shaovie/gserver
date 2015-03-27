// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-26 13:48
 */
//========================================================================

#ifndef ASYNC_TASK_H_
#define ASYNC_TASK_H_

// Lib header
#include "guard.h"
#include "reactor.h"
#include "singleton.h"
#include "thread_mutex.h"

#include "ilist.h"

// Forward declarations
class async_task_impl;
class async_opt_result_notify;

/**
 * @class async_opt
 * 
 * @brief
 */
class async_opt
{
public:
  async_opt() { }
  virtual ~async_opt() { }

  virtual int id() = 0;
  virtual void do_operation(async_opt_result_notify *) = 0;
};
/**
 * @class async_opt_result
 * 
 * @brief
 */
class async_opt_result
{
public:
  async_opt_result() { }
  virtual ~async_opt_result() { }

  virtual void handle_result() = 0;
};
/**
 * @class async_opt_notify
 * 
 * @brief 异步作业通知
 */
class async_opt_notify : public reactor_notify
{
public:
  async_opt_notify(async_task_impl *t) : task_(t) { }
  virtual void handle_notify();

  async_task_impl *task_;
};
/**
 * @class async_opt_result_notify
 * 
 * @brief 异步作业结果返回通知
 */
class async_opt_result_notify : public reactor_notify
{
public:
  void post_result(async_opt_result *aor)
  {
    guard<thread_mutex> g(this->async_opt_result_list_mtx_);
    this->async_opt_result_list_.push_back(aor);
    this->notify();
  }
  virtual void handle_notify()
  {
    guard<thread_mutex> g(this->async_opt_result_list_mtx_);
    if (this->async_opt_result_list_.empty()) return ;

    async_opt_result *aor = this->async_opt_result_list_.pop_front();
    aor->handle_result();
  }
  ilist<async_opt_result *> async_opt_result_list_;
  thread_mutex async_opt_result_list_mtx_;
};

/**
 * @class async_task
 * 
 * @brief
 */
class async_task : public singleton<async_task>
{
  friend class singleton<async_task>;
public:
  int open(async_opt_result_notify *, const int task_num);

  void post_opt(async_opt *ao);
private:
  async_task();

  int task_num_;
  async_task_impl *impl_;
};

#endif // ASYNC_TASK_H_

