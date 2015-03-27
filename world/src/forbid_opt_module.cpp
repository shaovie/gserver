#include "forbid_opt_module.h"
#include "all_char_info.h"
#include "forbid_opt.h"
#include "time_util.h"
#include "db_proxy.h"
#include "istream.h"
#include "message.h"
#include "client.h"
#include "util.h"
#include "sys.h"

// Lib header
#include <map>
#include <string>

typedef std::map<std::string, forbid_opt *> forbid_ac_login_map_t;
typedef std::map<std::string, forbid_opt *>::iterator forbid_ac_login_map_itor;
static forbid_ac_login_map_t s_forbid_ac_login_map;

typedef std::map<std::string, forbid_opt *> forbid_ip_login_map_t;
typedef std::map<std::string, forbid_opt *>::iterator forbid_ip_login_map_itor;
static forbid_ip_login_map_t s_forbid_ip_login_map;

typedef std::map<int, forbid_opt *> forbid_char_talk_map_t;
typedef std::map<int, forbid_opt *>::iterator forbid_char_talk_map_itor;
static forbid_char_talk_map_t s_forbid_char_talk_map;

typedef std::map<int, forbid_opt *> forbid_char_login_map_t;
typedef std::map<int, forbid_opt *>::iterator forbid_char_login_map_itor;
static forbid_char_login_map_t s_forbid_char_login_map;

void forbid_opt_module::db_insert_forbit_opt(const forbid_opt *fo)
{
  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr fo_so((char *)fo, sizeof(forbid_opt));
  os << 0 << fo_so;
  db_proxy::instance()->send_request(0, REQ_INSERT_FORBID_OPT, &os);
}
int forbid_opt_module::do_insert_forbid_opt(const forbid_opt &fo, const bool preload)
{
  if (fo.opt_ == forbid_opt::FORBID_AC_LOGIN)
    return forbid_opt_module::do_insert_forbid_ac_login(fo, preload);
  else if (fo.opt_ == forbid_opt::FORBID_IP_LOGIN)
    return forbid_opt_module::do_insert_forbid_ip_login(fo, preload);
  else if (fo.opt_ == forbid_opt::FORBID_CHAR_LOGIN)
    return forbid_opt_module::do_insert_forbid_char_login(fo, preload);
  else if (fo.opt_ == forbid_opt::FORBID_CHAR_TALK)
    return forbid_opt_module::do_insert_forbid_char_talk(fo, preload);
  return -1;
}
// char
bool forbid_opt_module::can_login_for_char(const int char_id)
{
  forbid_char_login_map_itor itor = s_forbid_char_login_map.find(char_id);
  if (itor == s_forbid_char_login_map.end()) return true;

  if (time_util::now > itor->second->begin_time_
      && time_util::now < itor->second->end_time_)
    return false;
  return true;
}
int forbid_opt_module::do_insert_forbid_char_login(const forbid_opt &fo, const bool preload)
{
  int char_id = ::atoi(fo.target_);
  if (all_char_info::instance()->get_char_brief_info(char_id) == NULL)
    return -1;
  forbid_char_login_map_itor itor = s_forbid_char_login_map.find(char_id);
  if (itor == s_forbid_char_login_map.end())
  {
    forbid_opt *p = new forbid_opt();
    *p = fo;
    s_forbid_char_login_map.insert(std::make_pair(char_id, p));
    if (!preload)
    {
      p->id_ = sys::assign_forbid_id();
      forbid_opt_module::db_insert_forbit_opt(p);
    }
  }else
  {
    itor->second->begin_time_ = fo.begin_time_;
    itor->second->end_time_   = fo.end_time_;
    if (!preload)
      forbid_opt_module::db_insert_forbit_opt(itor->second);
  }
  return 0;
}
// ip
bool forbid_opt_module::can_login_for_ip(const char *ip)
{
  forbid_ip_login_map_itor itor = s_forbid_ip_login_map.find(ip);
  if (itor == s_forbid_ip_login_map.end()) return true;

  if (time_util::now > itor->second->begin_time_
      && time_util::now < itor->second->end_time_)
    return false;
  return true;
}
int forbid_opt_module::do_insert_forbid_ip_login(const forbid_opt &fo, const bool preload)
{
  forbid_ip_login_map_itor itor = s_forbid_ip_login_map.find(fo.target_);
  if (itor == s_forbid_ip_login_map.end())
  {
    forbid_opt *p = new forbid_opt();
    *p = fo;
    s_forbid_ip_login_map.insert(std::make_pair(fo.target_, p));
    if (!preload)
    {
      p->id_ = sys::assign_forbid_id();
      forbid_opt_module::db_insert_forbit_opt(p);
    }
  }else
  {
    itor->second->begin_time_ = fo.begin_time_;
    itor->second->end_time_   = fo.end_time_;
    if (!preload)
      forbid_opt_module::db_insert_forbit_opt(itor->second);
  }
  return 0;
}
// account
bool forbid_opt_module::can_login_for_account(const char *account)
{
  forbid_ac_login_map_itor itor = s_forbid_ac_login_map.find(account);
  if (itor == s_forbid_ac_login_map.end()) return true;

  if (time_util::now > itor->second->begin_time_
      && time_util::now < itor->second->end_time_)
    return false;
  return true;
}
int forbid_opt_module::do_insert_forbid_ac_login(const forbid_opt &fo, const bool preload)
{
  forbid_ac_login_map_itor itor = s_forbid_ac_login_map.find(fo.target_);
  if (itor == s_forbid_ac_login_map.end())
  {
    forbid_opt *p = new forbid_opt();
    *p = fo;
    s_forbid_ac_login_map.insert(std::make_pair(fo.target_, p));
    if (!preload)
    {
      p->id_ = sys::assign_forbid_id();
      forbid_opt_module::db_insert_forbit_opt(p);
    }
  }else
  {
    itor->second->begin_time_ = fo.begin_time_;
    itor->second->end_time_   = fo.end_time_;
    forbid_opt_module::db_insert_forbit_opt(itor->second);
  }
  return 0;
}
// char talk
bool forbid_opt_module::can_talk(const int char_id)
{
  forbid_char_talk_map_itor itor = s_forbid_char_talk_map.find(char_id);
  if (itor == s_forbid_char_talk_map.end()) return true;

  if (time_util::now > itor->second->begin_time_
      && time_util::now < itor->second->end_time_)
    return false;
  return true;
}
int forbid_opt_module::do_insert_forbid_char_talk(const forbid_opt &fo, const bool preload)
{
  int char_id = ::atoi(fo.target_);
  if (all_char_info::instance()->get_char_brief_info(char_id) == NULL)
    return -1;
  forbid_char_talk_map_itor itor = s_forbid_char_talk_map.find(char_id);
  if (itor == s_forbid_char_talk_map.end())
  {
    forbid_opt *p = new forbid_opt();
    *p = fo;
    s_forbid_char_talk_map.insert(std::make_pair(char_id, p));
    if (!preload)
    {
      p->id_ = sys::assign_forbid_id();
      forbid_opt_module::db_insert_forbit_opt(p);
    }
  }else
  {
    itor->second->begin_time_ = fo.begin_time_;
    itor->second->end_time_   = fo.end_time_;
    if (!preload)
      forbid_opt_module::db_insert_forbit_opt(itor->second);
  }
  return 0;
}
