#include "world_svc.h"
#include "time_value.h"
#include "world_svc_mgr.h"
#include "session_mgr.h"
#include "request_mgr.h"
#include "time_util.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("world_svc");
static ilog_obj *e_log = err_log::instance()->get_ilog("world_svc");

world_svc::world_svc() :
  client(CLIENT_RECV_BUFF_SIZE, "world svc"),
  login_ok_(false),
  sid_(0),
  svc_id_(-1),
  last_heart_beat_(0)
{
  ::memset(group_, 0, sizeof(group_));
}
world_svc::~world_svc()
{
  if (this->login_ok_)
  {
    world_svc_mgr::instance()->remove(this->group_, this->svc_id_);
    session_mgr::instance()->remove(this->sid_);
  }
}
int world_svc::open(void *)
{
  if (client::open(NULL) == -1)
    return -1;

  socket::set_nodelay(this->get_handle());
  socket::set_sndbuf(this->get_handle(), MAX_INNER_SOCKET_BUF_SIZE);

  this->last_heart_beat_ = time_util::now;

  int ret = this->get_reactor()->schedule_timer(this,
                                                time_value(5, 0),
                                                time_value(CROSS_HEART_BEAT, 0));
  return ret == -1 ? -1 : 0;
}
int world_svc::handle_timeout(const time_value &now)
{
  if ((int)now.sec() - this->last_heart_beat_ > CROSS_HEART_BEAT * 3)
  {
    e_log->rinfo("world pf=%s id=%d heart beat timeout [diff = %d]",
                 this->group_, this->svc_id_,
                 (int)now.sec() - this->last_heart_beat_);
    return -1;
  }
  if (!this->login_ok_)
  {
    e_log->rinfo("world don't login for a long time! h=%d", this->get_handle());
    return -1;
  }
  return 0;
}
int world_svc::dispatch_msg(const int id,
                            const int res,
                            const char *msg,
                            const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:   \
  ret = this->FUNC(msg, len);           \
  break
#define ROUTE_SHORT_CODE(ID, FUNC) case ID:   \
  ret = this->FUNC(res, msg, len);           \
  break
#define SHORT_DEFAULT default:          \
  e_log->error("unknow msg id %d", id); \
  break

  if (!this->login_ok_
      && id != REQ_WORLD_LOGIN)
  {
    e_log->wning("handle msg %d before login ok", id);
    return -1;
  }

  int ret = 0;
  switch (id)
  {
    SHORT_CODE(REQ_WORLD_HEART_BEAT, wld_heart_beat);
    SHORT_CODE(REQ_WORLD_LOGIN, wld_login);
    ROUTE_SHORT_CODE(REQ_ROUTE_REQ_MEG, wld_route_req_msg);
    ROUTE_SHORT_CODE(RES_ROUTE_RES_MEG, wld_route_res_msg);
    SHORT_DEFAULT;
  }

  if (ret != 0)
    e_log->wning("handle msg %d return %d", id, ret);
  return ret;
}
int world_svc::wld_heart_beat(const char *, const int )
{
  this->last_heart_beat_ = time_util::now;
  return this->send_respond_ok(RES_WORLD_HEART_BEAT);
}
int world_svc::wld_login(const char *msg, const int len)
{
  if (this->login_ok_)
  {
    e_log->rinfo("world pf=%s id=%d has been login ok!",
                 this->group_, this->svc_id_);
    return -1;
  }

  in_stream is(msg, len);
  stream_istr group(this->group_, sizeof(this->group_));
  is >> group >> this->svc_id_;
  if (group.str_len() == 0 || this->svc_id_ < 0)
  {
    e_log->rinfo("world pf=%s id=%d, id is error!",
                 this->group_, this->svc_id_);
    return -1;
  }
  this->login_ok_ = true;
  world_svc_mgr::instance()->insert(this->group_, this->svc_id_, this);
  this->sid_ = session_mgr::instance()->alloc_session_id();
  session_mgr::instance()->insert(this->sid_, this);
  s_log->rinfo("world group=%s id=%d h=%d sid=%d login ok!",
               this->group_, this->svc_id_, this->get_handle(), this->sid_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->sid_;
  return this->send_respond_ok(RES_WORLD_LOGIN, &os);
}
int world_svc::wld_route_req_msg(const int , const char *msg, const int len)
{
  in_stream is(msg, len);
  char group[MAX_GROUP_NAME_LEN + 1] = {0};
  int svc_id = 0, data = 0, message = 0;
  stream_istr group_str(group, sizeof(group));
  is >> data >> group_str >> svc_id >> message;

  world_svc *aim_ws = NULL;
  if (::strcmp(group, "random") == 0)
    aim_ws = session_mgr::instance()->random();
  else if (svc_id == 0)
    aim_ws = world_svc_mgr::instance()->random(group);
  else
    aim_ws = world_svc_mgr::instance()->find(group, svc_id);
  if (aim_ws == NULL)
  {
    s_log->debug("Can't find world_svc, param data:%d group:%s svc_id:%d message:%d",
                 data, group, svc_id, message);
    return 0;
  }

  int sid = request_mgr::instance()->store(this, data);
  out_stream os(client::send_buf, client::send_buf_len);
  os << sid << is;
  aim_ws->send_request(message, &os);
  return 0;
}
int world_svc::wld_route_res_msg(const int res, const char *msg, const int len)
{
  int sid = 0, message = 0, data = 0;
  in_stream is(msg, len);
  is >> sid >> message;

  world_svc *res_ws = request_mgr::instance()->get(sid, data);
  if (res_ws == NULL)
  {
    s_log->debug("Can't find world_svc, param sid:%d message:%d", sid, message);
    return 0;
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << data << is;
  res_ws->send_msg(message, res, &os);
  return 0;
}
