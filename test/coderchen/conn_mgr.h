#ifndef CONN_MGR_H_
#define CONN_MGR_H_

#include "inet_address.h"
#include "ev_handler.h"
#include "connector.h"
#include "client_mgr.h"

template <typename CONN>
class conn_mgr : public ev_handler, public client_mgr
{
  public:
  conn_mgr(const char *ip, short port, int concurrency)
    : concurrency_(concurrency), addr_(port, ip) 
  { }

  int open(reactor *r)
  {
    this->set_reactor(r);
    if (r->schedule_timer(this, time_value(0, 0), time_value(60, 0)) == -1)
      return -1;
    this->conn_.open(this->reactor_);
    return 0;
  }
  virtual int handle_timeout(const time_value&)
  {
    time_value tv(5, 0);
    for (int i = 0; i < this->concurrency_ - this->cur_conn_cnt_; ++i)
      this->conn_.connect(new CONN(this), this->addr_, &tv);

    return 0;
  }

private:
  int concurrency_;
  inet_address addr_;
  connector<CONN> conn_;
};

#endif
