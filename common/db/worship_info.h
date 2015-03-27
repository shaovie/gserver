// -*- C++ -*-

//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-08-04 10:20
 */
//========================================================================

#ifndef WORSHIP_INFO_H_
#define WORSHIP_INFO_H_

// Lib header

// Forward declarations

const char db_tb_worship_info[] = "worship_info";

/**
 * @class worship_info
 *
 * @brief
 */
class worship_info
{
public:
  worship_info() { this->reset(); }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "left_coin=%d,acc_coin=%d,acc_worship_cnt=%d",
                      db_tb_worship_info,
                      this->left_coin_,
                      this->acc_coin_, this->acc_worship_cnt_);
  }

  void reset()
  {
    this->left_coin_         = 0;
    this->acc_coin_          = 0;
    this->acc_worship_cnt_   = 0;
  }
public:
  int  acc_worship_cnt_;   // 王族被崇拜次数
  int  left_coin_;         // 被崇拜王族可领金币
  int  acc_coin_;          // 被崇拜王族累积金币
};

#endif  // WORSHIP_INFO_H_
