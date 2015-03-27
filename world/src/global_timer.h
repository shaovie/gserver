// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-10-25 11:31
 */
//========================================================================

#ifndef GLOBAL_TIMER_H_
#define GLOBAL_TIMER_H_

//Lib Header
#include "singleton.h"
#include "ev_handler.h"

// Forward declarations

class reactor;
class time_value;

/**
 * @class global_timer
 * 
 * @brief
 */
class global_timer : public ev_handler
                     , public singleton<global_timer>
{
  friend class singleton<global_timer>;
public:
  virtual int handle_timeout(const time_value &);

private:
  void check_svc_open(const int );

  void do_svc_open();

  bool is_new_day(const int now);
  void do_something_on_new_day();
  void do_something_on_new_week(const int now);
private:
  global_timer();
  global_timer(const global_timer &);
  global_timer& operator= (const global_timer&);
};

#endif // GLOBAL_TIMER_H_

