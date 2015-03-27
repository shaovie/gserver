#include "cross_svc.h"
#include "global_macros.h"
#include "sys_log.h"
#include "sys.h"
#include "mblock.h"
#include "mblock_pool.h"
#include "istream.h"
#include "svc_config.h"
#include "message.h"
#include "time_util.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "cross_module.h"
#include "cache_module.h"
#include "db_proxy.h"
#include "package_module.h"

// Lib header
#include "reactor.h"
#include "connector.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("cross_svc");
static ilog_obj *e_log = err_log::instance()->get_ilog("cross_svc");

static connector<cross_svc> s_connector;

cross_svc::cross_svc()
  : client(MAX_INNER_SOCKET_BUF_SIZE, "cross svc"),
  login_ok_(false),
  be_connecting_(false),
  last_heart_beat_time_(0),
  port_(0),
  sid_(0)
{
  ::memset(host_, 0, sizeof(host_));
}
int cross_svc::init(const char *host, const int port)
{
  s_connector.open(sys::r);
  this->port_ = port;
  ::strncpy(this->host_, host, sizeof(this->host_) - 1);
  return sys::r->schedule_timer(this,
                                time_value(0, 0),
                                time_value(CROSS_HEART_BEAT, 0)
                               ) == -1 ? -1 : 0;
}
int cross_svc::open(void *)
{
  int ret = this->get_reactor()->register_handler(this,
                                                  ev_handler::read_mask);
  if (ret == -1)
  {
    e_log->error("register handle %d failed! %s:%d",
                 this->get_handle(),
                 __FILE__,
                 __LINE__);
    return -1;
  }

  s_log->rinfo("connect to cross %s:%d ok! h = %d",
               this->host_,
               this->port_,
               this->get_handle());

  socket::set_nodelay(this->get_handle());
  socket::set_sndbuf(this->get_handle(), MAX_INNER_SOCKET_BUF_SIZE);
  this->sock_status_   = _SOCK_CON_OK;
  this->be_connecting_ = false;

  return this->do_login();
}
int cross_svc::handle_timeout(const time_value &now)
{
  if (this->sock_status_ == _SOCK_INIT && !this->be_connecting_)
  {
    s_log->rinfo("connecting to cross %s:%d ....",
                 this->host_,
                 this->port_);
    inet_address remote_addr(this->port_, this->host_);
    s_connector.connect(this, remote_addr, NULL,
                        MAX_INNER_SOCKET_BUF_SIZE);
    this->be_connecting_ = true;
    return 0;
  }else if (this->sock_status_ == _SOCK_ACTIVE
            && this->login_ok_)
  {
    if (this->do_heart_beat() == 0
        && this->check_heart_beat(now) == 0)
      return 0;
    return -1;
  }

  return 0;
}
int cross_svc::handle_close(const int , reactor_mask m)
{
  if (m == ev_handler::write_mask)
  {
    if (this->sock_status_ != _SOCK_SEND_ERROR)
      return 0;
  }

  if (m == ev_handler::connect_mask)
    s_log->rinfo("connect failed! h = %d", this->get_handle());
  else
    s_log->rinfo("connection disconnect! h = %d", this->get_handle());
  this->shutdown();
  return 0;
}
void cross_svc::shutdown()
{
  // 清理数据
  this->shutdown_i();

  this->recv_buff_->reset();
  while (!this->buff_list_.empty())
  {
    mblock *h = this->buff_list_.pop_front();
    mblock_pool::instance()->release(h);
  }

  this->sock_status_   = _SOCK_INIT;
  this->be_connecting_ = false;
  this->login_ok_      = false;

  // 开始下一次连接
  int ret = sys::r->schedule_timer(this,
                                   time_value(3, 0),
                                   time_value(CROSS_HEART_BEAT, 0));
  if (ret == -1)
    e_log->error("schedule timer failed! %s:%d", __FILE__, __LINE__);
}
int cross_svc::do_login()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << stream_ostr(svc_config::instance()->group(),
                    ::strlen(svc_config::instance()->group()))
    << svc_config::instance()->service_sn();
  return this->send_request(REQ_WORLD_LOGIN, &os);
}
int cross_svc::do_heart_beat()
{
  if (!this->login_ok_) return 0;
  return this->send_request(REQ_WORLD_HEART_BEAT, NULL);
}
int cross_svc::check_heart_beat(const time_value &now)
{
  int diff_v = (int)now.sec() - this->last_heart_beat_time_;
  if (diff_v > CROSS_HEART_BEAT * 3)
  {
    e_log->rinfo("world cross heart beat time out - diff: %d", diff_v);
    return -1;
  }
  return 0;
}

int cross_svc::dispatch_msg(const int id,
                            const int res,
                            const char *msg,
                            const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:       \
  ret = this->FUNC(msg, len, res);        \
  break

  int ret = 0;
  switch (id)
  {
    SHORT_CODE(RES_WORLD_HEART_BEAT, handle_heart_beat_result);
    SHORT_CODE(RES_WORLD_LOGIN, handle_login_result);

    SHORT_CODE(REQ_GET_RIVAL_LIST, crs_get_rival_list);
    SHORT_CODE(REQ_GET_OTHER_DETAIL, crs_get_other_detail);

  default:
    {
      in_stream is(msg, len);
      int char_id = -1;
      is >> char_id;
      if (char_id == GLOBAL_SID_COMMUNICATE_WITH_CROSS)
        ret = this->dispatch_res_from_other_world(id, res, is.rd_ptr(), is.length());
      else
      {
        player_obj *player = player_mgr::instance()->find(char_id);
        if (player != NULL)
          ret = cross_module::dispatch_msg(player, id, is.rd_ptr(), is.length(), res);
        else
          e_log->wning("msg %d  can't find char %d", id, char_id);
      }
    }
  }

  if (ret != 0)
    e_log->wning("handle msg %d return %d", id, ret);
  return ret;
}
int cross_svc::dispatch_res_from_other_world(const int id,
                                             const int /*res*/,
                                             const char * /*msg*/,
                                             const int /*len*/)
{
#define SHORT_CODE(ID, FUNC) case ID:       \
  ret = this->FUNC(msg, len, res);        \
  break
#define SHORT_DEFAULT default:              \
  e_log->error("unkown msg id %d", id);   \
  break

  int ret = 0;
  switch (id)
  {
    SHORT_DEFAULT;
  }

  if (ret != 0)
    e_log->wning("handle msg %d return %d", id, ret);
  return ret;
}
int cross_svc::handle_heart_beat_result(const char *,
                                        const int ,
                                        const int )
{
  this->last_heart_beat_time_ = time_util::now;
  return 0;
}
int cross_svc::handle_login_result(const char *msg,
                                   const int len,
                                   const int )
{
  in_stream is(msg, len);
  is >> this->sid_;

  this->login_ok_ = true;
  this->last_heart_beat_time_ = time_util::now;
  return 0;
}
int cross_svc::crs_get_rival_list(const char *msg,
                                  const int len,
                                  const int )
{
  int sid = 0;
  in_stream is(msg, len);
  is >> sid;
  out_stream os(client::send_buf, client::send_buf_len);
  os << sid << RES_GET_RIVAL_LIST << 0;
  return this->send_respond_ok(RES_ROUTE_RES_MSG, &os);
}
int cross_svc::crs_get_other_detail(const char *msg,
                                    const int len,
                                    const int )
{
  int sid = 0, target_id = 0;
  in_stream is(msg, len);
  is >> sid >> target_id;

  out_stream os(client::send_buf, client::send_buf_len);
  os << sid;
  player_obj *player = player_mgr::instance()->find(target_id);
  if (player == NULL)
  {
    ilist<item_obj *> *elist = cache_module::get_player_equip(target_id);
    if (elist != NULL) // cache
    {
      os << RES_GET_OTHER_DETAIL;
      player_obj::do_build_look_detail_info(NULL, target_id, elist, os);
    }
    else // miss
    {
      os << target_id << (char)PKG_EQUIP;
      db_proxy::instance()->send_request(target_id, REQ_CROSS_GET_ITEM_SINGLE_PKG, &os);
      return 0;
    }
  }else // online
  {
    os << RES_GET_OTHER_DETAIL;
    static ilist<item_obj*> equip_list;
    equip_list.clear();
    package_module::find_all_item_in_pkg(player, PKG_EQUIP, equip_list);
    player_obj::do_build_look_detail_info(player, target_id, &equip_list, os);
  }
  return this->send_respond_ok(RES_ROUTE_RES_MSG, &os);
}
