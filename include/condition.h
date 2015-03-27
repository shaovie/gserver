// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 02:24
 */
//========================================================================

#ifndef CONDITION_H_
#define CONDITION_H_

#include <errno.h>
#include <pthread.h>

/**
 * @class condition
 *
 * @brief 
 */
template<typename mtx_t>
class condition
{
public:
  condition(mtx_t &m) : mutex_(m)
  { ::pthread_cond_init(&this->cond_, NULL); }

  ~condition()
  {
    int retry = 0;
    while ((::pthread_cond_destroy(&this->cond_) == EBUSY)
           && retry++ < 3)
      this->broadcast();
  }

  inline int wait(void)
  {
    int result = ::pthread_cond_wait(&this->cond_,
                                     const_cast<pthread_mutex_t*>(&this->mutex_.lock()));
    if (result != 0)	{ errno = result; result = -1;}
    return result;
  }

  inline int signal(void)
  { return ::pthread_cond_signal(&this->cond_) == 0 ? 0 : -1; }

  inline int broadcast(void)
  { return ::pthread_cond_broadcast(&this->cond_) == 0 ? 0 : -1; }

  inline mtx_t &mutex(void)
  { return this->mutex_; }
private:
  mtx_t &mutex_;
  pthread_cond_t cond_;
};

#endif // CONDITION_H_

