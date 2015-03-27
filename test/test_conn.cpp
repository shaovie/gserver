#include "reactor.h"
#include "ev_handler.h"
#include "svc_handler.h"
#include "connector.h"
#include "mblock.h"
#include "ilist.h"
#include "singleton.h"

#include "def.h"
#include "istream.h"

#include <signal.h>
#include <stdlib.h>

char *g_host = NULL;
int g_port = 4010;
int g_total_payload = 0;

class conn : public svc_handler
{
public:
  conn()
    : recv_buff_(NULL),
    conn_ok_(false)
  { }

  ~conn()
  {
    if (this->recv_buff_ != NULL)
      this->recv_buff_->release();
  }

  virtual int open(void *)
  {
    this->recv_buff_ = new mblock(1024);
    return 0;
  }

  virtual int handle_input(const int handle)
  {
    if (this->recv_buff_->space() == 0)
    {
      int len = this->recv_buff_->length();
      if (len != 0)
        ::memmove(this->recv_buff_->data(), this->recv_buff_->rd_ptr(), len);
      this->recv_buff_->reset();
      this->recv_buff_->wr_ptr(len);
    }
    int ret = socket::recv(handle,
                           this->recv_buff_->wr_ptr(), 
                           this->recv_buff_->space());
    if (ret == -1)
    {
      if (errno == EWOULDBLOCK) return 0;  // Maybe
      return -1;
    }else if (ret == 0)
      return -1;

    this->recv_buff_->wr_ptr(ret);

    return this->handle_data();
  }

  virtual int handle_close(const int , reactor_mask )
  {
    delete this;
    return 0;
  }

  virtual int handle_data()
  { return -1; }
protected:
  bool conn_ok_;
  mblock *recv_buff_;
};

class conn_mgr
{
public:
  void insert(conn *p)
  { this->conn_list_.push_back(p); }
  void remove(conn *p)
  { this->conn_list_.remove(p); }
protected:
  ilist<conn *> conn_list_;
};
//=
class short_conn : public conn
{
  typedef conn super;
public:
  short_conn();
  ~short_conn();

  virtual int open(void *)
  {
    this->conn_ok_ = true;
    this->get_reactor()->register_handler(this, ev_handler::read_mask);
    return -1;
  }
  virtual int handle_timeout(const time_value &)
  {
    fprintf(stderr, "short connect timeout!\n");
    return -1;
  }
};
class short_conn_mgr : public conn_mgr
                       , public ev_handler
                       , public singleton<short_conn_mgr>
{
  friend class singleton<short_conn_mgr>;
public:
  void init(reactor *r)
  {
    this->connector_.open(r);
    int ret = r->schedule_timer(this,
                                time_value(0, 600*1000),
                                time_value(0, 600*1000));
    assert(ret != -1);
  }
  virtual int handle_timeout(const time_value &)
  {
    for (int i = 0; i < 32; ++i)
    {
      if (this->conn_list_.size() > 256)
        break;

      inet_address remote_addr(g_port, g_host);
      time_value tv(2, 0);
      this->connector_.connect(new short_conn(), remote_addr, &tv);
    }
    return 0;
  }
private:
  short_conn_mgr() { }
  short_conn_mgr(const short_conn_mgr &);
  short_conn_mgr& operator= (const short_conn_mgr&);

  connector<short_conn> connector_;
};
short_conn::short_conn()
: conn()
{ short_conn_mgr::instance()->insert(this); }
short_conn::~short_conn()
{ short_conn_mgr::instance()->remove(this); }
class long_conn : public conn
{
  typedef conn super;
public:
  long_conn();
  ~long_conn();

  virtual int open(void *p)
  {
    if (super::open(p) != 0)
      return -1;

    this->get_reactor()->register_handler(this, ev_handler::read_mask);
    this->conn_ok_ = true;

    int r = rand() % 100;
    int ret = this->get_reactor()->schedule_timer(this,
                                                  time_value(0, (300 + r)*1000),
                                                  time_value(0, (300 + r)*1000));
    assert(ret != -1);
    return this->send_req();
  }

  int send_req()
  {
    static char str[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    short r = rand() % (sizeof(str) - 1) + 1;
    proto_head ph(this->seq_++, 100001, 0, sizeof(proto_head) + r + 2);
    socket::send(this->get_handle(), (char *)&ph, sizeof(ph));
    socket::send(this->get_handle(), (char *)&r, sizeof(short));
    int ret = socket::send(this->get_handle(), str, r);
    if (ret <= 0)
      fprintf(stderr, "long conn send req failed![%s]\n", strerror(errno));
    return ret > 0 ? 0 : -1;
  }

  virtual int handle_timeout(const time_value &)
  {
    if (!this->conn_ok_)
    {
      fprintf(stderr, "long connect timeout!\n");
      return -1;
    }
    return this->send_req();
  }
  virtual int handle_data()
  {
    int ret = 0;
    while (this->recv_buff_->length() >= (int)sizeof(proto_head))
    {
      proto_head *ph = (proto_head *)this->recv_buff_->rd_ptr();
      if (ph->len_ < (int)sizeof(proto_head))
      {
        fprintf(stderr, "invalid proto ! [%d][len = %d]", ph->seq_, ph->len_);
        return -1;
      }
      if (ph->len_ > this->recv_buff_->size())
      {
        fprintf(stderr, "package too large! h=%d [%d][msg=%d][len = %d rlen = %d]",
                this->get_handle(),
                ph->seq_,
                ph->id_,
                ph->len_,
                this->recv_buff_->length());
        return -1;
      }
      if (this->recv_buff_->length() < ph->len_)
        return 0; // continue recv data
      this->recv_buff_->rd_ptr(sizeof(proto_head));

      ++this->recv_seq_;

      // to move forward
      char *msg = this->recv_buff_->rd_ptr();
      this->recv_buff_->rd_ptr(ph->len_ - sizeof(proto_head));
      // handle msg
      ret = this->dispatch_msg(ph->id_,
                               ph->result_,
                               msg,
                               ph->len_ - sizeof(proto_head));
      if (ret != 0)
        break;
    }
    return ret;
  }
  virtual int dispatch_msg(const int id, 
                           const int result,
                           const char *msg,
                           const int len)
  {
    in_stream is(msg, len);
    int sq = 0;
    is >> sq;
    assert(sq == this->recv_seq_);
    return 0;
  }
private:
  int seq_;
  int recv_seq_;
};
class long_conn_mgr : public conn_mgr
                      , public ev_handler
                      , public singleton<long_conn_mgr>
{
  friend class singleton<long_conn_mgr>;
public:
  void init(reactor *r)
  {
    this->connector_.open(r);
    int ret = r->schedule_timer(this,
                                time_value(0, 600*1000),
                                time_value(0, 600*1000));
    assert(ret != -1);
  }
  virtual int handle_timeout(const time_value &)
  {
    for (int i = 0; i < 32; ++i)
    {
      if (this->conn_list_.size() >= g_total_payload)
        break;

      inet_address remote_addr(g_port, g_host);
      time_value tv(2, 0);
      this->connector_.connect(new long_conn(), remote_addr, &tv);
    }
    return 0;
  }
private:
  long_conn_mgr() { }
  long_conn_mgr(const long_conn_mgr &);
  long_conn_mgr& operator= (const long_conn_mgr&);

  connector<long_conn> connector_;
};
long_conn::long_conn()
: conn(),
  seq_(20121221),
  recv_seq_(0)
{ long_conn_mgr::instance()->insert(this); }
long_conn::~long_conn()
{ long_conn_mgr::instance()->remove(this); }

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    fprintf(stderr, "./prog 192.168.1.222 4010 900\n");
    return -1;
  }
  signal(SIGPIPE ,SIG_IGN);
  signal(SIGHUP ,SIG_IGN);
  reactor *r = new reactor();
  int ret = r->open(81920, 512);
  if (ret != 0)
  {
    fprintf(stderr, "open failed![%s]\n", strerror(errno));
    return -1;
  }

  g_host = argv[1];
  g_port = ::atoi(argv[2]);
  g_total_payload = ::atoi(argv[3]);

  short_conn_mgr::instance()->init(r);
  long_conn_mgr::instance()->init(r);
  r->run_reactor_event_loop();
  return 0;
}
