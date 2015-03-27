#include "reactor.h"
#include "macros.h"
#include "timer_heap.h"
#include "socket.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace help
{
  inline int reactor_mask_to_epoll_event(reactor_mask mask)
  {
    int events = ev_handler::null_mask;
    if (BIT_ENABLED(mask, ev_handler::read_mask | ev_handler::accept_mask))
      SET_BITS(events, EPOLLIN);

    if (BIT_ENABLED(mask, ev_handler::connect_mask))
      SET_BITS(events, EPOLLIN|EPOLLOUT);

    if (BIT_ENABLED(mask, ev_handler::write_mask))
      SET_BITS(events, EPOLLOUT);

    return events;
  }
}
class reactor_event_tuple
{
public:
  reactor_event_tuple() :
    eh_(NULL),
    mask_(ev_handler::null_mask)
  { }

  ev_handler *eh_;
  reactor_mask  mask_;
};
//=
reactor::reactor() :
  max_fds_(0),
  epoll_fd_(-1),
  events_(NULL),
  start_pevents_(NULL),
  end_pevents_(NULL),
  timer_queue_(NULL),
  handler_rep_(NULL)
{ }
int reactor::open(const int max_timers, const int pre_alloc_timer_nodes)
{
  struct rlimit rl;
  ::memset((void *)&rl, 0, sizeof(rl));
  int r = ::getrlimit(RLIMIT_NOFILE, &rl);
  if (r == 0 && rl.rlim_cur != RLIM_INFINITY)
    this->max_fds_ = rl.rlim_cur;

  this->epoll_fd_ = ::epoll_create(this->max_fds_);

  if (this->max_fds_ <= 0
      || this->epoll_fd_ == -1)
    return -1;

  ::fcntl(this->epoll_fd_, F_SETFD, FD_CLOEXEC);

  this->events_      = new epoll_event[this->max_fds_]();
  this->timer_queue_ = new timer_heap(max_timers, pre_alloc_timer_nodes);
  this->handler_rep_ = new reactor_event_tuple[this->max_fds_]();

  for (int i = 0; i < this->max_fds_; ++i)
  {
    this->handler_rep_[i].eh_   = NULL;
    this->handler_rep_[i].mask_ = ev_handler::null_mask;
  }

  return 0;
}
int reactor::register_handler(ev_handler *eh, reactor_mask mask)
{
  if (eh == NULL || mask == ev_handler::null_mask)
    return -1;

  int handle = eh->get_handle();
  if (handle < 0 || handle >= this->max_fds_)
    return -1;

  struct epoll_event epev;
  ::memset(&epev, 0, sizeof(epoll_event));
  epev.data.fd = handle;
  if (this->handler_rep_[handle].eh_ == NULL)
  {
    epev.events = help::reactor_mask_to_epoll_event(mask);
    if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, handle, &epev) == 0
        || (errno == EEXIST
            && ::epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, handle, &epev) == 0))
    {
      this->handler_rep_[handle].eh_   = eh;
      this->handler_rep_[handle].mask_ = mask;
      return 0;
    }
  }else
  {
    SET_BITS(this->handler_rep_[handle].mask_, mask);
    epev.events = help::reactor_mask_to_epoll_event(this->handler_rep_[handle].mask_);
    if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, handle, &epev) == -1)
    {
      if (errno == ENOENT)
      {
        // If a handle is closed, epoll removes it from the poll set
        // automatically - we may not know about it yet. If that's the
        // case, a mod operation will fail with ENOENT. Retry it as
        // an add.
        return ::epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, handle, &epev);
      }
    }else
      return 0;
  }
  return -1;
}
int reactor::remove_handler(ev_handler *eh, reactor_mask mask)
{
  if (eh == NULL) return -1;

  int handle = eh->get_handle();
  if (handle < 0 || handle >= this->max_fds_)
    return -1;

  if (BIT_DISABLED(mask, ev_handler::dont_call))
    eh->handle_close(handle, mask);

  if (this->handler_rep_[handle].eh_ == NULL)
    return 0; // already removed

  CLR_BITS(this->handler_rep_[handle].mask_, mask);

  struct epoll_event epev;
  ::memset(&epev, 0, sizeof(epoll_event));
  epev.data.fd = handle;
  if (this->handler_rep_[handle].mask_ != 0)
  {
    epev.events = help::reactor_mask_to_epoll_event(this->handler_rep_[handle].mask_);
    if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, handle, &epev) == -1)
    {
      if (errno == ENOENT)
      {
        // If a handle is closed, epoll removes it from the poll set
        // automatically - we may not know about it yet. If that's the
        // case, a mod operation will fail with ENOENT. Retry it as
        // an add.
        return ::epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, handle, &epev);
      }else
        return -1;
    }
  }else
  {
    ::epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, handle, &epev);
    this->handler_rep_[handle].eh_   = NULL;
    this->handler_rep_[handle].mask_ = ev_handler::null_mask;
  }

  return 0;
}
int reactor::run_reactor_event_loop()
{
  while (1)
  {
    if (this->handle_events() == -1)
      break;
  }
  return -1;
}
int reactor::handle_events()
{
  const time_value *timeout_val = this->timer_queue_->calculate_timeout();

  int nfds = 0;
  do 
  {
    if (this->start_pevents_ != this->end_pevents_)
    {
      nfds = 1;
      break;
    }
    int msec = -1;
    if (timeout_val != NULL)
      msec = timeout_val->sec() * 1000 + (timeout_val->usec() + 999) / 1000;

    nfds = ::epoll_wait(this->epoll_fd_, this->events_, this->max_fds_, msec);
    if (nfds > 0)
    {
      this->start_pevents_ = this->events_;
      this->end_pevents_   = this->start_pevents_ + nfds;
    }
  }while (nfds == -1 && errno == EINTR);

  if (timeout_val != NULL || nfds > 0)
  {
    this->dispatch_events();
    return 0;
  }
  return -1;
}
void reactor::dispatch_events()
{
  // handle timers early since they may have higher latency
  // constraints than I/O handlers.  Ideally, the order of
  // dispatching should be a strategy...
  this->timer_queue_->expire();

  // check to see if there are no more I/O handles left to
  // dispatch AFTER we've handled the timers.
  this->dispatch_io_events();
}
void reactor::dispatch_io_events()
{
  struct epoll_event *& ev_itor = this->start_pevents_;
  while (ev_itor < this->end_pevents_ )
  {
    // 
    ev_handler *eh = this->handler_rep_[ev_itor->data.fd].eh_;
    if (eh == NULL)
    {
      ++ev_itor;
      continue;
    }
    //
    if (BIT_ENABLED(ev_itor->events, EPOLLHUP | EPOLLERR))
    {
      // Note that if there's an error(such as the handle was closed
      // without being removed from the event set) the EPOLLHUP and/or
      // EPOLLERR bits will be set in ev_itor->events.
      this->remove_handler(eh, ev_handler::all_events_mask|ev_handler::error_mask);
      ++ev_itor;
      continue;
    }else if (BIT_ENABLED(ev_itor->events, EPOLLOUT))
    {
      CLR_BITS(ev_itor->events, EPOLLOUT);
      if (eh->handle_output(ev_itor->data.fd) < 0)
        this->remove_handler(eh, ev_handler::write_mask);
    }else if (BIT_ENABLED(ev_itor->events, EPOLLIN))
    {
      CLR_BITS(ev_itor->events, EPOLLIN);
      if (eh->handle_input(ev_itor->data.fd) < 0)
        this->remove_handler(eh, ev_handler::read_mask);
    }else
    {
      ++ev_itor;
      continue;
    } 
    // If more than one event comes in between epoll_wait(2) calls,
    // they will be combined reported.
    if (ev_itor->events == 0) ++ev_itor; 
  } // end of `while (ev_itor < this->end_pevents_ ...'
}
int reactor::schedule_timer(ev_handler *eh,
                            const time_value &delay,
                            const time_value &interval/* = time_value::zero*/)
{ return this->timer_queue_->schedule(eh, delay, interval); }
int reactor::cancel_timer(const int timer_id, const int dont_call_handle_close/* = 1*/)
{ return this->timer_queue_->cancel(timer_id, dont_call_handle_close); }
int reactor::timer_size() const
{ return this->timer_queue_->curr_size(); }
//-----------------------------------------------------------------------------------
reactor_notify::reactor_notify()
{ handles_[0] = handles_[1] = -1; }
reactor_notify::~reactor_notify()
{ this->close(); }
int reactor_notify::open(reactor *r)
{
  if (::pipe(this->handles_) != 0) return -1;
  this->set_reactor(r);

  socket::set_nonblock(this->handles_[0]);
  socket::set_nonblock(this->handles_[1]);
  if (this->get_reactor()->register_handler(this, ev_handler::read_mask) != 0)
  {
    this->close();
    return -1;
  }
  return 0;
}
int reactor_notify::handle_input(const int handle)
{
  static char r_pipe_bf[128];
  int ret = 0;
  do
  {
    ret = ::read(handle, r_pipe_bf, sizeof(r_pipe_bf));
  }while (ret > 0 || (ret == -1 && errno == EINTR));

  this->handle_notify();
  return 0;
}
void reactor_notify::notify()
{ ::write(this->handles_[1], &(this->handles_), 1); }
void reactor_notify::close()
{
  if (this->get_reactor() != NULL)
    this->get_reactor()->remove_handler(this, ev_handler::read_mask);
  this->set_reactor(NULL);
  if (this->handles_[0] != -1)
  {
    ::close(this->handles_[0]);
    ::close(this->handles_[1]);
  }
  this->handles_[0] = this->handles_[1] = -1;
}
