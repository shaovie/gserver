// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:24
 */
//========================================================================

#ifndef EV_HANDLER_H_
#define EV_HANDLER_H_

#include <cstddef>

// Forward declarations
class reactor; 
class time_value;

typedef int reactor_mask;

/**
 * @class ev_handler
 *
 * @brief 
 */
class ev_handler
{
public:
  enum 
  {
    null_mask       = 0,
    read_mask       = 1L << 1,
    write_mask      = 1L << 2,
    except_mask     = 1L << 3,
    accept_mask     = 1L << 4,
    connect_mask    = 1L << 5,
    timer_mask      = 1L << 6,
    error_mask      = 1L << 7,
    dont_call       = 1L << 31,

    all_events_mask = read_mask  | \
                      write_mask   |
                      except_mask  |
                      accept_mask  |
                      connect_mask |
                      timer_mask   ,
  };
public:
  virtual ~ev_handler() {}

  // the handle_close(int, reactor_mask) will be called if below handle_* return -1. 
  virtual int handle_input(const int /*handle*/)        { return -1; }
  virtual int handle_output(const int /*handle*/)       { return -1; }
  virtual int handle_exception(const int /*handle*/)    { return -1; }
  virtual int handle_timeout(const time_value &/*now*/) { return -1; }

  virtual int handle_close(const int /*handle*/, reactor_mask /*mask*/)
  { return -1; }

  virtual int get_handle() const { return -1; }
  virtual void set_handle(const int /*handle*/) { }

  int timer_id() const { return this->timer_id_; }
  void timer_id(const int id) { this->timer_id_ = id; }

  void set_reactor(reactor *r) { this->reactor_ = r; }
  reactor *get_reactor(void) const { return this->reactor_; }
protected:
  ev_handler()
    : timer_id_(-1),
    reactor_(NULL)
  { }

  int timer_id_;
  reactor *reactor_;
};

#endif // EV_HANDLER_H_

