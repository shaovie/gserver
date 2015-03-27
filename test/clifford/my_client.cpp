#include "my_client.h"
#include "mblock_pool.h"
#include "def.h"

extern int g_type;
extern int g_concurrency;
extern int g_good_cnt;
extern int g_done_cnt;
extern int g_success_cnt;
extern inet_address g_addr;
extern int g_recv_cnt;
extern void print_result();

char g_mess_buff[512 + sizeof(short) + sizeof(proto_head) + 1] = 
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
proto_head* const g_ph = (proto_head *)g_mess_buff;
short* const g_str_length = (short *)(g_mess_buff + sizeof(proto_head));

const time_value g_tv(5, 0);
svc_handler *alloc_svc_handler()
{
  if (g_type == TYPE_CONNECT)
    return new just_connect();
  else if (g_type == TYPE_ONE_REQUEST)
    return new one_request();
  return new my_client();
}
void connect_timer::init(reactor *r)
{
  this->set_reactor(r);
  this->conn_.open(r);
}
int connect_timer::handle_timeout(const time_value &)
{
  if (g_done_cnt >= g_concurrency)
  {
    print_result();
    exit(0);
  }
  if (g_good_cnt < g_concurrency)
  {
    int left = g_concurrency - g_good_cnt;
    for (int i = 0; i < 512 && i < left; ++i)
      this->conn_.connect(alloc_svc_handler(), g_addr, &g_tv);
  }
  return 0;
}

just_connect::~just_connect()
{ ++ g_done_cnt; }
int just_connect::open(void *)
{
  ++ g_good_cnt;
  return -1;
}

one_request::~one_request()
{ if (this->index_ != -1) ++ g_done_cnt; }
int one_request::open(void *)
{
  if (g_good_cnt >= g_concurrency)
    return -1;
  this->index_ = g_good_cnt ++;
  
  this->send_msg();
  return -1;
}
int one_request::send_msg()
{
  *g_str_length = rand() % 512 + 1;
  int pack_length = *g_str_length + sizeof(short) + sizeof(proto_head);
  g_ph->set(this->send_seq_++, 100001, 0, pack_length);

  // 每次一个，长度有限，不考虑缓冲区放不下和正在发送的情况
  if (socket::send(this->handle_, g_mess_buff, pack_length) == -1) return -1;
  ++ g_success_cnt;
  return 0;
}

my_client::my_client()
  : recv_seq_(0), recv_buff_(NULL)
{ recv_buff_ = mblock_pool::instance()->alloc(2 * 1024); }
my_client::~my_client()
{
  if (this->recv_buff_)
    mblock_pool::instance()->release(this->recv_buff_);
}
int my_client::open(void *)
{
  if (g_good_cnt >= g_concurrency)
    return -1;

  if (this->get_reactor()->register_handler(this,
                                            ev_handler::read_mask) == -1)
    return -1;
  this->index_ = g_good_cnt ++;
  return this->send_msg();
}
int my_client::handle_input(const int handle)
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
  if (ret == 0)
    return -1;
  else if (ret == -1)
  {
    if (errno == EWOULDBLOCK) return 0;
    return -1;
  }
  this->recv_buff_->wr_ptr(ret);
  return this->handle_data();
}
int my_client::handle_data()
{
  while (this->recv_buff_->length() >= (int)sizeof(proto_head))
  {
    proto_head *ph = (proto_head *)this->recv_buff_->rd_ptr();
    if (ph->len_ < (int)sizeof(proto_head) || ph->len_ > this->recv_buff_->size())
      return -1;

    if (this->recv_buff_->length() < ph->len_)
      return 0;
    this->recv_buff_->rd_ptr(sizeof(proto_head));

    {
      int seq = *((int*)this->recv_buff_->rd_ptr());
      if (seq != this->recv_seq_ + 1)
      { assert(0); }
      this->recv_seq_ = seq;
      ++ g_recv_cnt;
      if (this->send_msg() != 0) return -1;
    }
    this->recv_buff_->rd_ptr(ph->len_ - sizeof(proto_head));
  }
  return 0;
}
