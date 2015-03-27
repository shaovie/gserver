// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-26 16:56
 */
//========================================================================

#ifndef REACTOR_H_
#define REACTOR_H_

#include "time_value.h"
#include "ev_handler.h"

// Forward declarations
struct epoll_event;
class timer_heap;
class reactor_event_tuple;

/**
 * @class reactor
 *
 * @brief 
 */
class reactor
{
public:
  reactor();

  int open(const int max_timers, // limit
           const int pre_alloc_timer_nodes);

  int run_reactor_event_loop();

  int register_handler(ev_handler *eh, reactor_mask mask);

  int remove_handler(ev_handler *eh, reactor_mask mask);

  //= timer 
  int schedule_timer(ev_handler *eh,
                     const time_value &delay,
                     const time_value &interval = time_value::zero);

  int cancel_timer(const int timer_id, const int dont_call_handle_close = 1);

  int timer_size() const;

private:
  int handle_events();

  void dispatch_events();

  void dispatch_io_events();
private:
  reactor(const reactor &);
  reactor &operator = (const reactor &);
private:
  int max_fds_;
  int epoll_fd_;

  struct epoll_event *events_;
  struct epoll_event *start_pevents_;
  struct epoll_event *end_pevents_;

  timer_heap *timer_queue_;

  reactor_event_tuple *handler_rep_;
};

/**
 * @class reactor_notify
 *
 * @brief 
 */
class reactor_notify : public ev_handler
{
public:
  reactor_notify();
  ~reactor_notify();

  int open(reactor *r);
  void close();
  void notify();

  virtual int get_handle() const { return this->handles_[0]; }

  virtual int handle_input(const int );
  virtual void handle_notify() = 0;
private:
  int handles_[2];
};
#endif // REACTOR_H_

