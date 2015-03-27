#include "cross_module.h"
#include "istream.h"
#include "message.h"
#include "player_obj.h"
#include "client.h"
#include "sys_log.h"
#include "error.h"
#include "cross_svc.h"
#include "package_module.h"
#include "cache_module.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("cross");
static ilog_obj *e_log = err_log::instance()->get_ilog("cross");

int cross_module::dispatch_msg(player_obj *player,
                               const int msg_id,
                               const char *msg,
                               const int len,
                               const int res)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = cross_module::FUNC(player, msg, len);   \
  break
#define RES_SHORT_CODE(ID, FUNC) case ID:             \
  ret = cross_module::FUNC(player, msg, len, res);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GET_RIVAL_LIST,    clt_get_rival_list);
    SHORT_CODE(REQ_GET_OTHER_DETAIL,  clt_get_other_detail);

    RES_SHORT_CODE(RES_GET_RIVAL_LIST,    handle_get_rival_list);
    RES_SHORT_CODE(RES_GET_OTHER_DETAIL,  handle_get_other_detail);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int cross_module::clt_get_rival_list(player_obj *player,
                                     const char *,
                                     const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id()
    << stream_ostr("random", ::strlen("random")) << 0
    << REQ_GET_RIVAL_LIST;
  cross_svc::instance()->send_request(REQ_ROUTE_REQ_MSG, &os);
  return 0;
}
int cross_module::clt_get_other_detail(player_obj *player,
                                       const char *msg,
                                       const int len)
{
  in_stream is(msg, len);
  char group[MAX_GROUP_NAME_LEN + 1] = {0};
  int svc_id = 0, char_id = 0;
  stream_istr group_str(group, sizeof(group));
  is >> group_str >> svc_id >> char_id;

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id()
    << stream_ostr(group, ::strlen(group)) << svc_id
    << REQ_GET_OTHER_DETAIL << char_id;
  cross_svc::instance()->send_request(REQ_ROUTE_REQ_MSG, &os);
  return 0;
}
int cross_module::handle_get_rival_list(player_obj *player,
                                        const char *msg,
                                        const int len,
                                        const int res)
{
  if (res != 0)
    player->send_respond_err(RES_GET_RIVAL_LIST, res);
  else
  {
    in_stream is(msg, len);
    out_stream os(client::send_buf, client::send_buf_len);
    os << is;
    player->send_respond_ok(RES_GET_RIVAL_LIST, &os);
  }
  return 0;
}
int cross_module::handle_get_other_detail(player_obj *player,
                                          const char *msg,
                                          const int len,
                                          const int res)
{
  if (res != 0)
    player->send_respond_err(RES_GET_OTHER_DETAIL, res);
  else
  {
    in_stream is(msg, len);
    out_stream os(client::send_buf, client::send_buf_len);
    os << is;
    player->send_respond_ok(RES_GET_OTHER_DETAIL, &os);
  }
  return 0;
}
void cross_module::handle_db_cross_get_item_single_pkg_result(const char *msg, const int len)
{
  in_stream is(msg, len);
  int sid = 0, target_id = 0, cnt = 0;
  is >> sid >> target_id >> cnt;

  ilist<item_obj*> *elist = new ilist<item_obj *>();
  for (int i = 0; i < cnt; ++i)
  {
    item_obj *io = package_module::alloc_new_item();
    is.rd_ptr(sizeof(short)); // Refer: db_proxy::proxy_obj.cpp::proc_result::for (mblock
    is >> io;
    elist->push_back(io);
  }
  cache_module::equip_cache_add_player(target_id, elist);

  out_stream os(client::send_buf, client::send_buf_len);
  os << sid << RES_GET_OTHER_DETAIL;

  elist = cache_module::get_player_equip(target_id);
  if (elist != NULL) // cache
  {
    player_obj::do_build_look_detail_info(NULL, target_id, elist, os);
    cross_svc::instance()->send_respond_ok(RES_ROUTE_RES_MSG, &os);
  }
}
