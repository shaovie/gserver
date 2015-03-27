// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-11-19 15:18
 */
//========================================================================

#ifndef CONSUME_LOG_H_
#define CONSUME_LOG_H_

// Lib header
#include <string.h>

#include "global_macros.h"

const char db_tb_consume_log[] = "consume_log";

class consume_log_info
{
public:
  consume_log_info(const int cs_type,
                   const int cs_time,
                   const int value) :
    cs_type_(cs_type),
    cs_time_(cs_time),
    value_(value)
  { }

  int cs_type_;                           // 消费来源
  int cs_time_;                           // 消费时间
  int value_;                             // 
};
/**
 * @class consume_log
 * 
 * @brief
 */
class consume_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CS_TYPE, IDX_CS_TIME, IDX_VALUE,
    IDX_END
  };
  consume_log () { reset(); }

  void reset()
  {
    this->char_id_    = 0;
    this->cs_type_    = 0;
    this->cs_time_    = 0;
    this->value_      = 0;
  }

  static const char *all_col()
  { return "char_id,cs_type,cs_time,value"; }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "%d,%d,%d,%d"
                      ")",
                      db_tb_consume_log,
                      consume_log::all_col(),
                      this->char_id_, this->cs_type_, this->cs_time_, this->value_);
  }

  int char_id_;
  int cs_type_;                           // 消费来源
  int cs_time_;                           // 消费时间
  int value_;                             //
};
#endif // CONSUME_LOG_H_

