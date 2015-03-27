// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-22 21:46
 */
//========================================================================

#ifndef SCP_LOG_H_
#define SCP_LOG_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_scp_log[] = "scp_log";

/**
 * @class scp_log
 *
 * @brief
 */
class scp_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_ENTER_CNT, IDX_ENTER_TIME,
    IDX_END
  };

  scp_log() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->cid_                     = 0;
    this->enter_cnt_               = 0;
    this->enter_time_              = 0;
  }
  static const char *all_col()
  {
    return "char_id,cid,enter_cnt,enter_time";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      scp_log::all_col(),
                      db_tb_scp_log,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,%d,%d"
                      ")",
                      db_tb_scp_log,
                      scp_log::all_col(),
                      this->char_id_, this->cid_, this->enter_cnt_, this->enter_time_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "enter_cnt=%d,enter_time=%d"
                      " where char_id=%d and cid=%d",
                      db_tb_scp_log,
                      this->enter_cnt_, this->enter_time_,
                      this->char_id_, this->cid_);
  }
  int clear_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set enter_cnt=0,enter_time=0 where char_id=%d",
                      db_tb_scp_log,
                      this->char_id_);
  }

  char enter_cnt_;    // 进入次数
  int cid_;
  int char_id_;
  int enter_time_;
};
#endif // SCP_LOG_H_

