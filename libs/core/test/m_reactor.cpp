#include "reactor.h"
#include "svc_handler.h"
#include "acceptor.h"
#include "task.h"
#include "thread_mutex.h"
#include "guard.h"
#include "ilist.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <cassert>
#include <signal.h>

const int r_cnt = 4;
reactor m_reactor[r_cnt];

class worker : public task
{
public:
  worker(reactor *r)
  : reactor_(r)
  { }
protected:
  virtual int svc()
  {
    this->reactor_->run_reactor_event_loop();
    return 0;
  }
private:
  reactor *reactor_;
};
class client;
class client_dispatch : public reactor_notify
{
public:
  client_dispatch()
  { }
  void init(reactor *r)
  {
    this->set_reactor(r);
    this->open(r);
  }
  void notify_new_client(client *c)
  {
    {
      guard<thread_mutex> g(this->mtx_);
      this->new_client_list_.push_back(c);
    }
    this->notify();
  }
  virtual void handle_notify();
private:
  thread_mutex mtx_;
  ilist<client *> new_client_list_;
};
//
client_dispatch m_client_dispatch[r_cnt];
class client : public svc_handler
{
public:
  virtual int open(void *)
  {
    const int ar[] = {0, 1, 2, 3};
    m_client_dispatch[ar[this->get_handle() % 4]].notify_new_client(this);
    return 0;
  }
  int connect_ok(reactor *r)
  {
    this->set_reactor(r);
    this->get_reactor()->register_handler(this,
                                          ev_handler::read_mask);
    return 0;
  }
  virtual int handle_input(const int handle)
  {
    static char bf[512] = {0};
    int ret = socket::recv(handle, bf, sizeof(bf));
    if (ret <= 0)
      return -1;

    static char http_resp[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: 5\r\n\r\nhello";
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
void client_dispatch::handle_notify()
{
  while (true)
  {
    client *c = NULL;
    {
      guard<thread_mutex> g(this->mtx_);
      if (this->new_client_list_.empty())
        return ;
      c = this->new_client_list_.pop_front();
    }
    if (c->connect_ok(this->get_reactor()) != 0)
      c->close();
  }
}
int main()
{
  int port = 7777;

  signal(SIGPIPE ,SIG_IGN);

  for (size_t i = 0; i < sizeof(m_reactor) / sizeof(m_reactor[0]); ++i)
  {
    if (m_reactor[i].open(100000, 8192) != 0)
    {
      fprintf(stderr, "open failed![%s]\n", strerror(errno));
      return -1;
    }
    m_client_dispatch[i].init(&(m_reactor[i]));
  }

  inet_address local_addr(port, "0.0.0.0");
  acceptor<client> i_acceptor;
  int ret = i_acceptor.open(local_addr, &(m_reactor[0]));
  if (ret == -1)
  {
    fprintf(stderr, "acceptor open failed![%s]\n", strerror(errno));
    return -1;
  }

  for (size_t i = 1; i < sizeof(m_reactor) / sizeof(m_reactor[0]); ++i)
  {
    worker *w = new worker(&(m_reactor[i]));
    if (w->activate(1, 1024) != 0)
      return -1;
  }
  m_reactor[0].run_reactor_event_loop();
  return 0;
}
