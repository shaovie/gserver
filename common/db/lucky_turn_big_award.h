// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-24 11:46
 */
//========================================================================

#ifndef LUCKY_TURN_BIG_AWARD_H_
#define LUCKY_TURN_BIG_AWARD_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_lucky_turn_big_award[] = "lucky_turn_big_award";

/**
 * @class lucky_turn_big_award
 *
 * @brief
 */
class lucky_turn_big_award
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_AWARD_TIME, IDX_AWARD_V,
    IDX_END
  };

  lucky_turn_big_award() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->award_time_              = 0;
    this->award_v_                 = 0;
  }
  static const char *all_col()
  { return "char_id,award_time,award_v"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s limit 36",
                      lucky_turn_big_award::all_col(),
                      db_tb_lucky_turn_big_award);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,%d"
                      ")",
                      db_tb_lucky_turn_big_award,
                      lucky_turn_big_award::all_col(),
                      this->char_id_, this->award_time_, this->award_v_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where award_time < %d",
                      db_tb_lucky_turn_big_award,
                      this->award_time_);
  }

  int char_id_;
  int award_time_;
  int award_v_;
};
#endif // LUCKY_TURN_BIG_AWARD_H_

