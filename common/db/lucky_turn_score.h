// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-24 11:46
 */
//========================================================================

#ifndef LUCKY_TURN_SCORE_H_
#define LUCKY_TURN_SCORE_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_lucky_turn_score[] = "lucky_turn_score";

/**
 * @class lucky_turn_score
 *
 * @brief
 */
class lucky_turn_score
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_SCORE, IDX_TURN_TIME,
    IDX_END
  };

  lucky_turn_score() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->score_                   = 0;
    this->turn_time_               = 0;
  }
  static const char *all_col()
  { return "char_id,score,turn_time"; }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s)values(%d,%d,%d)",
                      db_tb_lucky_turn_score,
                      lucky_turn_score::all_col(),
                      this->char_id_, this->score_, this->turn_time_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where turn_time < %d",
                      db_tb_lucky_turn_score,
                      this->turn_time_);
  }

  int char_id_;
  int score_;                // 排名
  int turn_time_;
};
#endif // LUCKY_TURN_SCORE_H_

