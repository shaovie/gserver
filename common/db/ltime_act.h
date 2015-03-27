// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-6-13 12:19
 */
//========================================================================

#ifndef LTIME_ACT_H_
#define LTIME_ACT_H_ 

//Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_ltime_act[] = "ltime_act";

/**
 * @class ltime_act
 * 
 * @brief
 */
class ltime_act
{
public:
  enum
  {
    IDX_ACT_ID = 0,
    IDX_BEGIN_TIME,
    IDX_END_TIME,
    IDX_END
  };
  ltime_act() { this->reset(); }

  void reset()
  {
    this->act_id_       = 0;
    this->begin_time_   = 0;
    this->end_time_     = 0;
  }
  static const char *all_col() { return "act_id,begin_time,end_time"; }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values(%d,"
                      "from_unixtime(%d,'%%Y-%%m-%%d %%H:%%i:%%S'),"
                      "from_unixtime(%d,'%%Y-%%m-%%d %%H:%%i:%%S'))",
                      db_tb_ltime_act,
                      ltime_act::all_col(),
                      this->act_id_,
                      this->begin_time_,
                      this->end_time_);
  }

  int act_id_;
  int begin_time_;
  int end_time_; 
};

#endif // LTIME_ACT_H_

