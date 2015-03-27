#include <stdio.h>
#include <reactor.h>
#include <signal.h>
#include <unistd.h>

#include "svc_handler.h"
#include "connector.h"
#include "mblock_pool.h"
#include "istream.h"
#include "def.h"
#include "time_value.h"

#define TIME_INTERVAL           100
#define MAX_ONE_MESSAGE_SIZE    1024  // 1K
#define TYPE_SHORT_CONNECT      1
#define TYPE_SHORT_CONNEC_MSG   2
#define TYPE_LONG_CONNECT       3

class client;
class timer;

char *host = NULL;
int port   = 0;

int g_type   = 0;
int g_count  = 1;
connector<client> c_con;

uint64_t g_suc_connect = 0;
int g_every_connect = 0;
time_value last_send_con_time;
time_value last_suc_con_time;

uint64_t g_recv_pkg_count = 0;
double g_qps = 0;

time_value g_start_time;

static char *shared_send_buf = new char[MAX_ONE_MESSAGE_SIZE];
int send_buf_len = MAX_ONE_MESSAGE_SIZE - sizeof(proto_head);
char *send_buf = shared_send_buf + sizeof(proto_head);

const char test_bf[512 + 1] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

class client : public svc_handler
{
public:
  client()
    : seq_(SEQ_NO_BEGIN),
    sever_seq_(0)
  {
    recv_buff_ = mblock_pool::instance()->alloc(1024);
  }
  ~client()
  {
    if (this->recv_buff_)
      mblock_pool::instance()->release(this->recv_buff_);
  }

  virtual int open(void *) { return -1; }
  virtual int handle_data() { return -1; }

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
    {
      return -1;
    }
    this->recv_buff_->wr_ptr(ret);

    ret = this->handle_data();

    return ret;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    delete this;
    return 0;
  }

  int send_msg(const int msg_id, const int res, out_stream *os)
  {
    proto_head *ph = (proto_head *)shared_send_buf;
    if (os == NULL)
      ph->set(this->seq_, msg_id, res, sizeof(proto_head));
    else
    {
      ph->set(this->seq_, msg_id, res, os->length() + sizeof(proto_head));
      assert(ph->len_ < MAX_ONE_MESSAGE_SIZE);
    }
    this->seq_++;

    int send_len = 0;
    for (int ret = 0; send_len < ph->len_; send_len += ret)
    {
      ret = socket::send(this->get_handle(), shared_send_buf + send_len, ph->len_ - send_len);
      if (ret == -1) return -1;
    }
    return 0;
  }

public:
  int seq_;
  int sever_seq_;
  mblock *recv_buff_;
};

class short_client : public client
{
public:
  virtual int open(void *)
  {
    g_suc_connect++;
    last_suc_con_time = time_value::gettimeofday();

    return -1;
  }
};

class short_client_msg : public short_client
{
  typedef short_client supper;
public:
  virtual int open(void *)
  {
    supper::open(NULL);

    this->send_msg(100001, 0, NULL);
    return -1;
  }
};

class long_client : public client
{
public:
  virtual int open(void *)
  {
    g_suc_connect++;
    socket::set_nodelay(this->get_handle());

    int ret = this->get_reactor()->register_handler(this, ev_handler::read_mask);
    if (ret == -1) return -1;

    return this->send_test_msg();
  }
  int handle_data()
  {
    int ret = 0;
    while (this->recv_buff_->length() >= (int)sizeof(proto_head))
    {
      proto_head *ph = (proto_head *)this->recv_buff_->rd_ptr();
      if (ph->len_ < (int)sizeof(proto_head))
        return -1;
      if (ph->len_ > this->recv_buff_->size())
        return -1;
      if (this->recv_buff_->length() < ph->len_)
        return 0;
      this->recv_buff_->rd_ptr(sizeof(proto_head));

      // handle msg
      int seq = *((int*)this->recv_buff_->rd_ptr());
      this->sever_seq_++;
      assert(this->sever_seq_ == seq);
      g_recv_pkg_count++;

      this->recv_buff_->rd_ptr(ph->len_ - sizeof(proto_head));

      ret = this->send_test_msg();
      if (ret != 0)
        break;
    }
    return ret;
  }
  int send_test_msg()
  {
    out_stream os(send_buf, send_buf_len);
    stream_ostr so(test_bf, rand() % 512 + 1);
    os << so;
    return this->send_msg(100001, 0, &os);
  }
};

class timer : public ev_handler
{
public:
  timer()
  { }

  virtual int handle_timeout(const time_value &)
  {
    if (g_type == TYPE_SHORT_CONNECT || g_type == TYPE_SHORT_CONNEC_MSG)
    {
      if (last_suc_con_time > last_send_con_time)
      {
        time_value con_time = last_suc_con_time - last_send_con_time;
        int usec = con_time.sec() * 1000000 + con_time.usec();
        if (g_qps == 0)
          g_qps = (int)(g_every_connect * 1.0 / usec * 1000000);
        else
          g_qps = (int)(g_qps + g_every_connect * 1.0 / usec * 1000000) / 2;
        g_every_connect = 0;
      }
    }

    for (int i = 0; i < g_count; ++i)
    {
      inet_address remote_addr(port, host);
      client *c = NULL; 
      if (g_type == TYPE_SHORT_CONNECT)
        c = new short_client();
      else if (g_type == TYPE_SHORT_CONNEC_MSG)
        c = new short_client_msg();
      else
        c = new long_client();

      time_value tv(2, 0);
      c_con.connect(c, remote_addr, &tv);
    }
    if (g_type == TYPE_SHORT_CONNECT || g_type == TYPE_SHORT_CONNEC_MSG)
      last_send_con_time = time_value::gettimeofday();

    return 0;
  }
};
void init()
{
  g_suc_connect = 0;

  g_qps = 0;
  g_recv_pkg_count = 0;

  g_start_time = time_value::gettimeofday();
}
void print_test_info(int)
{
  time_value run_time = time_value::gettimeofday() - g_start_time;
  fprintf(stdout, "\n");
  fprintf(stdout, "success connect count: %ld\n", g_suc_connect);
  fprintf(stdout, "success recv package count: %ld\n", g_recv_pkg_count);
  fprintf(stdout, "TIME: %d\n", (int)run_time.sec());

  if (g_type == TYPE_LONG_CONNECT)
    g_qps = g_recv_pkg_count / (run_time.sec() + run_time.usec() / 1000000.0); 

  fprintf(stdout, "QPS: %.2f/sec\n", g_qps);

  init();
}
int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    fprintf(stdout, "Usage: %s `host` `port` `type` `count`\n", argv[0]);
    return 0;
  }

  host    = argv[1];
  port    = ::atoi(argv[2]);
  g_type  = ::atoi(argv[3]);
  g_count = ::atoi(argv[4]);

  if (g_count > 1000)
  {
    fprintf(stdout, "count can not > 1000\n");
    return 0;
  }

  ::signal(SIGHUP, print_test_info);
  ::signal(SIGPIPE, SIG_IGN);

  reactor *r = new reactor();
  if (r->open(1024, 32))
    return -1;

  c_con.open(r);

  r->schedule_timer(new timer(),
                    time_value(3, 0),
                    time_value(3, 0)); 

  init();
  r->run_reactor_event_loop();

  return 0;
}
