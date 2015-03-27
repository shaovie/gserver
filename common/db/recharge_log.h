// -*- C++ -*-

//========================================================================
/**
 * Author   : alvinhu 
 * Date     : 2012-12-10 18:02
 */
//========================================================================

#ifndef RECHARGE_LOG_H_
#define RECHARGE_LOG_H_ 

// Lib header
#include <stdio.h>
#include <string.h>

#include "global_macros.h"

const char db_tb_recharge_log[] = "recharge_log";

class recharge_log_info
{
public:
  recharge_log_info(const int t, const int v) :
    rc_time_(t),
    value_(v)
  { }

  int rc_time_;
  int value_;
};
/**
 * @class recharge_log
 * 
 * @brief
 */
class recharge_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_RC_TIME, IDX_VALUE,
    IDX_END
  };
  recharge_log () { this->reset(); }

  void reset()
  {
    this->char_id_    = 0;
    this->rc_time_    = 0;
    this->value_      = 0;
  }

  static const char *all_col()
  { return "char_id,rc_time,value"; }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "%d,%d,%d"
                      ")",
                      db_tb_recharge_log,
                      recharge_log::all_col(),
                      this->char_id_, this->rc_time_, this->value_);
  }

  int char_id_;
  int rc_time_;                           // 充值时间
  int value_;                             // 单笔充值数额 
};
#endif // RECHARGE_LOG_H_ 
