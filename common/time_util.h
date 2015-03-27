// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-19 14:24
 */
//========================================================================

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_

//Lib Header
#include "date_time.h"

// Forward declarations

/**
 * @class time_util
 *
 * @brief
 */
class time_util
{
public:
  static int init();

  static int diff_days(const int time_1, const int time_2)
  {
    int t1 = time_1 - time_util::greenwich_diff_sec;
    int t2 = time_2 - time_util::greenwich_diff_sec;
    return ((t1 - t1 % 86400) - (t2 - t2 % 86400))/86400;
  }
  static int diff_days(const int t)
  { return time_util::diff_days(time_util::now, t); }

  static bool diff_month(const int t)
  {
    date_time dt1(t);
    date_time dt2(time_util::now);
    if (dt1.year() != dt2.year()) return true;
    if (dt1.month() != dt2.month()) return true;
    return false;
  }
  static bool diff_week(const int t)
  {
    date_time dt1(t);
    date_time dt2(time_util::now);
    if (time_util::diff_days(t) >= 7) return true;
    if (dt1.wday() < dt2.wday()) return true;
    return false;
  }

  static int time()
  {
    date_time dt(time_util::now);
    return time_util::now - (int)dt.date();
  }

  static int date()
  {
    date_time dt(time_util::now);
    return (int)dt.date();
  }

  static int wday()
  {
    date_time dt(time_util::now);
    return time_util::wday(dt);
  }
  static int wday(date_time &dt)
  { return dt.wday() == 0 ? 7 : dt.wday(); }

  static int mday()
  {
    date_time dt(time_util::now);
    return dt.mday();
  }
public:
  static int now;
  static int greenwich_diff_sec;
};

#endif // TIME_UTIL_H_

