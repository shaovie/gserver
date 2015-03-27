// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-11 17:00
 */
//========================================================================

#ifndef MYSQL_DB_H_
#define MYSQL_DB_H_

#include "mysql/mysql.h"

// Forward declarations
//
class mysql_db_impl;

/**
 * @class mysql_db
 * 
 * @brief
 */
class mysql_db
{
public:
  mysql_db();

  ~mysql_db();

  //
  int open(const short port,
           const char *host,
           const char *user,
           const char *passwd,
           const char *db_name);

  void close();

  // return mysql_error description
  int mysql_err();

  // return mysql_error description
  const char *mysql_strerror();

  //
  MYSQL_RES *query(const char *sql, const int sql_len);

  int store(const char *sql, const int sql_len);

  MYSQL *mysql_ptr();

  void ping();
private:
  mysql_db_impl *impl_;
};

#endif // MYSQL_DB_H_

