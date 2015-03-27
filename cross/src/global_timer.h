// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-18 19:31
 */
//========================================================================

#ifndef GLOBAL_TIMER_H_
#define GLOBAL_TIMER_H_

//Lib Header
#include "singleton.h"
#include "ev_handler.h"

// Forward declarations

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
  global_timer() {};
  global_timer(const global_timer &);
  global_timer& operator= (const global_timer&);
};

#endif // GLOBAL_TIMER_H_

