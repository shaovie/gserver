#include "async_task.h"

// Lib header
#include <stdio.h>
#include <stdlib.h>
#include "task.h"

/**
 * @class async_task
 * 
 * @brief
 */
class async_task_impl : public task
{
public:
  async_task_impl() :
    r_(NULL),
    opt_notify_(NULL),
    opt_result_notify_(NULL)
  { }
  int open(async_opt_result_notify *opt_result_notify)
  {
    if (opt_result_notify == NULL)
      return -1;

    this->r_ = new reactor();
    this->opt_notify_ = new async_opt_notify(this);
    this->opt_result_notify_ = opt_result_notify;

    if (this->r_->open(16, 16) != 0)
    {
      fprintf(stderr, "Error: async reactor - open failed!\n");
      return -1;
    }

    if (this->opt_notify_->open(this->r_) != 0)
      return -1;
    return this->activate(1, 0);
  }
  void handle_opt_list()
  {
    guard<thread_mutex> g(this->async_opt_list_mtx_);
    while (!this->async_opt_list_.empty())
    {
      async_opt *ao = this->async_opt_list_.pop_front();
      ao->do_operation(this->opt_result_notify_);
    }
  }
  void post_opt(async_opt *ao)
  {
    guard<thread_mutex> g(this->async_opt_list_mtx_);
    this->async_opt_list_.push_back(ao);
    this->opt_notify_->notify();
  }
protected:
  virtual int svc()
  {
    this->r_->run_reactor_event_loop();
    return 0;
  }
private:
  reactor *r_;
  async_opt_notify *opt_notify_;
  async_opt_result_notify *opt_result_notify_;
  ilist<async_opt *> async_opt_list_;
  thread_mutex async_opt_list_mtx_;
};
async_task::async_task() : task_num_(1), impl_(NULL)
{ }
int async_task::open(async_opt_result_notify *opt_result_notify, const int task_num)
{
  if (task_num <= 0) return -1;
  this->task_num_ = task_num;

  this->impl_ = new async_task_impl[this->task_num_];
  for (int i = 0; i < this->task_num_; ++i)
  {
    if (this->impl_[i].open(opt_result_notify) != 0)
      return -1;
  }
  return 0;
}
void async_task::post_opt(async_opt *ao)
{ this->impl_[::abs(ao->id() % this->task_num_)].post_opt(ao); }

//
void async_opt_notify::handle_notify()
{
  this->task_->handle_opt_list();
}

