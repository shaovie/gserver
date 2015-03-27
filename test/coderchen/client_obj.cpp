#include <stdlib.h>
#include <assert.h>

#include "client_obj.h"
#include "mblock.h"
#include "mblock_pool.h"
#include "def.h"
#include "client_mgr.h"

int client_only_connect::open(void *)
{
  this->conn_ok_ = true;
  this->cli_mgr_->on_conn_success();
  return -1;
}
int client_only_connect::handle_close(const int, reactor_mask mask) 
{
  if (this->conn_ok_)
    this->cli_mgr_->on_conn_close();
  else
    this->cli_mgr_->on_conn_failed();

  delete this;
  return 0;
}
//----------------------------------------------------------------------
client_send_1_msg::client_send_1_msg(client_mgr *mgr) 
  : client_only_connect(mgr), snd_msg_seq_(SEQ_NO_BEGIN)
{ }
int client_send_1_msg::open(void *)
{
  client_only_connect::open(NULL);
  char buf[sizeof(proto_head) + 2 + 512];
  int msg_size = this->build_msg(buf);
  socket::send(this->get_handle(), buf, msg_size);
  return -1;
}
int client_send_1_msg::build_msg(char *buf)
{
  int str_size = ::rand() % 512 + 1;
  int msg_size = sizeof(proto_head) + 2 + str_size;
  proto_head *head = (proto_head*)buf;
  head->set(this->snd_msg_seq_++, 100001, 0, msg_size);
  *(short*)(buf + sizeof(proto_head)) = str_size;

  return msg_size;
}
//--------------------------------------------------------------------------------
client_keep_alive::~client_keep_alive()
{
  if (this->rcv_buf_)
    mblock_pool::instance()->release(this->rcv_buf_);
  if (this->snd_buf_)
    mblock_pool::instance()->release(this->snd_buf_);
}
int client_keep_alive::open(void *)
{
  client_send_1_msg::open(NULL);
  if (this->get_reactor()->register_handler(this, ev_handler::read_mask) != 0)
    return -1;
  this->rcv_buf_ = mblock_pool::instance()->alloc(1024 * 2);
  this->snd_buf_ = mblock_pool::instance()->alloc(sizeof(proto_head) + 2 + 512);
  return 0;
}
int client_keep_alive::handle_input(const int)
{
  if (this->rcv_buf_->space() == 0)
  {
    int len = this->rcv_buf_->length();
    if (len != 0)
      ::memmove(this->rcv_buf_->data(), this->rcv_buf_->rd_ptr(), len);
    this->rcv_buf_->reset();
    this->rcv_buf_->wr_ptr(len);
  }

  int rcv_len = socket::recv(this->get_handle(), 
                             this->rcv_buf_->wr_ptr(),
                             this->rcv_buf_->space());
  if (rcv_len == 0)
    return -1;
  else if (rcv_len == -1)
  {
    if (errno != EAGAIN)
      return -1;
    else
      rcv_len = 0;
  }

  this->rcv_buf_->wr_ptr(rcv_len);
  return this->handle_data();
}
int client_keep_alive::handle_data()
{
  if (this->rcv_buf_->length() < (int)sizeof(proto_head))
    return 0;

  proto_head *head = (proto_head*)this->rcv_buf_->rd_ptr();
  if (head->len_ < (int)sizeof(proto_head))
  {
    assert(0);
  }
  else if (head->len_ <= this->rcv_buf_->length())
  {
    this->cli_mgr_->on_rcv_pkg();
    int rcv_id = *(int*)(this->rcv_buf_->rd_ptr() + sizeof(proto_head));
    if (rcv_id != this->should_rcv_id_++)
    {
      assert(0);
    }

    this->rcv_buf_->rd_ptr(head->len_);

    //send pkg
    char buf[sizeof(proto_head) + 2 + 512];
    int msg_size = this->build_msg(buf);
    int snd_len = 0;
    while (snd_len < msg_size)
    {
      int snd_ret = socket::send(this->get_handle(), 
                                 buf + snd_len,
                                 msg_size - snd_len);
      if (snd_ret == -1)
      {
        if (errno == EAGAIN)
          break;
        else
          return -1;
      }
      snd_len += snd_ret;
    }

    if (snd_len < msg_size)
    {
      if (this->get_reactor()->register_handler(this, ev_handler::write_mask) != 0)
        return -1;
      this->snd_buf_->copy(buf + snd_len, msg_size - snd_len);
    }

  }
  return 0;
}
int client_keep_alive::handle_output(const int)
{
  while (this->snd_buf_->length() > 0)
  {
    int send_ret = socket::send(this->get_handle(), 
                                this->snd_buf_->rd_ptr(), 
                                this->snd_buf_->length());
    if (send_ret == -1)
    {
      if (errno == EAGAIN)
        break;
      else
        return -1;
    }
    this->snd_buf_->rd_ptr(send_ret);
  }

  if (this->snd_buf_->length() == 0)
  {
    this->snd_buf_->reset(); 
    if (this->get_reactor()->remove_handler(this, ev_handler::write_mask) != 0)
      return -1;
  }

  return 0;
}
