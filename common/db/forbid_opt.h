// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-30 20:26
 */
//========================================================================

#ifndef FORBID_OPT_H_
#define FORBID_OPT_H_

#include "global_macros.h"

//Lib header
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Forward declarations
//

const char db_tb_forbid_opt[] = "forbid_opt";

/**
 * @class forbid_opt
 * 
 * @brief
 */
class forbid_opt
{
public:
  enum
  {
    FORBID_AC_LOGIN         = 1,
    FORBID_IP_LOGIN         = 2,
    FORBID_CHAR_LOGIN       = 3,
    FORBID_CHAR_TALK        = 4,
    FORBID_END
  };
  enum
  {
    IDX_ID = 0,
    IDX_OPT,
    IDX_BEGIN_TIME, IDX_END_TIME,
    IDX_TARGET,
    IDX_END
  };
  forbid_opt() { this->reset(); }

  void reset()
  {
    this->id_               = 0;
    this->opt_              = 0;
    this->begin_time_       = 0;
    this->end_time_         = 0;
    ::memset(this->target_, '\0', sizeof(this->target_));
  }
  void operator = (const forbid_opt &v)
  {
    if (&v == this) return ;

    this->id_               = v.id_;
    this->opt_              = v.opt_;
    this->begin_time_       = v.begin_time_;
    this->end_time_         = v.end_time_;
    ::strncpy(this->target_, v.target_, sizeof(v.target_) - 1);
  }
  static const char *all_col()
  { return "id,opt,begin_time,end_time,target"; }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values("
                      "%d,%d,%d,%d,'%s'"
                      ")",
                      db_tb_forbid_opt,
                      forbid_opt::all_col(),
                      this->id_,
                      this->opt_,
                      this->begin_time_, this->end_time_,
                      this->target_);
  }

  char    id_;                             // 
  char    opt_;                            // 
  int     end_time_;                       // 持续时间
  int     begin_time_;                     // 开始时间
  char    target_[MAX_ACCOUNT_LEN + 1];    // 值
};

#endif // FORBID_OPT_H_

