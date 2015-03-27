// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-08 12:39
 */
//========================================================================

#ifndef SQL_MEASURING_H_
#define SQL_MEASURING_H_

// Lib header
#include "singleton.h"

// Forward declarations
class out_stream;
class sql_measuring_impl;

/**
 * @class sql_measuring
 * 
 * @brief
 */
class sql_measuring : public singleton<sql_measuring>
{
  friend class singleton<sql_measuring>;
public:
  void do_sql(const int t);

  void do_measure(out_stream &os);
private:
  sql_measuring();

  sql_measuring_impl *impl_;
};

#endif // SQL_MEASURING_H_

