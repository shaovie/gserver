#include "proxy_obj.h"
#include "proxy_mgr.h"
#include "mblock_pool.h"
#include "db_async_store.h"
#include "global_macros.h"
#include "sql_measuring.h"
#include "tb_msg_map.h"
#include "istream.h"
#include "message.h"
#include "sys_log.h"
#include "daemon.h"
#include "def.h"

// Lib header
#include "ilist.h"
#include "mblock.h"
#include "socket.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

proxy_obj::proxy_obj() :
  client(MAX_INNER_SOCKET_BUF_SIZE, "proxy client"),
  proxy_id_(-1),
  async_result_head_(NULL),
  async_result_tail_(NULL),
  notify_(this)
{ }
proxy_obj::~proxy_obj()
{
  if (this->proxy_id_ != -1)
  {
    s_log->rinfo("remove proxy obj id = %d", this->proxy_id_);
    proxy_mgr::instance()->remove(this->proxy_id_);
    proxy_mgr::instance()->release_id(this->proxy_id_);
  }
  this->notify_.close();
  for (mblock *itor = this->async_result_head_; itor != NULL; )
  {
    mblock *t = itor;
    itor = itor->next();
    async_result *ar = (async_result *)t->data();
    ar->destroy();
    mblock_pool::instance()->release(t);
  }
}
int proxy_obj::open(void *)
{
  socket::set_nodelay(this->get_handle());
  socket::set_sndbuf(this->get_handle(), MAX_INNER_SOCKET_BUF_SIZE);

  if (this->get_reactor()->register_handler(this,
                                            ev_handler::read_mask) == -1)
  {
    e_log->rinfo("register handle %d failed!", this->get_handle());
    return -1;
  }
  if (this->notify_.open(this->get_reactor()) == -1)
  {
    e_log->rinfo("proxy notify open failed![%s]", strerror(errno));
    return -1;
  }

  if ((this->proxy_id_ = proxy_mgr::instance()->alloc_id()) == -1)
  {
    e_log->error("alloc proxy_id failed![proxy number is out of limit]");
    return -1;
  }

  char bf[32] = {0};
  this->remote_addr_.get_host_addr(bf, sizeof(bf));

  s_log->rinfo("new %s connection fd=%d id=%d -> [%s:%d]", 
               this->client_type_,
               this->get_handle(),
               this->proxy_id_,
               bf,
               this->remote_addr_.get_port_number());

  this->sock_status_ = _SOCK_CON_OK;

  proxy_mgr::instance()->insert(this->proxy_id_, this);
  return 0;
}
int proxy_obj::dispatch_msg(const int id, 
                            const int /*res*/,
                            const char *msg, 
                            const int len)
{
  if (id == REQ_DB_PROXY_HEART_BEAT)
    return this->send_respond_ok(RES_DB_PROXY_HEART_BEAT);
  else if (id == REQ_DB_PROXY_LOGIN)
    return this->clt_login(msg, len);
  else if (id == REQ_DB_PROXY_PAYLOAD)
    return this->clt_get_payload(msg, len);

  return tb_msg_map::instance()->call(this, id, msg, len);
}
int proxy_obj::clt_login(const char *, const int )
{
  struct tm tb;
  strptime(__DATE__, "%b %d %Y", &tb);
  char date_bf[16] = {0};
  strftime(date_bf, sizeof(date_bf), "%Y-%m-%d", &tb);
  char build_time[64] = {0};
  ::snprintf(build_time, sizeof(build_time), "%s %s", date_bf, __TIME__);

  out_stream os(client::send_buf, client::send_buf_len);
  os << stream_ostr(g_svc_name, ::strlen(g_svc_name))
    << stream_ostr(SVC_EDITION, ::strlen(SVC_EDITION))
    << stream_ostr(BIN_V, ::strlen(BIN_V))
    << stream_ostr(build_time, ::strlen(build_time))
    << (int)time_value::start_time.sec();

  return this->send_respond_ok(RES_DB_PROXY_LOGIN, &os);
}
int proxy_obj::clt_get_payload(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  sql_measuring::instance()->do_measure(os);
  return this->send_respond_ok(RES_DB_PROXY_PAYLOAD, &os);
}
void proxy_obj::post_result(mblock *mb)
{
  guard<thread_mutex> g(this->async_result_list_mtx_);
  if (this->async_result_tail_ == NULL)
  {
    this->async_result_head_ = mb;
    this->async_result_tail_ = mb;
  }else
  {
    this->async_result_tail_->next(mb);
    this->async_result_tail_ = mb;
  }
  this->notify_.notify();
}
void proxy_obj::_notify::handle_notify()
{
  mblock *head = NULL;
  {
    guard<thread_mutex> g(this->proxy_obj_->async_result_list_mtx_);
    head = this->proxy_obj_->async_result_head_;
    this->proxy_obj_->async_result_head_ = NULL;
    this->proxy_obj_->async_result_tail_ = NULL;
  }
  int err = 0;
  for (mblock *itor = head; itor != NULL; )
  {
    mblock *t = itor;
    itor = itor->next();
    async_result *ar = (async_result *)t->data();
    if (this->proxy_obj_->proc_result(ar) != 0)
      err = -1;
    ar->destroy();
    mblock_pool::instance()->release(t);
  }
  if (err == -1)
  {
    this->proxy_obj_->sock_status_ = client::_SOCK_SEND_ERROR;
    this->proxy_obj_->shutdown(); // destroy
  }
}
int proxy_obj::proc_result(async_result *ar)
{
  if (ar->result_ != DB_ASYNC_OK)
  {
    for (mblock *itor = ar->result_head_; itor != NULL; itor = itor->next())
    {
      e_log->rinfo("msg %d do sql failed![%s][%d]",
                   ar->resp_id_,
                   itor->rd_ptr(),
                   ar->result_);
    }
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << ar->db_sid_
    << ar->arg_
    << ar->result_cnt_;
  for (mblock *itor = ar->result_head_; itor != NULL; itor = itor->next())
    os << itor;

  if ((int)os.length() == client::send_buf_len)
    e_log->fatal("msg %d body is too large !", ar->resp_id_);

  return this->send_respond(ar->resp_id_, ar->result_, &os);
}
int proxy_obj::proc_result(const int db_sid,
                           const int arg,
                           const int resp_id,
                           const int result,
                           const int rs_len,
                           const char *res)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << db_sid << arg;
  if (rs_len == 0)
    os << (int)0;
  else
    os << (int)1 << stream_ostr(res, rs_len);

  if ((int)os.length() == client::send_buf_len)
    e_log->fatal("msg %d body is too large !", resp_id);

  return this->send_respond(resp_id, result, &os);
}
