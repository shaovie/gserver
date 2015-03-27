// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 00:10
 */
//========================================================================

#ifndef GUARD_H_
#define GUARD_H_

/**
 * @class guard
 *
 * @brief scoped auto lock
 */
template<typename lock>
class guard
{
public:
  inline guard(lock &l) : mutex_(l)
  { this->mutex_.acquire(); }

  inline guard(const lock &l) : mutex_(const_cast<lock &>(l))
  { this->mutex_.acquire(); }

  inline ~guard()
  { this->mutex_.release(); }
private:
  guard(const guard<lock> &);
  void operator = (const guard<lock> &);

  lock &mutex_;
};

#endif // GUARD_H_

