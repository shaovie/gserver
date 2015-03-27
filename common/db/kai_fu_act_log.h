// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-10-30 11:46
 */
//========================================================================

#ifndef KAI_FU_ACT_LOG_H_
#define KAI_FU_ACT_LOG_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_kai_fu_act_log[] = "kai_fu_act_log";

/**
 * @class kai_fu_act_log
 *
 * @brief
 */
class kai_fu_act_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_ACT_TYPE, IDX_VALUE,
    IDX_END
  };

  kai_fu_act_log() { this->reset(); }
  ~kai_fu_act_log() { }

  void reset()
  {
    this->char_id_                 = 0;
    this->act_type_                = 0;
    this->value_                   = 0;
  }
  static const char *all_col()
  {
    return "char_id,act_type,value";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      kai_fu_act_log::all_col(),
                      db_tb_kai_fu_act_log,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,%d"
                      ")",
                      db_tb_kai_fu_act_log,
                      kai_fu_act_log::all_col(),
                      this->char_id_, this->act_type_, this->value_);
  }

  int char_id_;
  int act_type_;
  int value_;
};
#endif // KAI_FU_ACT_LOG_H_

