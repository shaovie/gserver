// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-30 16:20
 */
//========================================================================

#ifndef NOTICE_INFO_H_
#define NOTICE_INFO_H_

#include "global_macros.h"

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

// Forward declarations

const char db_tb_notice_info[] = "notice";

/**
 * @class notice_info
 *
 * @brief
 */
class notice_info
{
public:
  enum
  {
    IDX_ID = 0,
    IDX_BEGIN_TIME,
    IDX_END_TIME,
    IDX_INTERVAL_TIME,
    IDX_CONTENT,
    IDX_END
  };

  notice_info() { this->reset(); }
  ~notice_info() { }

  static const char *all_col()
  { return "id,begin_time,end_time,interval_time,content"; }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s) values("
                      "%d,"
                      "from_unixtime(%d,'%%Y-%%m-%%d %%H:%%i:%%S'),"
                      "from_unixtime(%d,'%%Y-%%m-%%d %%H:%%i:%%S'),"
                      "%d,'%s'"
                      ")",
                      db_tb_notice_info,
                      notice_info::all_col(),
                      this->id_,
                      this->begin_time_, this->end_time_, this->interval_time_,
                      this->content_);
  }

  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s "
                      "where id=%d",
                      db_tb_notice_info,
                      this->id_);
  }

  void reset()
  {
    this->id_             = 0;
    this->begin_time_     = 0;
    this->end_time_       = 0;
    this->interval_time_  = 0;
    this->last_time_      = 0;
    ::memset(this->content_, 0, sizeof(this->content_));
  }

  int   id_;            // 角色id
  int   begin_time_;    // 公告开始时间
  int   end_time_;      // 公告结束时间
  int   interval_time_; // 公告间隔时间
  int   last_time_;     // 上次公告时间（不保存）
  char  content_[MAX_NOTICE_CONTENT_LEN + 1];   // 公告内容
};

#endif  // NOTICE_INFO_H_
