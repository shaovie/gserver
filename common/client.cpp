#include "client.h"
#include "mblock.h"
#include "sys_log.h"
#include "istream.h"
#include "global_macros.h"
#include "mblock_pool.h"
#include "message.h"
#include "daemon.h"
#include "def.h"

// Lib header
#include <cassert>
#include "ilog.h"
#include "reactor.h"

const char *g_sock_status[] = 
{
  "SOCKET:_SOCK_INIT - NULL",
  "SOCKET:_SOCK_CON_OK - ESTABLISHED",
  "SOCKET:_SOCK_ACTIVE - HAD TRANSFER DATA",
  "SOCKET:_SOCK_CLOSED_AS_TIMEOUT - CLOSED AS RECV DATA TIMEOUT",
  "SOCKET:_SOCK_CLOSED_BY_SVC - CLOSED BY SERVICE",
  "SOCKET:_SOCK_RESET_BY_PEER - RESET BY PEER",
  "SOCKET:_SOCK_SEND_ERROR - SEND DATA EXCEPTION",
  "SOCKET:_SOCK_CLOSED_BY_PEER - CLOSED BY PEER GRACEFULLY",
  "SOCKET:_SOCK_PACKAGE_IS_TOO_LARGE - PACKAGE IS TOO LARGE",
  "SOCKET:_SOCK_SND_BUF_IS_FULL - SEND BUF IS FULL",
  "SOCKET:_SOCK_ERROR - EPOLL CATCH ERROR|HUP"
};

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

//= shared buffer
static char *shared_send_buf = new char[MAX_ONE_MESSAGE_SIZE];
int client::send_buf_len = MAX_ONE_MESSAGE_SIZE - sizeof(proto_head);
char *client::send_buf = shared_send_buf + sizeof(proto_head);

client::client(const int recv_buf_len, const char *client_type) :
  seq_(SEQ_NO_BEGIN),
  sock_status_(_SOCK_INIT),
  client_type_(client_type),
  recv_buff_(NULL)
{
  recv_buff_ = mblock_pool::instance()->alloc(recv_buf_len);
} 
client::~client()
{
  if (this->recv_buff_)
    mblock_pool::instance()->release(this->recv_buff_);

  while (!this->buff_list_.empty())
  {
    mblock *h = this->buff_list_.pop_front();
    mblock_pool::instance()->release(h);
  }
}
const char *client::client_type()
{ return this->client_type_; }
void client::set_remote_addr(const inet_address &rm)
{ this->remote_addr_ = rm; }
void client::get_remote_addr(char *addr, const int len)
{ this->remote_addr_.get_host_addr(addr, len); }
int client::open(void *)
{
  socket::set_nodelay(this->get_handle());

  if (this->get_reactor()->register_handler(this,
                                            ev_handler::read_mask) != 0)
  {
    e_log->error("register handle %d failed! %s:%d", 
                 this->get_handle(),
                 __FILE__, 
                 __LINE__);
    return -1;
  }

  char bf[32] = {0};
  this->remote_addr_.get_host_addr(bf, sizeof(bf));

  s_log->rinfo("new %s connection h = %d addr = [%s:%d]", 
               this->client_type_,
               this->get_handle(),
               bf,
               this->remote_addr_.get_port_number());

  this->sock_status_ = _SOCK_CON_OK;

  return 0;
}
int client::handle_input(const int handle)
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
    this->sock_status_ = _SOCK_RESET_BY_PEER;
    return -1;
  }else if (ret == 0)
  {
    this->sock_status_ = _SOCK_CLOSED_BY_PEER;
    return -1;
  }
  this->recv_buff_->wr_ptr(ret);

  if (this->sock_status_ == _SOCK_SND_BUF_IS_FULL)
    return -1;
  this->sock_status_ = _SOCK_ACTIVE;

  ret = this->handle_data();
  if (ret != 0 && this->sock_status_ == _SOCK_ACTIVE)
    this->sock_status_ = _SOCK_CLOSED_BY_SVC;

  return ret;
}
int client::handle_output(const int handle)
{
  int ret = 0;
  while (!this->buff_list_.empty())
  {
    mblock *mb = this->buff_list_.pop_front();
    ret = socket::send(handle, mb->rd_ptr(), mb->length());
    //e_log->wning("handle %d handle output send %d", handle, ret);
    if (ret == -1)
    {
      if (errno != EWOULDBLOCK)
      {
        this->sock_status_ = _SOCK_SEND_ERROR;
        mblock_pool::instance()->release(mb);
      }else
      {
        this->buff_list_.push_front(mb);
        ret = 0;
      }
      break;  //
    }else
    {
      mb->rd_ptr(ret);
      if (mb->length() == 0)
      {
        mblock_pool::instance()->release(mb);
      }else
        this->buff_list_.push_front(mb);
      ret = 0;
    }
  }
  if (ret == 0)
    this->sock_status_ = _SOCK_ACTIVE;

  if (this->buff_list_.empty())
    return -1;

  return ret;
}
int client::send_data(const char *msg, const int len)
{
  if (this->sock_status_ == _SOCK_INIT)
    return -1;
  //
  if (!this->buff_list_.empty())
  {
    if (this->buff_list_.size() == 24)
    {
      this->sock_status_ = _SOCK_SND_BUF_IS_FULL;
      //e_log->wning("socket %d buff is full", this->get_handle());
      return -1;
    }
    mblock *mb = mblock_pool::instance()->alloc(len);
    mb->copy(msg, len);
    this->buff_list_.push_back(mb);
    return 0;
  }

  //
  int send_len = 0;
  for (int ret = 0; send_len < len; send_len += ret)
  {
    ret = socket::send(this->get_handle(), msg + send_len, len - send_len);
    if (ret == -1)
    {
      if (errno != EWOULDBLOCK)
      {
        this->sock_status_ = _SOCK_SEND_ERROR;
        return -1;
      }
      this->get_reactor()->register_handler(this,
                                            ev_handler::write_mask);
      break;
    }
  }
  this->sock_status_ = _SOCK_ACTIVE;
  if (send_len < len)
  {
    mblock *mb = mblock_pool::instance()->alloc(len - send_len);
    mb->copy(msg + send_len, len - send_len);
    this->buff_list_.push_back(mb);
  }
  return 0;
}
int client::handle_close(const int , reactor_mask m)
{
  if (m == ev_handler::write_mask)
  {
    if (this->sock_status_ != _SOCK_SEND_ERROR)
      return 0;
  }

  char bf[32] = {0};
  this->remote_addr_.get_host_addr(bf, sizeof(bf));
  s_log->rinfo("handle %s close - m:%d h:%d peer %s:%d [%s]", 
               this->client_type_,
               m,
               this->get_handle(),
               bf,
               this->remote_addr_.get_port_number(),
               g_sock_status[this->sock_status_]);
  delete this;
  return 0;
}
void client::shutdown() { this->handle_close(-1, 0); }
int client::handle_data()
{
  int ret = 0;
  while (this->recv_buff_->length() >= (int)sizeof(proto_head))
  {
    proto_head *ph = (proto_head *)this->recv_buff_->rd_ptr();
    if (ph->len_ < (int)sizeof(proto_head))
    {
      char bf[32] = {0};
      this->remote_addr_.get_host_addr(bf, sizeof(bf));
      e_log->wning("invalid proto ! [%d][len = %d] ip = %s", ph->seq_, ph->len_, bf);
      return -1;
    }
    if (ph->len_ > this->recv_buff_->size())
    {
      char bf[32] = {0};
      this->remote_addr_.get_host_addr(bf, sizeof(bf));
      e_log->wning("package too large! h=%d [%d][msg=%d][len = %d rlen = %d] ip = %s",
                   this->get_handle(),
                   ph->seq_,
                   ph->id_,
                   ph->len_,
                   this->recv_buff_->length(),
                   bf);
      this->sock_status_ = _SOCK_PACKAGE_IS_TOO_LARGE;
      return -1;
    }
    if (this->recv_buff_->length() < ph->len_)
      return 0; // continue recv data
    this->recv_buff_->rd_ptr(sizeof(proto_head));

    if (this->check_package_seq(ph->seq_) != 0) return -1;

    ++this->seq_;

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
int client::send_msg(const int msg_id, const int res, out_stream *os)
{
  proto_head *ph = (proto_head *)shared_send_buf;
  if (os == NULL)
    ph->set(0, msg_id, res, sizeof(proto_head));
  else
  {
    assert(os->get_ptr() == client::send_buf);
    ph->set(0, msg_id, res, os->length() + sizeof(proto_head));
    assert(ph->len_ < MAX_ONE_MESSAGE_SIZE);
  }
  return this->send_data(shared_send_buf, ph->len_);
}
