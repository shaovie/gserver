#ifndef  CLIENT_NO_MSG_H_
#define  CLIENT_NO_MSG_H_

#include "svc_handler.h"

extern int g_success_connect_num;
extern int g_failed_connect_num;

class client_no_msg : public svc_handler
{
public:
  virtual int open(void * = NULL)
  {
    ++g_success_connect_num;
    return -1;
  }
  virtual int handle_timeout(const time_value &)
  {
    ++g_failed_connect_num;
    return -1;
  }
};
#endif

