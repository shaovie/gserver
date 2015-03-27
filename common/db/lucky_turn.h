// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-24 11:46
 */
//========================================================================

#ifndef LUCKY_TURN_H_
#define LUCKY_TURN_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_lucky_turn[] = "lucky_turn";

/**
 * @class lucky_turn
 *
 * @brief
 */
class lucky_turn
{
public:
  enum
  {
    IDX_ACC_AWARD = 0,
    IDX_END
  };

  lucky_turn() { this->reset(); }

  void reset()
  {
    this->acc_award_                   = 0;
  }
  static const char *all_col()
  { return "acc_award"; }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set acc_award=%d",
                      db_tb_lucky_turn,
                      this->acc_award_);
  }
  int acc_award_;
};
#endif // LUCKY_TURN_H_

