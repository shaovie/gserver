#include "db_proxy.h"
#include "player_obj.h"
#include "global_macros.h"
#include "mblock_pool.h"
#include "time_util.h"
#include "sys_log.h"
#include "message.h"
#include "daemon.h"

// Lib header
#include "socket.h"
#include "reactor.h"
#include "connector.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("db_proxy");
static ilog_obj *e_log = err_log::instance()->get_ilog("db_proxy");

static connector<db_proxy_impl> s_connector;

static char s_db_proxy_name[64];
static char s_db_proxy_for[64];
static char s_db_proxy_bin_v[64];
static char s_db_proxy_bin_build_time[64];
static int  s_db_proxy_start_time = 0;

class db_proxy_impl : public client 
{
public:
  db_proxy_impl() :
    client(MAX_INNER_SOCKET_BUF_SIZE, "db proxy"),
    be_connecting_(false),
    last_heart_beat_time_(0),
    port_(0),
    dh_(NULL)
  { }
  int open(db_handler *dh, const int port, const char *host)
  {
    this->port_ = port;
    this->dh_   = dh;
    ::strncpy(this->host_, host, sizeof(this->host_) - 1);
    s_connector.open(this->get_reactor());
    return this->get_reactor()->schedule_timer(this,
                                               time_value(0, 0),
                                               time_value(DB_PROXY_HEART_BEAT, 0)) == -1 ? -1 : 0;
  }
  virtual int open(void *)
  {
    if (this->get_reactor()->register_handler(this,
                                              ev_handler::read_mask) != 0)
    {
      e_log->error("register handle %d failed! %s:%d", 
                   this->get_handle(),
                   __FILE__, 
                   __LINE__);
      return -1;
    }

    s_log->rinfo("connect to db proxy %s:%d ok! h=%d",
                 this->host_,
                 this->port_,
                 this->get_handle());

    socket::set_nodelay(this->get_handle());
    socket::set_sndbuf(this->get_handle(), MAX_INNER_SOCKET_BUF_SIZE);

    this->sock_status_ = _SOCK_CON_OK;
    this->be_connecting_ = false;

    this->last_heart_beat_time_ = time_util::now;
    return this->do_login();
  }
  virtual int handle_timeout(const time_value &now)
  {
    if (this->sock_status_ == _SOCK_INIT
        && !this->be_connecting_)
    {
      this->be_connecting_ = true;
      inet_address remote_addr(this->port_, this->host_);
      s_connector.connect(this, remote_addr, NULL, MAX_INNER_SOCKET_BUF_SIZE);
      s_log->rinfo("begin to connect db proxy %s:%d!", this->host_, this->port_);
      return 0;
    }else if (this->sock_status_ == _SOCK_ACTIVE
              || this->sock_status_ == _SOCK_CON_OK)
    {
      if (this->do_heart_beat() == 0
          && this->check_heart_beat(now) == 0)
        return 0;
      return -1;
    }
    return 0;
  }
  virtual int handle_close(const int , reactor_mask m)
  {
    if (m == ev_handler::write_mask)
    {
      if (this->sock_status_ != _SOCK_SEND_ERROR)
        return 0;
    }
    e_log->rinfo("handle db proxy close h=%d m=%d %s:%d![%s]",
                 this->get_handle(),
                 m,
                 this->host_,
                 this->port_,
                 g_sock_status[this->sock_status_]);

    this->shutdown_i();

    this->recv_buff_->reset();

    while (!this->buff_list_.empty())
    {
      mblock *h = this->buff_list_.pop_front();
      mblock_pool::instance()->release(h);
    }

    this->be_connecting_ = false;
    this->sock_status_ = _SOCK_INIT;
    if (this->get_reactor()->schedule_timer(this,
                                            time_value(2, 0),
                                            time_value(DB_PROXY_HEART_BEAT, 0)) == -1)
    {
      e_log->error("schedule timer failed! %s:%d", __FILE__, __LINE__);
    }
    return 0;
  }
  int do_login()
  { return this->send_request(REQ_DB_PROXY_LOGIN, NULL); }
  int handle_login_result(const char *msg, const int len, const int )
  {
    in_stream is(msg, len);
    stream_istr name_si(s_db_proxy_name, sizeof(s_db_proxy_name));
    stream_istr for_si(s_db_proxy_for, sizeof(s_db_proxy_for));
    stream_istr bin_v_si(s_db_proxy_bin_v, sizeof(s_db_proxy_bin_v));
    stream_istr bin_build_time_si(s_db_proxy_bin_build_time, sizeof(s_db_proxy_bin_build_time));
    is >> name_si
      >> for_si
      >> bin_v_si
      >> bin_build_time_si
      >> s_db_proxy_start_time;
    return 0;
  }
  int do_heart_beat()
  { return this->send_request(REQ_DB_PROXY_HEART_BEAT, NULL); }
  int handle_heart_beat_result(const char *, const int , const int )
  {
    this->last_heart_beat_time_ = time_util::now;
    return 0;
  }
  int check_heart_beat(const time_value &now)
  {
    int diff_v = (int)(now.sec() - this->last_heart_beat_time_);
    if (diff_v > DB_PROXY_HEART_BEAT*2)
    {
      e_log->rinfo("heart beat time out - diff: %d", diff_v);
      return -1;
    }
    return 0;
  }
  virtual int dispatch_msg(const int id, const int res, const char *msg, const int len)
  {
    if (id == RES_DB_PROXY_HEART_BEAT)
      return this->handle_heart_beat_result(msg, len, res);
    else if (id == RES_DB_PROXY_LOGIN)
      return this->handle_login_result(msg, len, res);
    return this->dh_->dispatch_msg(id, res, msg, len);
  }
private:
  bool be_connecting_;
  int  last_heart_beat_time_;
  int  port_;
  db_handler *dh_;
  char host_[MAX_HOST_NAME + 1];
};
db_proxy::db_proxy() :
  proxy_num_(3),
  impl_(NULL)
{ }
int db_proxy::open(reactor *r,
                   db_handler *dh,
                   const int num,
                   const int port,
                   const char *host)
{
  this->proxy_num_ = num;
  this->impl_ = new db_proxy_impl[this->proxy_num_]();
  for (int i = 0; i < this->proxy_num_; ++i)
  {
    this->impl_[i].set_reactor(r);
    if (this->impl_[i].open(dh, port, host) != 0)
      return -1;
  }
  return 0;
}
const char *db_proxy::version()
{
  int ret = 0;
  return build_version_info(s_db_proxy_name,
                            s_db_proxy_for,
                            s_db_proxy_bin_v,
                            s_db_proxy_bin_build_time,
                            time_util::now - s_db_proxy_start_time,
                            ret);
}
int db_proxy::send_request(const int char_id, const int msg_id, out_stream *os)
{ return this->impl_[::abs(char_id % this->proxy_num_)].send_request(msg_id, os); }
