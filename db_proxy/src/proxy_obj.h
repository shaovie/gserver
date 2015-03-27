// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-08 12:39
 */
//========================================================================

#ifndef PROXY_OBJ_H_
#define PROXY_OBJ_H_

// Lib header
#include "guard.h"
#include "thread_mutex.h"

#include "client.h"

// Forward declarations
class async_result;

/**
 * @class proxy_obj
 * 
 * @brief
 */
class proxy_obj : public client
{
private:
  class _notify : public reactor_notify
  {
  public:
    _notify(proxy_obj *p) : proxy_obj_(p) { }
    virtual void handle_notify();

    proxy_obj *proxy_obj_;
  };
  friend class _notify;
public:
  proxy_obj();
  virtual ~proxy_obj();

  //=
  virtual int open(void *);

  //=
  int proxy_id() const { return this->proxy_id_; }
  int proc_result(const int db_sid,
                  const int arg,
                  const int resp_id,
                  const int res,
                  const int rs_len,
                  const char *result);
  void post_result(mblock *);
protected:
  virtual int dispatch_msg(const int id, 
                           const int res,
                           const char *msg, 
                           const int len);

  //= client message
  int clt_login(const char *, const int );
  int clt_get_payload(const char *, const int );
protected: 
  int proc_result(async_result *);

private:
  int proxy_id_;
  mblock *async_result_head_;
  mblock *async_result_tail_;
  thread_mutex async_result_list_mtx_;
  proxy_obj::_notify notify_;
};

/**
 * @class tb_msg_handler
 * 
 * @brief
 */
class tb_msg_handler
{
public:
  virtual ~tb_msg_handler() { }
  virtual int handle_msg(proxy_obj *, const char * /*msg*/, const int /*len*/) = 0;
};

#endif // PROXY_OBJ_H_

