// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-30 16:20
 */
//========================================================================

#ifndef SERVICE_INFO_H_
#define SERVICE_INFO_H_

#include "global_macros.h"

// Lib header
#include "singleton.h"

// Forward declarations

const char db_tb_service_info[] = "service_info";

/**
 * @class service_info
 *
 * @brief
 */
class service_info
{
public:
  service_info() { this->reset(); }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set open_time=from_unixtime(%d,'%%Y-%%m-%%d %%H:%%i:%%S'),"
                      "opened=%d,ghz_closed_time=%d",
                      db_tb_service_info,
                      this->open_time, this->opened, this->ghz_closed_time);
  }

  void reset()
  {
    this->opened            = 0;
    this->open_time         = 0;
    this->ghz_closed_time   = 0;
  }
public:
  char opened;            //
  int  open_time;         // 开服时间
  int  ghz_closed_time;   // 公会战结束时间
};

#endif  // SERVICE_INFO_H_
