#ifndef CLIENT_1_MSG_H_
#define CLIENT_1_MSG_H_

#include "svc_handler.h"
#include "head.h"
#include "socket.h"
#include "def.h"

extern int g_success_connect_num;
extern int g_failed_connect_num;

class client_1_msg : public svc_handler
{
public:
  virtual int open(void * = NULL)
  {
    char mb[1024] = {0};
    int msg_len = ::rand()%(512 + 1);
    proto_head *head = (proto_head *)mb;
    head->set(SEQ_BEGIN, MSG_ID_BEGIN, 0, msg_len + sizeof(proto_head));
    *(short *)(mb + sizeof(proto_head)) = msg_len;
    this->send_msg(mb, msg_len + sizeof(proto_head) + sizeof(short));
    return -1;
  }
  virtual int handle_timeout(const time_value &)
  {
    ++g_failed_connect_num;
    return -1;
  }
  int send_msg(const char *msg, const int len)
  {
    int ret = socket::send(this->get_handle(), msg, len);
    if (ret <= 0)
      ++g_failed_connect_num;
    else
      ++g_success_connect_num;
    return 0;
  }
};
#endif
