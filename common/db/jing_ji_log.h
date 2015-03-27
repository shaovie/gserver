// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-25 11:21
 */
//========================================================================

#ifndef JING_JI_LOG_H_
#define JING_JI_LOG_H_

#include "global_macros.h"

// Forward declarations

const char db_tb_jing_ji_log[] = "jing_ji_log";

/**
 * @class jing_ji_log
 *
 * @brief
 */
class jing_ji_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_COMPETITOR_ID,
    IDX_RANK_CHANGE,
    IDX_TIME,
    IDX_END
  };

  jing_ji_log() { this->reset(); }
  ~jing_ji_log() { }

  static const char *all_col()
  { return "char_id,competitor_id,rank_change,time"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d or competitor_id=%d "
                      "order by time desc limit 7",
                      jing_ji_log::all_col(),
                      db_tb_jing_ji_log,
                      this->char_id_,
                      this->char_id_);
  }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) values("
                      "%d,%d,"
                      "%d,%d"
                      ")",
                      db_tb_jing_ji_log,
                      jing_ji_log::all_col(),
                      this->char_id_, this->competitor_id_,
                      this->rank_change_, this->time_);
  }

  void reset()
  {
    this->char_id_        = 0;
    this->competitor_id_  = 0;
    this->rank_change_    = 0;
    this->time_           = 0;
  }

  int   char_id_;           // 角色id
  int   competitor_id_;     // 对手id
  int   rank_change_;       // 排名变化
  int   time_;              // 记录时间
};

#endif  // JING_JI_LOG_H_
