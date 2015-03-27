#include "time_util.h"

// Lib header
#include <sys/time.h>

int time_util::now = 0;
int time_util::greenwich_diff_sec = 0;

int time_util::init()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  time_util::greenwich_diff_sec = tz.tz_minuteswest * 60;
  time_util::now = (int)tv.tv_sec;
  return 0;
}
