#ifndef CONNECT_KEEP_ALIVE_H_
#define CONNECT_KEEP_ALIVE_H_

#include <iostream>
#include "head.h"
#include "mblock_pool.h"
#include "client.h"
#include "connect_mgr.h"
#include "def.h"

extern int g_success_connect_num;
extern int g_failed_connect_num;
extern int g_all_send_num;
extern int g_receive_right_num;
extern int g_discard_num;

class client_keep_alive : public client
{
public:
  client_keep_alive():
    client(2048, "keep_alive"),
    conn_ok_(false),
    seq_(SEQ_BEGIN),
    msg_id_(MSG_ID_BEGIN),
    recv_body_id_(1)
  { }
  virtual ~client_keep_alive()
  { }
  virtual int open(void * = NULL)
  {
    ++g_success_connect_num;
    if (client::open(NULL) != 0)
      return -1;
    this->conn_ok_ = true;
    return this->send_next_msg();
  }
  virtual int send_next_msg()
  {
    char mb[1024] = {0};
    int msg_len = ::rand()%(512 + 1);
    proto_head *head = (proto_head *)mb;
    head->set(this->seq_, this->msg_id_, 0, msg_len + sizeof(proto_head) + sizeof(short));
    *(short *)(mb + sizeof(proto_head)) = msg_len;
    if (this->send_data(mb, msg_len + sizeof(proto_head) + sizeof(short)) != 0)
    {
      this->conn_ok_ = false;
    }else
    {
      ++g_all_send_num;
      ++this->seq_;
    }
    return 0;
  }
  virtual int handle_timeout(const time_value &)
  {
    if (!this->conn_ok_)
    {
      ++g_failed_connect_num;
      return -1;
    }
    return 0;
  }
  virtual int handle_data()
  {
    while(this->recv_buff_->length() > sizeof(proto_head))
    {
      proto_head *ph = (proto_head*)(this->recv_buff_->rd_ptr());
      int msg_len = ph->len_;
      if (this->recv_buff_->length() >= msg_len)
      {
        //std::cout << "handle one package " << std::endl;
        this->recv_buff_->rd_ptr(sizeof(proto_head));
        int body_id = *((int *)this->recv_buff_->rd_ptr());
        if (body_id != this->recv_body_id_)
        {
          int discard_num = body_id - this->recv_body_id_;
          g_discard_num +=  discard_num;
          this->recv_body_id_ = body_id + 1;
        }
        else 
        {
          ++g_receive_right_num;
          ++this->recv_body_id_;
          this->send_next_msg();
        }
        this->recv_buff_->rd_ptr(msg_len - sizeof(proto_head));
      }else
        break;
    }
    return 0;
  }
  virtual int dispatch_msg(const int, const int, const char *, const int)
  { return 0; }
private:
  bool conn_ok_;
  int seq_;
  int msg_id_;
  int recv_body_id_;
};
#endif
