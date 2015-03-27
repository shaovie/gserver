#include "reactor.h"
#include "svc_handler.h"
#include <acceptor.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <cassert>
#include <signal.h>

int g_current_client = 0;
int g_pack_count = 0;

class dog : public svc_handler
{
public:
  virtual int handle_timeout(const time_value &now)
  {
    fprintf(stderr, "[%d:%d] current client: %d pack count: %d\n", 
            (int)now.sec(),
            (int)now.usec(),
            g_current_client,
            g_pack_count);
    return 0;
  }
};
class client : public svc_handler
{
public:
  virtual int open(void *)
  {
    //socket::set_nodelay(this->get_handle());
    int ret = this->get_reactor()->register_handler(this,
                                                    ev_handler::read_mask);
    assert(ret != -1);
    return 0;
  }
  virtual int handle_input(const int handle)
  {
    static char bf[512] = {0};
    int ret = socket::recv(handle, bf, sizeof(bf));
    if (ret <= 0)
      return -1;

    static char http_resp[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: 5\r\n\r\nhello";
    //static char http_resp[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 5\r\n\r\nhello";
    ret = socket::send(handle, http_resp, sizeof(http_resp)-1);
    if (ret <= 0)
    {
      fprintf(stderr, "send data failed!\n");
      return -1;
    }
    return 0;
  }
  virtual int handle_close(const int , reactor_mask m)
  {
    delete this;
    return 0;
  }
};
int main()
{
  int port = 7777;

  signal(SIGPIPE ,SIG_IGN);

  reactor r;
  int ret = r.open(1024, 128);
  if (ret != 0)
  {
    fprintf(stderr, "open failed![%s]\n", strerror(errno));
    return -1;
  }

  inet_address local_addr(port, "0.0.0.0");
  acceptor<client> i_acceptor;
  ret = i_acceptor.open(local_addr, &r);
  if (ret == -1)
  {
    fprintf(stderr, "acceptor open failed![%s]\n", strerror(errno));
    return -1;
  }
#if 0
  ret = r.schedule_timer(new dog,
                         time_value(0, 200*1000),
                         time_value(0, 200*1000));
#endif
  r.run_reactor_event_loop();
  return 0;
}
