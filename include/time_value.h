// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-26 19:59
 */
//========================================================================

#ifndef TIME_VALUE_H_
#define TIME_VALUE_H_

#include <cstddef>
#include <sys/time.h>

/**
 * @class time_value
 *
 * @brief 
 */
class time_value
{
public:
  // constant "0".
  static const time_value zero;

  static const time_value start_time;

  time_value() { tv_.tv_sec = 0; tv_.tv_usec = 0; }

  time_value(const time_value& tv)
  {
    this->tv_.tv_sec  = tv.tv_.tv_sec;
    this->tv_.tv_usec = tv.tv_.tv_usec;
  }

  explicit time_value(const time_t sec, const long int usec = 0)
  { set(sec, usec); }

  inline time_value &operator = (const time_value &tv)
  {
    this->tv_.tv_sec  = tv.tv_.tv_sec;
    this->tv_.tv_usec = tv.tv_.tv_usec;
    return *this;
  }

  // initializes the time_value from seconds and useconds.
  inline void set(const time_t sec, const long int usec)
  {
    this->tv_.tv_sec  = sec;
    this->tv_.tv_usec = usec;
    this->normalize();
  }

  inline long msec(void) const
  { return this->tv_.tv_sec * 1000 + this->tv_.tv_usec / 1000; }

  inline time_t sec(void) const { return this->tv_.tv_sec; }
  inline void sec(const time_t sec) { this->tv_.tv_sec = sec; }

  inline long int usec(void) const { return this->tv_.tv_usec; }
  inline void usec(const long int usec) { this->tv_.tv_usec = usec; }

  inline void update() { ::gettimeofday(&this->tv_, NULL); }

  inline time_value &operator += (const time_value &tv)
  {
    this->tv_.tv_sec  += tv.tv_.tv_sec;
    this->tv_.tv_usec += tv.tv_.tv_usec;
    this->normalize();
    return *this;
  }

  inline time_value &operator += (const time_t tv)
  {
    this->tv_.tv_sec += tv;
    return *this;
  }
  inline time_value &operator -= (const time_value &tv)
  {
    this->tv_.tv_sec  -= tv.tv_.tv_sec;
    this->tv_.tv_usec -= tv.tv_.tv_usec;
    this->normalize();
    return *this;
  }
  inline time_value &operator -= (const time_t tv)
  {
    this->tv_.tv_sec -= tv;
    return *this;
  }
  inline friend time_value operator + (const time_value &tv1, 
                                       const time_value &tv2)
  {
    time_value sum(tv1);
    sum += tv2;
    return sum;
  }
  inline friend time_value operator - (const time_value &tv1, 
                                       const time_value &tv2)
  {
    time_value data(tv1);
    data -= tv2;
    return data;
  }
  inline friend bool operator < (const time_value &tv1,
                                 const time_value &tv2)
  {
    if (tv1.tv_.tv_sec < tv2.tv_.tv_sec)
      return true;
    else if (tv1.tv_.tv_sec == tv2.tv_.tv_sec
             && tv1.tv_.tv_usec < tv2.tv_.tv_usec)
      return true;
    return false;
  }
  inline friend bool operator > (const time_value &tv1,
                                 const time_value &tv2)
  { return tv2 < tv1; }
  inline friend bool operator <= (const time_value &tv1,
                                  const time_value &tv2)
  {
    if (tv1.tv_.tv_sec < tv2.tv_.tv_sec)
      return true;
    else if (tv1.tv_.tv_sec == tv2.tv_.tv_sec
             && tv1.tv_.tv_usec <= tv2.tv_.tv_usec)
      return true;
    return false;
  }
  inline friend bool operator >= (const time_value &tv1,
                                  const time_value &tv2)
  { return tv2 <= tv1; }
  inline friend bool operator == (const time_value &tv1,
                                  const time_value &tv2)
  {
    return tv1.tv_.tv_sec == tv2.tv_.tv_sec
      && tv1.tv_.tv_usec == tv2.tv_.tv_usec;
  }
  inline friend bool operator != (const time_value &tv1,
                                  const time_value &tv2)
  { return !(tv1 == tv2); }
  static inline time_value gettimeofday()
  {
    time_value tv;
    ::gettimeofday(&(tv.tv_), NULL);
    return tv;
  }
protected:
  void normalize(void);

private:
  struct timeval tv_;
};

#endif // TIME_VALUE_H_

