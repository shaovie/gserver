#ifndef MY_CLIENT_H_
#define MY_CLIENT_H_

#include "svc_handler.h"
#include "singleton.h"
#include "connector.h"

#define SEQ_BEGIN 20121221
class mblock;

enum
{
  TYPE_CONNECT      = 1,
  TYPE_ONE_REQUEST  = 2,
  TYPE_ALIVE_CONN   = 3
};

class connect_timer : public ev_handler
                      , public singleton<connect_timer>
{
  friend class singleton<connect_timer>;
public:
  void init(reactor *);
  virtual int handle_timeout(const time_value &);
private:
  connector<svc_handler> conn_;
};

class just_connect : public svc_handler
{
public:
  virtual ~just_connect();

  virtual int open(void *);
};

class one_request : public svc_handler
{
public:
  one_request()
    : index_(-1), send_seq_(SEQ_BEGIN)
  { }
  virtual ~one_request();

  virtual int open(void *);
  int send_msg();
protected:
  int index_;
  int send_seq_;
};

class my_client : public one_request
{
public:
  my_client();
  virtual ~my_client();

  virtual int open(void *);
  virtual int handle_input(const int);
  virtual int handle_data();
protected:
  int recv_seq_;
  mblock *recv_buff_;
};
#endif // MY_CLIENT_H_
