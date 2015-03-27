#include "svc_monitor.h"
#include "player_mgr.h"
#include "monster_mgr.h"
#include "time_util.h"
#include "db_proxy.h"
#include "message.h"
#include "istream.h"
#include "svc_config.h"
#include "scene_config.h"
#include "scp_mgr.h"
#include "scene_mgr.h"
#include "scene_unit.h"
#include "clsid.h"
#include "sys.h"

// Lib header
#include "reactor.h"

/**
 * @class svc_monitor_impl
 * 
 * @brief implement of svc_monitor
 */
class svc_monitor_impl
{
public:
  svc_monitor_impl() :
    db_begin_time_(time_util::now),
    db_end_time_(time_util::now),
    insert_cnt_(0),
    max_insert_cnt_(0),
    update_cnt_(0),
    max_update_cnt_(0),
    select_cnt_(0),
    max_select_cnt_(0),
    delete_cnt_(0),
    max_delete_cnt_(0),
    last_update_db_proxy_payload_time_(time_util::now)
  { }

  void run(const int now)
  {
    this->to_update_db_proxy_payload(now);
  }
  void to_update_db_proxy_payload(const int now)
  {
    if (now - this->last_update_db_proxy_payload_time_ < 180)
      return ;
    
    this->last_update_db_proxy_payload_time_ = now;
    db_proxy::instance()->send_request(0, REQ_DB_PROXY_PAYLOAD, NULL);
  }
  void handle_get_db_proxy_payload_result(const char *msg, const int len)
  {
    in_stream is(msg, len);
    is >> this->db_begin_time_
      >> this->db_end_time_
      >> this->insert_cnt_ >> this->max_insert_cnt_
      >> this->update_cnt_ >> this->max_update_cnt_
      >> this->select_cnt_ >> this->max_select_cnt_
      >> this->delete_cnt_ >> this->max_delete_cnt_;
  }
  int do_build_online(char *bf, const int bf_len)
  {
    int len = 0;
    len += ::snprintf(bf + len, bf_len - len, "\"online\": %d, ",
                      player_mgr::instance()->size());
    len += ::snprintf(bf + len, bf_len - len, "\"online_limit\": %d, ",
                      svc_config::instance()->max_online());
    len += ::snprintf(bf + len, bf_len - len, "\"monster\": %d, ",
                      monster_mgr::instance()->size());
    len += ::snprintf(bf + len, bf_len - len, "\"scp\": %d, ",
                      scp_mgr::instance()->size());
    len += ::snprintf(bf + len, bf_len - len, "\"timer\": %d",
                      sys::r->timer_size());
    return len;
  }
  int do_build_scene_online(char *bf, const int bf_len)
  {
    int len = 0;
    ilist<int> &sl = scene_config::instance()->scene_list();
    for (ilist_node<int> *itor = sl.head();
         itor != NULL;
         itor = itor->next_)
    {
      int scene_cid = itor->value_;
      if (clsid::is_scp_scene(scene_cid)
          || clsid::is_tui_tu_scp_scene(scene_cid))
        continue; 
      len += ::snprintf(bf + len, bf_len - len, "\"%d\": {\"player\": %d, \"monster\": %d}, ",
                        scene_cid,
                        scene_mgr::instance()->unit_cnt(scene_cid, scene_unit::PLAYER),
                        scene_mgr::instance()->unit_cnt(scene_cid, scene_unit::MONSTER));
    }
    if (len > 0)
    {
      len -= 2;
      bf[len] = '\0';
    }
    return len;
  }
  int do_build_db_proxy_payload(char *bf, const int bf_len)
  {
    int len = 0;
    char time_bf[64] = {0};
    date_time(this->db_begin_time_).to_str(time_bf, sizeof(time_bf));
    len += ::snprintf(bf + len, bf_len - len, "\"begin\": \"%s\", ", time_bf);
    date_time(this->db_end_time_).to_str(time_bf, sizeof(time_bf));
    len += ::snprintf(bf + len, bf_len - len, "\"end\": \"%s\", ", time_bf);
    len += ::snprintf(bf + len, bf_len - len, "\"insert\": %d, ", this->insert_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"max-insert\": %d, ", this->max_insert_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"update\": %d, ", this->update_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"max-update\": %d, ", this->max_update_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"select\": %d, ", this->select_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"max-select\": %d, ", this->max_select_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"delete\": %d, ", this->delete_cnt_);
    len += ::snprintf(bf + len, bf_len - len, "\"max-delete\": %d", this->max_delete_cnt_);
    return len;
  }
private:
  int db_begin_time_;
  int db_end_time_;
  int insert_cnt_;
  int max_insert_cnt_;
  int update_cnt_;
  int max_update_cnt_;
  int select_cnt_;
  int max_select_cnt_;
  int delete_cnt_;
  int max_delete_cnt_;
  int last_update_db_proxy_payload_time_;
};
svc_monitor::svc_monitor() : impl_(new svc_monitor_impl())
{ }
void svc_monitor::run(const int now)
{ this->impl_->run(now); }
void svc_monitor::handle_get_db_proxy_payload_result(const char *msg, const int len)
{ this->impl_->handle_get_db_proxy_payload_result(msg, len); }
int svc_monitor::do_build_online(char *bf, const int bf_len)
{ return this->impl_->do_build_online(bf, bf_len); }
int svc_monitor::do_build_scene_online(char *bf, const int bf_len)
{ return this->impl_->do_build_scene_online(bf, bf_len); }
int svc_monitor::do_build_db_proxy_payload(char *bf, const int bf_len)
{ return this->impl_->do_build_db_proxy_payload(bf, bf_len); }
