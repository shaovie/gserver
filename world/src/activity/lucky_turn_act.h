// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-23 17:54
 */
//========================================================================

#ifndef LUCKY_TURN_ACT_H_
#define LUCKY_TURN_ACT_H_

#include "ltime_act_obj.h"

// Forward declarations

/**
 * @class lucky_turn_act
 * 
 * @brief
 */
class lucky_turn_act : public ltime_act_obj
{
public:
  lucky_turn_act(const int act_id,
                 const int begin_time,
                 const int end_time) :
    ltime_act_obj(act_id,
                  begin_time,
                  end_time)
  { }

  virtual int open();
  virtual int close();
};

#endif // LUCKY_TURN_ACT_H_

