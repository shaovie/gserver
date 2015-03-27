// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-24 11:46
 */
//========================================================================

#ifndef LTIME_RECHARGE_AWARD_H_
#define LTIME_RECHARGE_AWARD_H_

// Lib header
#include <stdio.h>
#include <string.h>

#include "global_macros.h"

// Forward declarations

const char db_tb_ltime_recharge_award[] = "ltime_recharge_award";

/**
 * @class ltime_recharge_award
 *
 * @brief
 */
class ltime_recharge_award
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_ACT_ID, IDX_SUB_ID, IDX_AWARD_TIME,
    IDX_END
  };

  ltime_recharge_award() { this->reset(); }

  void reset()
  {
    this->char_id_                  = 0;
    this->act_id_                   = 0;
    this->sub_id_                   = 0;
    this->award_time_               = 0;
  }
  static const char *all_col()
  { return "char_id,act_id,sub_id,award_time"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      ltime_recharge_award::all_col(),
                      db_tb_ltime_recharge_award,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s)values(%d,%d,%d,%d)",
                      db_tb_ltime_recharge_award,
                      ltime_recharge_award::all_col(),
                      this->char_id_, this->act_id_, this->sub_id_, this->award_time_);
  }

  int char_id_;
  int act_id_;
  int sub_id_;
  int award_time_;
};
#endif // LTIME_RECHARGE_AWARD_H_

