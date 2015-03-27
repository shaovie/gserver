#include "gm_notice_module.h"
#include "notice_info.h"
#include "sys_log.h"
#include "time_util.h"
#include "istream.h"
#include "client.h"
#include "message.h"
#include "db_proxy.h"
#include "ilist.h"
#include "chat_module.h"
#include "player_mgr.h"

// Lib header
#include <map>

static ilog_obj *e_log = err_log::instance()->get_ilog("gm_notice");

typedef std::map<int/*id*/, notice_info *> notice_info_map_t;
typedef std::map<int/*id*/, notice_info *>::iterator notice_info_map_iter;

static notice_info_map_t s_notice_info_map;

bool gm_notice_module::first_check      = true;
int  gm_notice_module::last_check_time  = 0;
int  gm_notice_module::max_notice_id    = 0;

void gm_notice_module::load_notice_from_db(notice_info *ni)
{
  if (gm_notice_module::max_notice_id < ni->id_)
    gm_notice_module::max_notice_id = ni->id_;

  s_notice_info_map.insert(std::make_pair(ni->id_, ni));
}
void gm_notice_module::do_timeout(const int now)
{
  // 5 seconds
  if (now - gm_notice_module::last_check_time < 5) return ;
  gm_notice_module::last_check_time = now;

  ilist<int> remove_notice_list;
  for (notice_info_map_iter iter = s_notice_info_map.begin();
       iter != s_notice_info_map.end();
       ++iter)
  {
    notice_info *ni = iter->second;
    if (now > ni->begin_time_
        && (now < ni->end_time_ || ni->end_time_ == 0)
        && now - ni->last_time_ > ni->interval_time_)
    {
      ni->last_time_ = now;
      if (gm_notice_module::first_check) continue ;

      // broadcast
      mblock mb(client::send_buf, client::send_buf_len);
      mb.wr_ptr(sizeof(proto_head));
      out_stream os(mb.wr_ptr(), mb.space());
      os << stream_ostr(ni->content_, ::strlen(ni->content_));
      mb.wr_ptr(os.length());
      player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_GM_NOTICE, &mb);
    }else if (now > ni->end_time_ && ni->end_time_ != 0)
      remove_notice_list.push_back(ni->id_);

  }
  while(!remove_notice_list.empty())
    gm_notice_module::delete_notice(remove_notice_list.pop_front());

  if (gm_notice_module::first_check)
    gm_notice_module::first_check = false;
}
void gm_notice_module::replace_notice(notice_info &ninfo)
{
  notice_info *ni = NULL;
  notice_info_map_iter iter = s_notice_info_map.find(ninfo.id_);
  if (iter == s_notice_info_map.end())
  {
    ninfo.id_ = gm_notice_module::assign_notice_id();
    if (ninfo.id_ == -1)
    {
      e_log->wning("over max notice count, insert notice failed!");
      return ;
    }
    ni = new notice_info();
    s_notice_info_map.insert(std::make_pair(ninfo.id_, ni));
  }else
  {
    if (time_util::now > ninfo.end_time_ && ninfo.end_time_ != 0)
    {
      gm_notice_module::delete_notice(ninfo.id_);
      return;
    }
    ni = iter->second;
  }

  ::memcpy(ni, &ninfo, sizeof(notice_info));

  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << stream_ostr((char *)ni, sizeof(notice_info));
  db_proxy::instance()->send_request(0, REQ_UPDATE_NOTICE_INFO, &os);
}

int gm_notice_module::assign_notice_id()
{
  if (++gm_notice_module::max_notice_id <= MAX_NOTICE_ID)
    return gm_notice_module::max_notice_id;

  for (int i = 1; i < MAX_NOTICE_ID; ++i)
  {
    if (s_notice_info_map.find(i) == s_notice_info_map.end())
      return i;
  }

  return -1;
}
void gm_notice_module::delete_notice(const int id)
{
  notice_info_map_iter iter = s_notice_info_map.find(id);
  if (iter == s_notice_info_map.end()) return ;

  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << id;
  db_proxy::instance()->send_request(0, REQ_DELETE_NOTICE_INFO, &os);

  delete iter->second;
  s_notice_info_map.erase(id);
}
