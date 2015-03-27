// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-01 14:38
 */
//========================================================================

#ifndef DB_PROXY_H_
#define DB_PROXY_H_

// Lib header
#include "singleton.h"

// Forward declarations
class reactor;
class out_stream;
class db_proxy_impl;

class db_handler
{
public:
  virtual ~db_handler() { }
  virtual int dispatch_msg(const int , const int , const char *, const int ) = 0;
};
/**
 * @class db_proxy
 *
 * @brief
 */
class db_proxy : public singleton<db_proxy>
{
  friend class singleton<db_proxy>;
public:
  int open(reactor *r,
           db_handler *dh,
           const int num,
           const int port,
           const char *host);

  const char *version();

  int send_request(const int char_id, const int msg_id, out_stream *os);
private:
  db_proxy();
  db_proxy(const db_proxy &);
  db_proxy& operator= (const db_proxy&);

  int proxy_num_;
  db_proxy_impl *impl_;
};

#endif // DB_PROXY_H_

