#include "time_value.h"

const time_value time_value::zero;

const time_value time_value::start_time = time_value::gettimeofday();

void time_value::normalize()
{
  if (this->tv_.tv_usec >= 1000000)
  {
    do{
      ++this->tv_.tv_sec;
      this->tv_.tv_usec -= 1000000;
    }while (this->tv_.tv_usec >= 1000000);
  }else if (this->tv_.tv_usec <= -1000000)
  {
    do{
      --this->tv_.tv_sec;
      this->tv_.tv_usec += 1000000;
    }while (this->tv_.tv_usec <= -1000000);
  }

  if (this->tv_.tv_sec >= 1 && this->tv_.tv_usec < 0)
  {
    --this->tv_.tv_sec;
    this->tv_.tv_usec += 1000000;
  }
}
