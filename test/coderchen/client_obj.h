#ifndef CLIENT_OBJ_H_
#define CLIENT_OBJ_H_

#include "svc_handler.h"

class mblock;
class client_mgr;
class client_only_connect : public svc_handler
{
public:
  client_only_connect(client_mgr *mgr) 
    : conn_ok_(false), cli_mgr_(mgr)
  { }
  virtual ~client_only_connect() { }
  virtual int open(void *);
  virtual int handle_close(const int, reactor_mask mask);
protected:
  bool conn_ok_;
  client_mgr *cli_mgr_;
};

class client_send_1_msg : public client_only_connect
{
public:
  client_send_1_msg(client_mgr *mgr);
  virtual ~client_send_1_msg(){ }
  virtual int open(void *);
protected:
  int build_msg(char *buf); // return msg_size
private:
  int snd_msg_seq_;
};

class client_keep_alive : public client_send_1_msg
{
public:
  client_keep_alive(client_mgr *mgr) 
    : client_send_1_msg(mgr), should_rcv_id_(1), rcv_buf_(NULL), snd_buf_(NULL)
  { }
  virtual ~client_keep_alive();
  virtual int open(void *);
  virtual int handle_input(const int);
  virtual int handle_output(const int);
private:
  int handle_data();
  int should_rcv_id_;
  mblock *rcv_buf_;
  mblock *snd_buf_;  
};

#endif
