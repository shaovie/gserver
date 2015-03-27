// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-29 13:55
 */
//========================================================================

#ifndef CLIENT_H_
#define CLIENT_H_

// Lib header
#include "ilist.h"
#include "svc_handler.h"

// Forward declarations
class mblock;
class out_stream;

extern const char *g_sock_status[];

/**
 * @class client
 * 
 * @brief
 */
class client : public svc_handler
{
public:
  enum
  {
    _SOCK_INIT    = 0,
    _SOCK_CON_OK,                       // 连接建立成功
    _SOCK_ACTIVE,                       // 连接开始传输数据
    _SOCK_CLOSED_AS_TIMEOUT,            // 由于接收数据超时被关闭
    _SOCK_CLOSED_BY_SVC,                // 连接被服务器主动断开
    _SOCK_RESET_BY_PEER,                // 连接被对方重置
    _SOCK_SEND_ERROR,                   // 发送数据遇到异常
    _SOCK_CLOSED_BY_PEER,               // 连接被对方主动关闭
    _SOCK_PACKAGE_IS_TOO_LARGE,         // 数据包太大
    _SOCK_SND_BUF_IS_FULL,              // 发送缓冲区满
    _SOCK_ERROR,                        // 连接检测到错误
    _SOCK_END
  };
  client(const int recv_buf_len, const char *client_type);

  virtual ~client();

  const char *client_type();

  //=
  virtual int open(void *);

  virtual int handle_input(const int );

  virtual int handle_output(const int );

  virtual int handle_close(const int , reactor_mask m);

  virtual int handle_data();

  virtual void set_remote_addr(const inet_address &rm);

  //=
  void shutdown();

  int send_data(const char *msg, const int len);

  void get_remote_addr(char *addr, const int len);

  //=
  int send_request(const int msg_id, out_stream *os)
  { return this->send_msg(msg_id, 0, os); }
  int send_respond(const int msg_id, const int res, out_stream *os = NULL)
  { return this->send_msg(msg_id, res, os); }
  int send_respond_ok(const int msg_id, out_stream *os = NULL)
  { return this->send_msg(msg_id, 0, os); }
  int send_respond_err(const int msg_id, const int res)
  { return this->send_msg(msg_id, res, NULL); }
  int send_msg(const int msg_id, const int res, out_stream *);

protected:
  virtual int dispatch_msg(const int id, 
                           const int result,
                           const char *msg,
                           const int len) = 0;
  virtual int check_package_seq(const int ) { return 0; }
protected:
  int seq_;
  int sock_status_;

  const char *client_type_;

  mblock *recv_buff_;

  ilist<mblock *> buff_list_;

  inet_address remote_addr_;
public:
  static int   send_buf_len;
  static char *send_buf;
};

#endif // CLIENT_H_

