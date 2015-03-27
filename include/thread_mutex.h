// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:24
 */
//========================================================================

#ifndef THREAD_MUTEX_H_
#define THREAD_MUTEX_H_

#include <pthread.h>

/**
 * @class thread_mutex
 *
 * @brief 
 */
class thread_mutex
{
public:
  inline thread_mutex()
  { ::pthread_mutex_init(&this->mutex_, NULL); }

  inline ~thread_mutex()
  { ::pthread_mutex_destroy(&this->mutex_); }

  inline int acquire(void)
  { return ::pthread_mutex_lock(&this->mutex_) == 0 ? 0 : -1; }

  inline int release(void) 
  { return ::pthread_mutex_unlock(&this->mutex_) == 0 ? 0 : -1; }

  inline const pthread_mutex_t &lock() const 
  { return this->mutex_; }
private:
  pthread_mutex_t mutex_;
};

#endif // THREAD_MUTEX_H_

