#ifndef CONNECT_MGR_H_
#define CONNECT_MGR_H_

#include <stdio.h>
#include "ev_handler.h"
#include "connector.h"

extern int g_all_connect_num;
extern int g_once_connect_num;

template<typename connector_type>
class connect_mgr : public ev_handler
{
public:
  int init(reactor *rt, unsigned short port, const char *host) 
  {
    this->reactor_ = rt;
    this->remote_address_  = new inet_address(port, host);

    this->had_try_connect_time_ = 0;
    if (this->conn_.open(rt) != 0)
      return -1;
    return this->reactor_->schedule_timer(this,
                                          time_value(0, 50000),
                                          time_value(0, 50000));
  }
  int handle_timeout(const time_value &)
  {
    time_value tm(0, 20000);
    for (int i = 0;
         i < g_once_connect_num && this->had_try_connect_time_ < g_all_connect_num;
         ++i)
    {
      this->conn_.connect(new connector_type, *(this->remote_address_), &tm, 0);
      ++this->had_try_connect_time_;
    }
    return 0;
  }
  static connect_mgr* instance()
  {
    static connect_mgr instance_;
    return &instance_;
  }
private:
  int had_try_connect_time_;
  inet_address *remote_address_;
  reactor *reactor_;
  connector<svc_handler> conn_;
};
#endif
