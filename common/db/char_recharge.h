// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-20 17:46
 */
//========================================================================

#ifndef CHAR_RECHARGE_H_
#define CHAR_RECHARGE_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "obj_pool.h"
#include "global_macros.h"

// Forward declarations

const char db_tb_char_recharge[] = "char_recharge";

/*
 * data_中使用json格式存储充值信息
 * key 为充值编号  value 为累计充值次数
 * {
 *  "1":2,
 *  "2":5
 * }
 * */
/**
 * @class char_recharge
 *
 * @brief
 */
class char_recharge
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_LAST_BUY_MC_TIME,
    IDX_DATA,
    IDX_END
  };
  char_recharge() { this->reset(); }
  char_recharge(const int char_id)
  {
    this->reset();
    this->char_id_ = char_id;
  }

  void reset()
  {
    this->char_id_ = 0;
    this->last_buy_mc_time_ = 0;
    ::snprintf(this->data_, sizeof(this->data_), "{}");
  }
  void operator = (const char_recharge &cr)
  {
    if (&cr == this) return ;

    this->char_id_ = cr.char_id_;
    this->last_buy_mc_time_ = cr.last_buy_mc_time_;
    ::strncpy(this->data_, cr.data_, sizeof(this->data_) - 1);
  }
  static const char *all_col()
  { return "char_id,last_buy_mc_time,data"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      char_recharge::all_col(),
                      db_tb_char_recharge,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s)values("
                      "%d,%d,'%s'"
                      ")",
                      db_tb_char_recharge,
                      char_recharge::all_col(),
                      this->char_id_, this->last_buy_mc_time_, this->data_);
  }

  int  char_id_;
  int  last_buy_mc_time_;
  char data_[MAX_CHAR_RECHARGE_LEN + 1];
};
#endif // CHAR_RECHARGE_H_

