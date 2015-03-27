// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:50
 */
//========================================================================

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <stddef.h> 

/**
 * @class singleton
 *
 * @brief 
 */
template<typename T>
class singleton
{
public:
  static T *instance()
  {
    if (singleton<T>::instance_ == NULL)
      singleton<T>::instance_ = new T();
    return singleton<T>::instance_;
  }

  static T *instance(T *t)
  {
    T *inst = singleton<T>::instance_;
    singleton<T>::instance_ = t;
    return inst;
  }
protected:
  singleton() { }
  ~singleton() { }
private:
  static T *instance_;
};
//
template<typename T> T *singleton<T>::instance_ = NULL;

#endif // SINGLETON_H_

