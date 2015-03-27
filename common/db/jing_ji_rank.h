// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-09 21:46
 */
//========================================================================

#ifndef JING_JI_RANK_H_
#define JING_JI_RANK_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_jing_ji_rank[] = "jing_ji_rank";

/**
 * @class jing_ji_rank
 *
 * @brief
 */
class jing_ji_rank
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_RANK,
    IDX_AWARD_TIME,
    IDX_END
  };

  jing_ji_rank() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->rank_                    = 0;
    this->award_time_              = 0;
  }
  static const char *all_col()
  { return "char_id,rank,award_time"; }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s)values(%d,%d,%d)",
                      db_tb_jing_ji_rank,
                      jing_ji_rank::all_col(),
                      this->char_id_, this->rank_, this->award_time_);
  }
  int update_sql_time(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set award_time=%d "
                      "where char_id=%d",
                      db_tb_jing_ji_rank,
                      this->award_time_,
                      this->char_id_);
  }

  int rank_;                // 排名
  int char_id_;
  int award_time_;          // 每日发奖时间
};
#endif // JING_JI_RANK_H_

