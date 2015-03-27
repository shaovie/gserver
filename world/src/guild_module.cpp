#include "guild_module.h"
#include "guild_info.h"
#include "guild_member_info.h"
#include "guild_apply_info.h"
#include "service_info.h"
#include "db_proxy.h"
#include "istream.h"
#include "global_macros.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "client.h"
#include "sys.h"
#include "time_util.h"
#include "player_obj.h"
#include "all_char_info.h"
#include "guild_config.h"
#include "player_mgr.h"
#include "global_param_cfg.h"
#include "behavior_id.h"
#include "chat_module.h"
#include "vip_module.h"
#include "item_obj.h"
#include "guild_zhu_di_scp.h"
#include "scp_mgr.h"
#include "clsid.h"
#include "guild_skill_info.h"
#include "package_module.h"
#include "cheng_jiu_module.h"
#include "huo_yue_du_module.h"
#include "task_module.h"
#include "ghz_module.h"
#include "util.h"
#include "notice_module.h"
#include "mail_module.h"
#include "mail_config.h"
#include "mail_info.h"
#include "mobai_module.h"
#include "scp_module.h"
#include "scene_config.h"

// Lib header
#include <map>
#include <vector>
#include <algorithm>

#define GUILD_DB_SID        0
#define GUILD_SORT_CD       5
#define GUILD_INIT_LVL      1
#define GUILD_SUMMON_RADIUS 12

static ilog_obj *s_log = sys_log::instance()->get_ilog("guild");
static ilog_obj *e_log = err_log::instance()->get_ilog("guild");

static int s_last_sort_guild_time = 0;
static int s_ghz_winner = 0;

enum
{
  GUILD_MEM_JOIN = 1,
  GUILD_MEM_EXIT = 0,
};
enum
{
  GUILD_APPLY_NOT = 0,
  GUILD_APPLY_HAD = 1, // 已申请过
};
enum
{
  GUILD_REPLY_DISAG = 0,
  GUILD_REPLY_AGREE = 1, // 同意
};
enum building_type
{
  GUILD_BLD_XXX          = 0,
  GUILD_BLD_DA_TING      = 1,
  GUILD_BLD_ZHAN_QI      = 2,
  GUILD_BLD_TU_LONG_DIAN = 3,
  GUILD_BLD_JU_BAO_DAI   = 4,
  GUILD_BLD_SCP          = 5,
  GUILD_BLD_END,
};
enum guild_scp_doing
{
  GSD_CANT_OPEN = 0,
  GSD_CAN_OPEN  = 1,
  GSD_ON_OPEN   = 2,
};

typedef std::map<int/*char_id*/, guild_member_info *> guild_member_map_t;
typedef std::map<int/*char_id*/, guild_member_info *>::iterator guild_member_map_iter;
typedef ilist<guild_apply_info *> guild_apply_list_t;
typedef ilist_node<guild_apply_info *> *guild_apply_list_iter;

class all_guild_info
{
public:
  all_guild_info() :
    zhan_li_(0),
    scp_id_(0),
    base_info_(NULL)
  { }
  ~all_guild_info()
  {
    delete this->base_info_;
    while (!this->apply_list_.empty())
      delete this->apply_list_.pop_front();
    for (guild_member_map_iter itor = this->member_map_.begin();
         itor != this->member_map_.end();
         ++itor)
      delete itor->second;
  }
public:
  static all_guild_info *guild_map_find(const int guild_id);
  static bool guild_map_find(const char *guild_name);
  static int  guild_sort_map_rank(const int guild_id);
  static void guild_sort_map_erase_and_free(const int guild_id);
  static int  guild_map_find_apply(const int char_id);
  static int  get_guild_id(const int char_id);
public:
  void erase_and_free_apply(const int char_id);
  bool find_apply(const int char_id);
  int  get_pos_mem_amt(const char pos);
  void cache_zhan_li();
  void do_broadcast_guild_info();
public:
  int zhan_li_;
  int scp_id_;
  guild_info *base_info_;
  guild_apply_list_t apply_list_;
  guild_member_map_t member_map_;
};
typedef std::map<int/*guild id*/, all_guild_info *> guild_map_t;
typedef std::map<int/*guild id*/, all_guild_info *>::iterator guild_map_iter;
static  guild_map_t s_guild_map;

typedef std::vector<all_guild_info *> guild_sort_t; // sort by 战斗力
typedef std::vector<all_guild_info *>::iterator guild_sort_iter;
static  guild_sort_t s_guild_sort;

all_guild_info *all_guild_info::guild_map_find(const int guild_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor != s_guild_map.end())
    return itor->second;
  return NULL;
}
bool all_guild_info::guild_map_find(const char *guild_name)
{
  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    if (::strcasecmp(itor->second->base_info_->name_, guild_name) == 0)
      return true;
  }
  return false;
}
int all_guild_info::guild_sort_map_rank(const int guild_id)
{
  int rank = 1;
  for (guild_sort_iter itor = s_guild_sort.begin();
       itor != s_guild_sort.end();
       ++itor, ++rank)
  {
    if ((*itor)->base_info_->guild_id_ == guild_id)
      return rank;
  }
  return 0;
}
void all_guild_info::guild_sort_map_erase_and_free(const int guild_id)
{
  for (guild_sort_iter itor = s_guild_sort.begin();
       itor != s_guild_sort.end();
       ++itor)
  {
    if ((*itor)->base_info_->guild_id_ == guild_id)
    {
      delete *itor;
      s_guild_sort.erase(itor);
      return ;
    }
  }
}
int all_guild_info::guild_map_find_apply(const int char_id)
{
  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    if (itor->second->find_apply(char_id))
      return itor->second->base_info_->guild_id_;
  }
  return 0;
}
int all_guild_info::get_guild_id(const int char_id)
{
  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    all_guild_info *agi = itor->second;
    if (agi->member_map_.find(char_id)
        != agi->member_map_.end())
      return agi->base_info_->guild_id_;
  }
  return 0;
}
bool all_guild_info::find_apply(const int char_id)
{
  for (guild_apply_list_iter itor = this->apply_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->char_id_ == char_id)
      return true;
  }
  return false;
}
void all_guild_info::erase_and_free_apply(const int char_id)
{
  for (guild_apply_list_iter itor = this->apply_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    guild_apply_info *info = itor->value_;
    if (info->char_id_ == char_id)
    {
      this->apply_list_.remove(info);
      delete info;
    }
  }
}
int all_guild_info::get_pos_mem_amt(const char pos)
{
  int amt = 0;
  for (guild_member_map_iter itor = this->member_map_.begin();
       itor != this->member_map_.end();
       ++itor)
  {
    if (itor->second->position_ == pos)
      ++amt;
  }
  return amt;
}
void all_guild_info::cache_zhan_li()
{
  int64_t total = 0;
  for (guild_member_map_iter itor = this->member_map_.begin();
       itor != this->member_map_.end();
       ++itor)
  {
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(itor->second->char_id_);
    if (cbi == NULL) continue;
    total += cbi->zhan_li_;
  }
  this->zhan_li_ = total < MAX_VALID_INT ? total : MAX_VALID_INT;
}
void all_guild_info::do_broadcast_guild_info()
{
  for (guild_member_map_iter itor = this->member_map_.begin();
       itor != this->member_map_.end();
       ++itor)
  {
    player_obj *player = player_mgr::instance()->find(itor->second->char_id_);
    if (player == NULL) continue;
    player->broadcast_guild_info();
  }
}
void guild_module::do_insert_guild(guild_info *info)
{
  all_guild_info *agi = new all_guild_info();
  agi->base_info_ = info;
  if (info->is_ghz_winner_)
    s_ghz_winner = info->guild_id_;
  s_guild_map.insert(std::make_pair(info->guild_id_, agi));
  s_guild_sort.push_back(agi);
  guild_module::on_new_guild(info->guild_id_);
}
void guild_module::do_insert_guild_member(guild_member_info *info)
{
  guild_map_iter itor = s_guild_map.find(info->guild_id_);
  if (itor == s_guild_map.end())
  {
    delete info;
    return ;
  }
  itor->second->member_map_.insert(std::make_pair(info->char_id_, info));
}
void guild_module::do_insert_guild_apply(guild_apply_info *info)
{
  guild_map_iter itor = s_guild_map.find(info->guild_id_);
  if (itor == s_guild_map.end())
  {
    delete info;
    return ;
  }
  itor->second->apply_list_.push_back(info);
}
int guild_module::do_create_guild(const int char_id, const char *name)
{
  const int guild_id = sys::assign_guild_id();
  all_guild_info *agi = new all_guild_info();
  s_guild_map.insert(std::make_pair(guild_id, agi));
  s_guild_sort.push_back(agi);

  agi->base_info_ = new guild_info();
  agi->base_info_->guild_id_    = guild_id;
  agi->base_info_->chairman_id_ = char_id;
  agi->base_info_->lvl_         = GUILD_INIT_LVL;
  agi->base_info_->zq_lvl_      = 0;
  agi->base_info_->tld_lvl_     = 0;
  agi->base_info_->jbd_lvl_     = 0;
  agi->base_info_->scp_lvl_     = 0;
  agi->base_info_->c_time_      = time_util::now;
  ::strncpy(agi->base_info_->name_, name, sizeof(agi->base_info_->name_) - 1);

  out_stream os(client::send_buf, client::send_buf_len);
  os << GUILD_DB_SID << agi->base_info_;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_INSERT_GUILD, &os);

  guild_member_info *gmi = new guild_member_info();
  agi->member_map_.insert(std::make_pair(char_id, gmi));
  gmi->char_id_ = char_id;
  gmi->guild_id_ = guild_id;
  gmi->position_ = guild_member_info::POS_HZ;

  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << stream_ostr((const char *)gmi, sizeof(*gmi));
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_INSERT_GUILD_MEMBER, &mos);

  guild_module::do_notify_guild_pos_to_clt(char_id, guild_member_info::POS_HZ);
  guild_module::on_join_guild(char_id, guild_id, true);
  guild_module::on_new_guild(guild_id);
  return guild_id;
}
void guild_module::do_destory_guild(const int guild_id)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << GUILD_DB_SID << guild_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD, &os);

  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << guild_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD_MEMBERS, &mos);

  out_stream aos(client::send_buf, client::send_buf_len);
  aos << GUILD_DB_SID << guild_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD_APPLYS, &aos);

  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  for (guild_member_map_iter mitor = itor->second->member_map_.begin();
       mitor != itor->second->member_map_.end();
       ++mitor)
    guild_module::on_exit_guild(mitor->second->char_id_, guild_id);

  guild_module::on_delete_guild(guild_id);

  all_guild_info::guild_sort_map_erase_and_free(guild_id);
  s_guild_map.erase(guild_id);
}
void guild_module::do_create_guild_member(const int guild_id, const int char_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  if (itor->second->member_map_.find(char_id)
      != itor->second->member_map_.end())
    return ;

  guild_member_info *gmi = new guild_member_info();
  itor->second->member_map_.insert(std::make_pair(char_id, gmi));
  gmi->char_id_ = char_id;
  gmi->guild_id_ = guild_id;
  gmi->position_ = guild_member_info::POS_HY;

  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << stream_ostr((const char *)gmi, sizeof(*gmi));
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_INSERT_GUILD_MEMBER, &mos);

  guild_module::on_join_guild(char_id, guild_id, false);
}
void guild_module::do_destory_guild_member(const int guild_id, const int char_id)
{
  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << char_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD_MEMBER, &mos);

  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  guild_module::on_exit_guild(char_id, guild_id);

  guild_member_map_iter mitor = itor->second->member_map_.find(char_id);
  if (mitor == itor->second->member_map_.end()) return ;
  delete mitor->second;
  itor->second->member_map_.erase(char_id);
}
void guild_module::do_create_guild_apply(const int guild_id, const int char_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  guild_apply_info *gai = new guild_apply_info();
  itor->second->apply_list_.push_back(gai);
  gai->char_id_ = char_id;
  gai->guild_id_ = guild_id;
  gai->apply_time_ = time_util::now;

  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << stream_ostr((const char *)gai, sizeof(*gai));
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_INSERT_GUILD_APPLY, &mos);
}
void guild_module::do_destory_guild_apply(const int guild_id, const int char_id)
{
  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << guild_id << char_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD_APPLY, &mos);

  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;
  itor->second->erase_and_free_apply(char_id);
}
void guild_module::do_destory_guild_apply_by_guild(const int guild_id)
{
  out_stream mos(client::send_buf, client::send_buf_len);
  mos << GUILD_DB_SID << guild_id;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_DELETE_GUILD_APPLYS, &mos);

  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  while (!itor->second->apply_list_.empty())
    delete itor->second->apply_list_.pop_front();
}
void guild_module::on_char_login(player_obj *player)
{
  int guild_id = all_guild_info::get_guild_id(player->id());
  if (guild_id == 0) return ;
  player->guild_id(guild_id);
}
void guild_module::on_enter_game(player_obj *player)
{
  if (player->guild_id() == 0) return ;
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end()) return ;

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end()) return ;
  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_agree_join(pos)) return ;

  const int amt = itor->second->apply_list_.size();
  if (amt > 0)
    guild_module::do_notify_apply_to_clt(player, amt);
}
void guild_module::on_join_guild(const int char_id, const int guild_id, const bool create_guild)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;
  player->guild_id(guild_id);
  player->broadcast_guild_info();
  chat_module::on_join_guild(char_id, guild_id);
  task_module::on_join_guild(player);

  if (!create_guild)
    guild_module::do_broadcast_mem_join_or_exit_to_guild(guild_id,
                                                         player->id(),
                                                         player->name(),
                                                         GUILD_MEM_JOIN);
}
void guild_module::do_broadcast_mem_join_or_exit_to_guild(const int guild_id,
                                                          const int char_id,
                                                          const char *name,
                                                          const char join_or_exit)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << join_or_exit << char_id << stream_ostr(name, ::strlen(name));
  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(guild_id, NTF_GUILD_MEM_JOIN_OR_EXIT, 0, &mb);
}
void guild_module::on_exit_guild(const int char_id, const int guild_id)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;
  player->guild_id(0);
  player->broadcast_guild_info();
  chat_module::on_exit_guild(char_id, guild_id);
  task_module::on_exit_guild(player);
  guild_zhu_di_scp::on_exit_guild(player, guild_id);
  guild_module::do_broadcast_mem_join_or_exit_to_guild(guild_id,
                                                       player->id(),
                                                       player->name(),
                                                       GUILD_MEM_EXIT);
}
void guild_module::on_ghz_over(const int win_guild_id)
{
  if (win_guild_id == 0) return ;
  all_guild_info *agi = all_guild_info::guild_map_find(win_guild_id);
  if (agi == NULL
      || agi->base_info_->is_ghz_winner_)
    return ;

  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    if (itor->second->base_info_->is_ghz_winner_
        && itor->second->base_info_->guild_id_ != win_guild_id)
    {
      itor->second->base_info_->is_ghz_winner_ = 0;
      s_ghz_winner = 0;
      itor->second->do_broadcast_guild_info();
      guild_module::do_update_guild_2_db(itor->second->base_info_);
    }
  }
  const int old_win_guild_id = s_ghz_winner;
  agi->base_info_->is_ghz_winner_ = 1;
  s_ghz_winner = agi->base_info_->guild_id_;
  guild_module::do_update_guild_2_db(agi->base_info_);
  agi->do_broadcast_guild_info();

  mobai_module::on_ghz_result(old_win_guild_id, win_guild_id);
}
static bool guild_sort_rule(all_guild_info *frs, all_guild_info *scn)
{ return frs->zhan_li_ > scn->zhan_li_; }
void guild_module::do_sort_guild()
{
  if (time_util::now < s_last_sort_guild_time + GUILD_SORT_CD)
    return ;
  s_last_sort_guild_time = time_util::now;

  // cache
  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
    itor->second->cache_zhan_li();

  std::sort(s_guild_sort.begin(), s_guild_sort.end(), guild_sort_rule);
}
void guild_module::do_fetch_char_info(char_brief_info *info, out_stream &os)
{
  os << info->char_id_
    << stream_ostr(info->name_, ::strlen(info->name_))
    << info->career_
    << info->lvl_;
}
void guild_module::do_fetch_chairman_info(char_brief_info *info, out_stream &os)
{
  os << info->char_id_
    << stream_ostr(info->name_, ::strlen(info->name_));
}
char* guild_module::get_guild_name(const int guild_id)
{
  if (guild_id <= 0) return NULL;
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor != s_guild_map.end())
    return itor->second->base_info_->name_;
  return NULL;
}
int guild_module::get_guild_zhan_li(const int guild_id)
{
  if (guild_id <= 0) return 0;
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor != s_guild_map.end())
    return itor->second->zhan_li_;
  return 0;
}
int guild_module::get_guild_id(const int char_id)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player != NULL)
    return player->guild_id();
  return all_guild_info::get_guild_id(char_id);
}
int guild_module::get_ghz_winner()
{ return s_ghz_winner; }
int guild_module::get_ghz_winner_chairman()
{
  if (s_ghz_winner <= 0) return 0;
  guild_map_iter itor = s_guild_map.find(s_ghz_winner);
  if (itor != s_guild_map.end())
    return itor->second->base_info_->chairman_id_;
  return 0;
}
int guild_module::get_chairman(const int guild_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor != s_guild_map.end())
    return itor->second->base_info_->chairman_id_;
  return 0;
}
void guild_module::get_member_list(const int guild_id, ilist<int> &mem_list)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor != s_guild_map.end())
  {
    for (guild_member_map_iter i = itor->second->member_map_.begin();
         i != itor->second->member_map_.end();
         ++i)
      mem_list.push_back(i->first);
  }
}
void guild_module::get_guild_list_before_rank(ilist<int> &id_list, const int n)
{
  guild_module::do_sort_guild();
  guild_sort_iter itor = s_guild_sort.begin();
  for (int i = 0; i < n && itor != s_guild_sort.end(); ++i, ++itor)
    id_list.push_back((*itor)->base_info_->guild_id_);
}
void guild_module::on_new_day()
{
  guild_module::do_check_chairman_turn();
  guild_module::do_check_no_mem_login_dissmis_guild();
}
void guild_module::do_check_chairman_turn()
{
  if (time_util::now - sys::svc_info->open_time
      < global_param_cfg::guild_chairman_turn_days * 24 * 3600)
    return ;

  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    const int cm_id = itor->second->base_info_->chairman_id_;
    char_brief_info *cm = all_char_info::instance()->get_char_brief_info(cm_id);
    if ((cm != NULL
         && (time_util::diff_days(cm->out_time_)
             < global_param_cfg::guild_chairman_turn_days))
        || player_mgr::instance()->find(cm_id) != NULL)
      continue;

    guild_member_info *gmi = NULL;
    guild_member_info *new_gmi = NULL;
    for (guild_member_map_iter mitor = itor->second->member_map_.begin();
         mitor != itor->second->member_map_.end();
         ++mitor)
    {
      if (mitor->second->char_id_ == cm_id)
      {
        gmi = mitor->second;
        continue;
      }
      char_brief_info *turner = all_char_info::instance()->get_char_brief_info(mitor->second->char_id_);
      if (turner == NULL
          || (((time_util::diff_days(turner->out_time_)
                > global_param_cfg::guild_chairman_turn_days))
              && player_mgr::instance()->find(cm_id) == NULL))
        continue;

      if (new_gmi == NULL
          || new_gmi->total_contrib_ < mitor->second->total_contrib_)
        new_gmi = mitor->second;
    }
    if (gmi == NULL
        || new_gmi == NULL)
      continue;

    gmi->position_ = guild_member_info::POS_HY;
    guild_module::do_update_guild_member_2_db(gmi);

    new_gmi->position_ = guild_member_info::POS_HZ;
    guild_module::do_update_guild_member_2_db(new_gmi);

    itor->second->base_info_->chairman_id_ = new_gmi->char_id_;
    guild_module::do_update_guild_2_db(itor->second->base_info_);
    guild_module::on_guild_demise(itor->second->base_info_->guild_id_);
    guild_module::do_notify_guild_pos_to_clt(gmi->char_id_, guild_member_info::POS_HY);
    guild_module::do_notify_guild_pos_to_clt(new_gmi->char_id_, guild_member_info::POS_HZ);
  }
}
void guild_module::on_guild_demise(const int guild_id)
{  mobai_module::on_guild_demise(guild_id); }
void guild_module::do_check_no_mem_login_dissmis_guild()
{
  if (time_util::now - sys::svc_info->open_time
      < global_param_cfg::guild_auto_dismiss_days * 24 * 3600)
    return ;

  for (guild_map_iter itor = s_guild_map.begin();
       itor != s_guild_map.end();
       ++itor)
  {
    bool is_near_login = false;
    for (guild_member_map_iter mitor = itor->second->member_map_.begin();
         mitor != itor->second->member_map_.end();
         ++mitor)
    {
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(mitor->second->char_id_);
      if (cbi != NULL
          && (((time_util::diff_days(cbi->out_time_)
                < global_param_cfg::guild_auto_dismiss_days))
              || player_mgr::instance()->find(mitor->second->char_id_) != NULL))
      {
        is_near_login = true;
        break;
      }
    }
    if (!is_near_login)
      guild_module::do_destory_guild(itor->second->base_info_->guild_id_);
  }
}
void guild_module::do_fetch_char_guild_info(const int guild_id, out_stream &os)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end())
    os << 0 << stream_ostr("", 0) << (char)0;
  else
  {
    os << guild_id
      << stream_ostr(itor->second->base_info_->name_, ::strlen(itor->second->base_info_->name_))
      << (char)(guild_id == s_ghz_winner ? 1 : 0);
  }
}
char guild_module::get_char_guild_pos(const int guild_id, const int char_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return 0;
  guild_member_map_iter mitor = itor->second->member_map_.find(char_id);
  if (mitor == itor->second->member_map_.end()) return 0;
  return mitor->second->position_;
}
void player_obj::broadcast_guild_info()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id();

  out_stream os(mb.wr_ptr(), mb.space());
  guild_module::do_fetch_char_guild_info(this->guild_id(), os);
  mb.wr_ptr(os.length());

  ph->set(0, NTF_BROADCAST_GUILD_INFO, 0, mb.length());
  this->do_broadcast(&mb, true);
}
void guild_module::broadcast_to_guild(const int guild_id,
                                      const int msg_id,
                                      const int exclude_char_id,
                                      mblock *mb)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;
  for (guild_member_map_iter mitor = itor->second->member_map_.begin();
       mitor != itor->second->member_map_.end();
       ++mitor)
  {
    if (mitor->second->char_id_ == exclude_char_id) continue;
    player_obj *member = player_mgr::instance()->find(mitor->second->char_id_);
    if (member == NULL) continue;
    member->do_delivery(msg_id, mb);
  }
}
void guild_module::do_notify_guild_pos_to_clt(const int char_id, const char pos)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << pos;
  player->do_delivery(NTF_GUILD_MEM_POS, &mb);
}
void guild_module::on_new_guild(const int guild_id)
{ guild_zhu_di_scp::on_new_guild(guild_id); }
void guild_module::on_delete_guild(const int guild_id)
{
  guild_zhu_di_scp::on_remove_guild(guild_id);
  mobai_module::on_guild_dismiss(guild_id);
}
void guild_module::on_got_contrib(player_obj *player, const int cur_value)
{
  cheng_jiu_module::on_guild_contrib(player, cur_value);
}
void guild_module::do_update_guild_2_db(const guild_info *info)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << GUILD_DB_SID << info;
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_UPDATE_GUILD, &os);
}
void guild_module::do_update_guild_member_2_db(const guild_member_info *info)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << GUILD_DB_SID << stream_ostr((const char *)info, sizeof(*info));
  db_proxy::instance()->send_request(GUILD_DB_SID, REQ_UPDATE_GUILD_MEMBER, &os);
}
int guild_module::handle_db_get_guild_skill_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(guild_skill_info) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    guild_skill_info *info = (guild_skill_info *)bf;
    guild_skill_info *new_info = new guild_skill_info();
    ::memcpy(new_info, info, sizeof(guild_skill_info));
    player->guild_skill_list_.push_back(new_info);
  }
  return 0;
}
void guild_module::destroy(player_obj *player)
{
  while (!player->guild_skill_list_.empty())
    delete player->guild_skill_list_.pop_front();
}
guild_skill_info *guild_module::to_find_guild_skill(player_obj *player, const int skill_cid)
{
  for (ilist_node<guild_skill_info *> *itor = player->guild_skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == skill_cid)
      return itor->value_;
  }
  return NULL;
}
void guild_module::do_build_guild_skill_list(player_obj *player, out_stream &os)
{
  short *skill_amt = (short *)os.wr_ptr();
  os << (short)0;

  for (ilist_node<guild_skill_info *> *itor = player->guild_skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    os << itor->value_->cid_ << itor->value_->lvl_;
    ++(*skill_amt);
  }
}
void guild_module::do_promote_guild_skill(player_obj *player,
                                          const int skill_cid,
                                          const short skill_lvl)
{
  guild_skill_info *skill = guild_module::to_find_guild_skill(player, skill_cid);
  if (skill == NULL)
  {
    skill = new guild_skill_info();
    skill->char_id_ = player->id();
    skill->cid_ = skill_cid;
    skill->lvl_ = skill_lvl;
    player->guild_skill_list_.push_back(skill);

    stream_ostr so((const char *)skill, sizeof(guild_skill_info));
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << so;
    db_proxy::instance()->send_request(player->id(), REQ_INSERT_GUILD_SKILL, &os);
  }else
  {
    skill->lvl_ = skill_lvl;

    stream_ostr so((const char *)skill, sizeof(guild_skill_info));
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << so;
    db_proxy::instance()->send_request(player->id(), REQ_UPDATE_GUILD_SKILL, &os);
  }
  player->do_calc_attr_affected_by_guild_skill();
  player->on_attr_update(ZHAN_LI_GSKILL, skill_cid);
}
void guild_module::do_got_contrib(const int guild_id, const int char_id, const int value)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  guild_member_map_iter mitor = itor->second->member_map_.find(char_id);
  if (mitor == itor->second->member_map_.end()) return ;

  if (mitor->second->total_contrib_ == MAX_VALID_INT)
    return ;

  int add = value;
  int left = MAX_VALID_INT - mitor->second->total_contrib_;
  add > left ? add = left : 0;

  mitor->second->contrib_ += add;
  mitor->second->total_contrib_ += add;
  guild_module::do_update_guild_member_2_db(mitor->second);

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;
  guild_module::do_notify_contrib_to_clt(player, mitor->second);
  guild_module::on_got_contrib(player, mitor->second->total_contrib_);
}
void guild_module::do_notify_contrib_to_clt(player_obj *player, const guild_member_info *info)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << info->contrib_
    << info->total_contrib_;
  player->send_request(NTF_GUILD_MEM_CONTRIB, &os);
}
void guild_module::do_got_resource(const int guild_id, const int value)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  if (itor->second->base_info_->guild_resource_ >= MAX_VALID_INT)
    return ;

  int add = value;
  int left = MAX_VALID_INT - itor->second->base_info_->guild_resource_;
  add > left ? add = left : 0;

  itor->second->base_info_->guild_resource_ += add;
  guild_module::do_update_guild_2_db(itor->second->base_info_);
}
void guild_module::do_notify_resource_to_clt(player_obj *player, const guild_info *info)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << info->guild_resource_;
  player->send_request(NTF_GUILD_RESOURCE, &os);
}
void guild_module::do_broadcast_apply_to_clt(const int guild_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;

  for (guild_member_map_iter mitor = itor->second->member_map_.begin();
       mitor != itor->second->member_map_.end();
       ++mitor)
  {
    guild_member_info *info = mitor->second;
    if (!guild_pos_config::instance()->had_right_to_agree_join(info->position_)) continue;
    player_obj *player = player_mgr::instance()->find(info->char_id_);
    if (player == NULL) continue;
    guild_module::do_notify_apply_to_clt(player, itor->second->apply_list_.size());
  }
}
void guild_module::do_notify_apply_to_clt(player_obj *player, const int amt)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << char(amt > 0 ? 1 : 0);
  player->send_request(NTF_GUILD_HAD_APPLY, &os);
}

int guild_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = guild_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GUILD_CREATE,          clt_guild_create);
    SHORT_CODE(REQ_GUILD_LIST,            clt_guild_list);
    SHORT_CODE(REQ_GUILD_APPLY,           clt_guild_apply);
    SHORT_CODE(REQ_GUILD_APPLY_LIST,      clt_guild_apply_list);
    SHORT_CODE(REQ_GUILD_REPLY,           clt_guild_reply);
    SHORT_CODE(REQ_GUILD_REPLY_ALL,       clt_guild_reply_all);
    SHORT_CODE(REQ_GUILD_MEM_EXIT,        clt_guild_mem_exit);
    SHORT_CODE(REQ_GUILD_MEMBER_LIST,     clt_guild_member_list);
    SHORT_CODE(REQ_GUILD_APPLY_LIMIT_SET, clt_guild_apply_limit_set);
    SHORT_CODE(REQ_GUILD_MEM_POS_SET,     clt_guild_mem_pos_set);
    SHORT_CODE(REQ_GUILD_EXPEL_MEM,       clt_guild_expel_mem);
    SHORT_CODE(REQ_GUILD_INFO,            clt_guild_info);
    SHORT_CODE(REQ_GUILD_PURPOSE_SET,     clt_guild_purpose_set);
    SHORT_CODE(REQ_GUILD_JUAN_XIAN,       clt_guild_juan_xian);
    SHORT_CODE(REQ_GUILD_UP_BUILDING,     clt_guild_up_building);
    SHORT_CODE(REQ_GUILD_SUMMON_BOSS,     clt_guild_summon_boss);
    SHORT_CODE(REQ_GUILD_JU_BAO,          clt_guild_ju_bao);
    SHORT_CODE(REQ_GUILD_ENTER_ZHU_DI,    clt_guild_enter_zhu_di);
    SHORT_CODE(REQ_GUILD_SKILL_LIST,      clt_guild_skill_list);
    SHORT_CODE(REQ_GUILD_PROMOTE_SKILL,   clt_guild_promote_skill);
    SHORT_CODE(REQ_ENTER_GHZ,             clt_enter_ghz);
    SHORT_CODE(REQ_GHZ_ACTIVATE_SHOU_WEI, clt_ghz_activate_shou_wei);
    SHORT_CODE(REQ_GHZ_OBTAIN_FIGHTING_INFO, clt_ghz_obtain_fighting_info);
    SHORT_CODE(REQ_START_GUILD_SCP,       clt_start_guild_scp);
    SHORT_CODE(REQ_ENTER_GUILD_SCP,       clt_enter_guild_scp);
    SHORT_CODE(REQ_SUMMON_GUILD_MEM,      clt_summon_guild_mem);
    SHORT_CODE(REQ_TRANSFER_BY_GUILD_SUMMON, clt_transfer_by_guild_summon);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int guild_module::clt_guild_create(player_obj *player, const char *msg, const int len)
{
  char guild_name[MAX_NAME_LEN + 1];
  stream_istr name_si(guild_name, sizeof(guild_name));
  in_stream is(msg, len);
  is >> name_si;

  if (name_si.str_len() < 2
      || name_si.str_len() > MAX_NAME_LEN
      || !util::verify_name(name_si.str()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_INPUT_IS_ILLEGAL);

  if (player->lvl() < global_param_cfg::guild_create_min_char_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  if (player->guild_id() != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_HAD);

  if (all_guild_info::guild_map_find(guild_name))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_NAME_EXIST);

  int ret = player->is_money_enough(M_COIN, global_param_cfg::create_guild_cost);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_lose_money(global_param_cfg::create_guild_cost,
                        M_COIN,
                        MONEY_LOSE_GUILD_CREATE,
                        0, 0, 0);

  const int had_apply_guild_id = all_guild_info::guild_map_find_apply(player->id());
  if (had_apply_guild_id != 0)
    guild_module::do_destory_guild_apply(had_apply_guild_id, player->id());

  const int guild_id = guild_module::do_create_guild(player->id(), guild_name);
  out_stream os(client::send_buf, client::send_buf_len);
  os << guild_id
    << stream_ostr(guild_name, ::strlen(guild_name));
  return player->send_respond_ok(RES_GUILD_CREATE, &os);
}
int guild_module::clt_guild_list(player_obj *player, const char *msg, const int len)
{
  short page = 0;
  in_stream is(msg, len);
  is >> page;

  const short total_page = (int)s_guild_sort.size() / global_param_cfg::guild_one_page_amt + 1;
  if (page <= 0
      || page > total_page)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  guild_module::do_sort_guild();

  out_stream os(client::send_buf, client::send_buf_len);
  os << page << total_page;
  short *amt = (short *)os.wr_ptr();
  os << (short)0;
  for (int i = (page - 1) * global_param_cfg::guild_one_page_amt;
       i < page * global_param_cfg::guild_one_page_amt && i < (int)s_guild_sort.size();
       ++i)
  {
    all_guild_info *agi = s_guild_sort[i];
    char_brief_info *cm = all_char_info::instance()->get_char_brief_info(agi->base_info_->chairman_id_);
    if (cm == NULL) continue;

    os << agi->base_info_->guild_id_
      << stream_ostr(agi->base_info_->name_, ::strlen(agi->base_info_->name_));
    guild_module::do_fetch_chairman_info(cm, os);
    os << (short)agi->member_map_.size()
      << stream_ostr(agi->base_info_->purpose_, ::strlen(agi->base_info_->purpose_))
      << agi->base_info_->lvl_
      << agi->zhan_li_
      << (char)(agi->find_apply(player->id()) ? GUILD_APPLY_HAD : GUILD_APPLY_NOT)
      << agi->base_info_->apply_dj_limit_
      << agi->base_info_->apply_zl_limit_;
    ++(*amt);
  }
  return player->send_respond_ok(RES_GUILD_LIST, &os);
}
int guild_module::clt_guild_apply(player_obj *player, const char *msg, const int len)
{
  int tar_guild_id = 0;
  in_stream is(msg, len);
  is >> tar_guild_id;

  if (player->guild_id() != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_HAD);

  guild_map_iter itor = s_guild_map.find(tar_guild_id);
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_NO_EXIST);

  if (player->lvl() < itor->second->base_info_->apply_dj_limit_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  if (player->zhan_li() < itor->second->base_info_->apply_zl_limit_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_NO_ENOUGH_ZHAN_LI);

  const int guild_lvl = itor->second->base_info_->lvl_;
  if ((int)itor->second->member_map_.size() >= guild_lvl_config::instance()->mem_capacity(guild_lvl))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_MEM_FULL);

  if (itor->second->find_apply(player->id()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_HAD_APPLY);

  const int had_apply_guild_id = all_guild_info::guild_map_find_apply(player->id());
  if (had_apply_guild_id != 0)
    guild_module::do_destory_guild_apply(had_apply_guild_id, player->id());

  guild_module::do_create_guild_apply(tar_guild_id, player->id());
  guild_module::do_broadcast_apply_to_clt(had_apply_guild_id);
  guild_module::do_broadcast_apply_to_clt(tar_guild_id);
  return player->send_respond_ok(RES_GUILD_APPLY);
}
int guild_module::clt_guild_apply_list(player_obj *player, const char *, const int)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_agree_join(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  out_stream os(client::send_buf, client::send_buf_len);
  short *amt = (short *)os.wr_ptr();
  os << (short)0;
  for (guild_apply_list_iter aitor = itor->second->apply_list_.head();
       aitor != NULL;
       aitor = aitor->next_)
  {
    guild_apply_info *info = aitor->value_;
    char_brief_info *member = all_char_info::instance()->get_char_brief_info(info->char_id_);
    if (member == NULL) continue;
    guild_module::do_fetch_char_info(member, os);
    ++(*amt);
  }
  return player->send_respond_ok(RES_GUILD_APPLY_LIST, &os);
}
int guild_module::clt_guild_reply(player_obj *player, const char *msg, const int len)
{
  int char_id = 0;
  char reply = 0;
  in_stream is(msg, len);
  is >> char_id >> reply;

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_agree_join(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  if (!itor->second->find_apply(char_id))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_NO_HAD_APPLY);

  char_brief_info *apply = all_char_info::instance()->get_char_brief_info(char_id);
  if (apply == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_EXIST);

  guild_module::do_destory_guild_apply(player->guild_id(), char_id);

  const char guild_lvl = itor->second->base_info_->lvl_;
  const short mem_capacity = guild_lvl_config::instance()->mem_capacity(guild_lvl);
  const short left_capacity = mem_capacity - (int)itor->second->member_map_.size();

  const int target_guild_id = all_guild_info::get_guild_id(char_id);
  if (target_guild_id == 0
      && left_capacity > 0
      && reply == GUILD_REPLY_AGREE)
    guild_module::do_create_guild_member(player->guild_id(), char_id);

  guild_module::do_broadcast_apply_to_clt(player->guild_id());
  out_stream os(client::send_buf, client::send_buf_len);
  os << left_capacity;
  return player->send_request(RES_GUILD_REPLY, &os);
}
int guild_module::clt_guild_reply_all(player_obj *player, const char *msg, const int len)
{
  char reply = 0;
  in_stream is(msg, len);
  is >> reply;

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_agree_join(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  if (reply == GUILD_REPLY_AGREE)
  {
    const int guild_lvl = itor->second->base_info_->lvl_;
    const int mem_capacity = guild_lvl_config::instance()->mem_capacity(guild_lvl);
    for (guild_apply_list_iter aitor = itor->second->apply_list_.head();
         aitor != NULL && (int)itor->second->member_map_.size() < mem_capacity;
         aitor = aitor->next_)
    {
      guild_apply_info *info = aitor->value_;
      const int target_guild_id = all_guild_info::get_guild_id(info->char_id_);
      if (target_guild_id == 0)
        guild_module::do_create_guild_member(player->guild_id(), info->char_id_);
    }
  }
  guild_module::do_destory_guild_apply_by_guild(player->guild_id());
  guild_module::do_broadcast_apply_to_clt(player->guild_id());
  return 0;
}
int guild_module::clt_guild_mem_exit(player_obj *player, const char *, const int)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int guild_id = player->guild_id();
  guild_module::do_destory_guild_member(guild_id, player->id());

  if (itor->second->base_info_->chairman_id_ == player->id())
    guild_module::do_destory_guild(guild_id);
  return player->send_respond_ok(RES_GUILD_MEM_EXIT);
}
int guild_module::clt_guild_member_list(player_obj *player, const char *, const int)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  out_stream os(client::send_buf, client::send_buf_len);
  short *amt = (short *)os.wr_ptr();
  os << (short)0;
  for (guild_member_map_iter mitor = itor->second->member_map_.begin();
       mitor != itor->second->member_map_.end();
       ++mitor)
  {
    guild_member_info *info = mitor->second;
    char_brief_info *member = all_char_info::instance()->get_char_brief_info(info->char_id_);
    if (member == NULL) continue;

    guild_module::do_fetch_char_info(member, os);
    os << member->zhan_li_
      << member->vip_
      << info->position_
      << info->total_contrib_;
    if (player_mgr::instance()->find(member->char_id_) != NULL)
      os << (int)0;
    else
      os << time_util::now - member->out_time_;
    ++(*amt);
  }
  return player->send_respond_ok(RES_GUILD_MEMBER_LIST, &os);
}
int guild_module::clt_guild_apply_limit_set(player_obj *player, const char *msg, const int len)
{
  short lvl = 0;
  int zhan_li = 0;
  in_stream is(msg, len);
  is >> lvl >> zhan_li;

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_set_apply(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  itor->second->base_info_->apply_dj_limit_ = lvl;
  itor->second->base_info_->apply_zl_limit_ = zhan_li;
  guild_module::do_update_guild_2_db(itor->second->base_info_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << lvl << zhan_li;
  return player->send_respond_ok(RES_GUILD_APPLY_LIMIT_SET, &os);
}
int guild_module::clt_guild_mem_pos_set(player_obj *player, const char *msg, const int len)
{
  int char_id = 0;
  char tar_pos = 0;
  in_stream is(msg, len);
  is >> char_id >> tar_pos;

  if (tar_pos == guild_member_info::POS_HZ)
    return guild_module::do_demise_chairman(player, char_id);

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter cmitor = itor->second->member_map_.find(char_id);
  if (cmitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_EXIST);

  const int pos = mitor->second->position_;
  const int cmp_pos = tar_pos > mitor->second->position_ ? tar_pos : mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_set_pos(cmp_pos, pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  const int amt = itor->second->get_pos_mem_amt(tar_pos);
  if (guild_pos_config::instance()->pos_full(tar_pos, amt))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_MEM_FULL);

  cmitor->second->position_ = tar_pos;
  guild_module::do_update_guild_member_2_db(cmitor->second);
  guild_module::do_notify_guild_pos_to_clt(char_id, tar_pos);
  return 0;
}
int guild_module::do_demise_chairman(player_obj *player, const int tar_id)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  if (mitor->second->position_ != guild_member_info::POS_HZ)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  guild_member_map_iter cmitor = itor->second->member_map_.find(tar_id);
  if (cmitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_EXIST);

  cmitor->second->position_ = guild_member_info::POS_HZ;
  guild_module::do_update_guild_member_2_db(cmitor->second);

  itor->second->base_info_->chairman_id_ = tar_id;
  guild_module::do_update_guild_2_db(itor->second->base_info_);

  mitor->second->position_ = guild_member_info::POS_HY;
  guild_module::do_update_guild_member_2_db(mitor->second);
  guild_module::on_guild_demise(player->guild_id());
  guild_module::do_notify_guild_pos_to_clt(player->id(), guild_member_info::POS_HY);
  guild_module::do_notify_guild_pos_to_clt(tar_id, guild_member_info::POS_HZ);
  return 0;
}
int guild_module::clt_guild_expel_mem(player_obj *player, const char *msg, const int len)
{
  int char_id = 0;
  in_stream is(msg, len);
  is >> char_id;

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter cmitor = itor->second->member_map_.find(char_id);
  if (cmitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_EXIST);

  const int pos = mitor->second->position_;
  const int tar_pos = cmitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_expel_mem(tar_pos, pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  // mail
  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::GUILD_EXPEL_MEM);
  if (mo != NULL)
  {
    mail_module::do_send_mail(char_id, mail_info::MAIL_SEND_SYSTEM_ID,
                              mo->sender_name_, mo->title_, mo->content_,
                              mail_info::MAIL_TYPE_GUILD,
                              0, 0, 0,
                              0, NULL,
                              player->db_sid(), time_util::now);
    player_obj *member = player_mgr::instance()->find(char_id);
    if (member != NULL)
      mail_module::do_notify_haved_new_mail(member, 1);
  }

  guild_module::do_destory_guild_member(player->guild_id(), char_id);
  return 0;
}
int guild_module::clt_guild_info(player_obj *player, const char *, const int)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  all_guild_info *agi = itor->second;
  const int cm_id = agi->base_info_->chairman_id_;
  char_brief_info *cm = all_char_info::instance()->get_char_brief_info(cm_id);
  if (cm == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = agi->member_map_.find(player->id());
  if (mitor == agi->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_module::do_sort_guild();

  out_stream os(client::send_buf, client::send_buf_len);
  os << (short)all_guild_info::guild_sort_map_rank(player->guild_id());

  os << agi->base_info_->guild_id_
    << stream_ostr(agi->base_info_->name_, ::strlen(agi->base_info_->name_));
  guild_module::do_fetch_chairman_info(cm, os);
  os << (short)agi->member_map_.size()
    << stream_ostr(agi->base_info_->purpose_, ::strlen(agi->base_info_->purpose_))
    << agi->base_info_->lvl_
    << agi->zhan_li_
    << agi->base_info_->guild_resource_
    << agi->base_info_->apply_dj_limit_
    << agi->base_info_->apply_zl_limit_
    << mitor->second->position_
    << mitor->second->contrib_
    << mitor->second->total_contrib_
    << agi->base_info_->zq_lvl_
    << agi->base_info_->tld_lvl_
    << agi->base_info_->jbd_lvl_
    << agi->base_info_->scp_lvl_
    << player->guild_free_jb_cnt()
    << player->guild_cost_jb_cnt()
    << player->guild_promote_skill_cnt()
    << guild_module::to_get_scp_doing(player->guild_id(), player->id());
  return player->send_respond_ok(RES_GUILD_INFO, &os);
}
int guild_module::clt_guild_purpose_set(player_obj *player, const char *msg, const int len)
{
  char purpose[MAX_GUILD_PURPOSE_LEN + 1];
  stream_istr purpose_si(purpose, sizeof(purpose));
  in_stream is(msg, len);
  is >> purpose_si;

  if (!util::verify_db(purpose_si.str()))
    return player->send_respond_err(RES_GUILD_PURPOSE_SET, ERR_INPUT_IS_ILLEGAL);

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(RES_GUILD_PURPOSE_SET, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(RES_GUILD_PURPOSE_SET, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_set_purpose(pos))
    return player->send_respond_err(RES_GUILD_PURPOSE_SET, ERR_GUILD_RIGHT_NO_HAD);

  ::strncpy(itor->second->base_info_->purpose_,
            purpose,
            sizeof(itor->second->base_info_->purpose_) - 1);

  guild_module::do_update_guild_2_db(itor->second->base_info_);
  return player->send_respond_ok(RES_GUILD_PURPOSE_SET);
}
int guild_module::clt_guild_up_building(player_obj *player, const char *msg, const int len)
{
  char building_type = 0;
  in_stream is(msg, len);
  is >> building_type;

  if (building_type < GUILD_BLD_XXX
      || building_type > GUILD_BLD_END)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  all_guild_info *agi = itor->second;
  guild_member_map_iter mitor = agi->member_map_.find(player->id());
  if (mitor == agi->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_up_buliding_lvl(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  int cost = MAX_INVALID_INT;
  switch (building_type)
  {
  case GUILD_BLD_DA_TING:
    if (guild_lvl_config::instance()->is_lvl_valid(agi->base_info_->lvl_ + 1))
      cost = guild_lvl_config::instance()->dt_lvl_up_cost(agi->base_info_->lvl_ + 1);
    break;
  case GUILD_BLD_ZHAN_QI:
    if (agi->base_info_->zq_lvl_ < agi->base_info_->lvl_)
      cost = guild_lvl_config::instance()->zq_lvl_up_cost(agi->base_info_->zq_lvl_ + 1);
    break;
  case GUILD_BLD_TU_LONG_DIAN:
    if (agi->base_info_->tld_lvl_ < agi->base_info_->lvl_)
      cost = guild_lvl_config::instance()->tld_lvl_up_cost(agi->base_info_->tld_lvl_ + 1);
    break;
  case GUILD_BLD_JU_BAO_DAI:
    if (agi->base_info_->jbd_lvl_ < agi->base_info_->lvl_)
      cost = guild_lvl_config::instance()->jbd_lvl_up_cost(agi->base_info_->jbd_lvl_ + 1);
    break;
  case GUILD_BLD_SCP:
    if (agi->base_info_->scp_lvl_ < agi->base_info_->lvl_)
      cost = guild_lvl_config::instance()->scp_lvl_up_cost(agi->base_info_->scp_lvl_ + 1);
    break;
  }
  if (cost == MAX_INVALID_INT)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_DA_TING_LVL_LACK);

  if (agi->base_info_->guild_resource_ < cost)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RESOURCE_LACK);

  agi->base_info_->guild_resource_ -= cost;
  switch (building_type)
  {
  case GUILD_BLD_DA_TING:
    {
      ++agi->base_info_->lvl_;
      notice_module::guild_lvl_up(player->guild_id(),
                                  player->id(),
                                  player->name(),
                                  agi->base_info_->lvl_);
    }
    break;
  case GUILD_BLD_ZHAN_QI:
    ++agi->base_info_->zq_lvl_;
    break;
  case GUILD_BLD_TU_LONG_DIAN:
    ++agi->base_info_->tld_lvl_;
    break;
  case GUILD_BLD_JU_BAO_DAI:
    ++agi->base_info_->jbd_lvl_;
    break;
  case GUILD_BLD_SCP:
    ++agi->base_info_->scp_lvl_;
    break;
  }
  guild_module::do_update_guild_2_db(agi->base_info_);
  guild_module::do_notify_resource_to_clt(player, itor->second->base_info_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << building_type;
  return player->send_request(RES_GUILD_UP_BULIDING, &os);
}
int guild_module::clt_guild_juan_xian(player_obj *player, const char *msg, const int len)
{
  int diamond = 0;
  in_stream is(msg, len);
  is >> diamond;

  if (diamond <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  if (mitor->second->total_contrib_ >= MAX_VALID_INT
      && itor->second->base_info_->guild_resource_ >= MAX_VALID_INT)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAPACITY_OUT_OF_LIMIT);

  int64_t add_resource = diamond * global_param_cfg::guild_juan_xian_resource_rate;
  int left_resource = MAX_VALID_INT - itor->second->base_info_->guild_resource_;
  add_resource > left_resource ? add_resource = left_resource : 0;
  int cost_diamond = add_resource / global_param_cfg::guild_juan_xian_resource_rate;

  int64_t add_contrib = diamond * global_param_cfg::guild_juan_xian_contrib_rate;
  int left_contrib = MAX_VALID_INT - mitor->second->total_contrib_;
  add_contrib > left_contrib ? add_contrib = left_contrib : 0;
  const int contrib_cost = add_contrib / global_param_cfg::guild_juan_xian_contrib_rate;
  cost_diamond < contrib_cost ? cost_diamond = contrib_cost : 0;

  const int ret = player->is_money_enough(M_DIAMOND, cost_diamond);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_lose_money(cost_diamond,
                        M_DIAMOND,
                        MONEY_LOSE_GUILD_JUAN_XIAN,
                        0, 0, 0);

  guild_module::do_got_resource(player->guild_id(), add_resource);
  guild_module::do_got_contrib(player->guild_id(), player->id(), add_contrib);
  guild_module::do_notify_resource_to_clt(player, itor->second->base_info_);
  return player->send_respond_ok(RES_GUILD_JUAN_XIAN);
}
int guild_module::clt_guild_ju_bao(player_obj *player, const char *, const int)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  bool is_cost = false;
  const int jbd_lvl = itor->second->base_info_->jbd_lvl_;

  if ((guild_lvl_config::instance()->max_day_jb_cnt(jbd_lvl)
       - player->guild_free_jb_cnt()
       - player->guild_cost_jb_cnt()) <= 0
      && vip_module::to_get_left_guild_ju_bao_cnt(player) <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_JU_BAO_CNT_LACK);

  if (mitor->second->total_contrib_ >= MAX_VALID_INT)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAPACITY_OUT_OF_LIMIT);

  const int award_cid  = guild_lvl_config::instance()->ju_bao_award_item(jbd_lvl);
  const int award_amt  = 1;
  const int award_bind = BIND_TYPE;
  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    award_cid,
                                    award_amt,
                                    award_bind))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  if (!is_cost
      && (guild_lvl_config::instance()->free_day_jb_cnt(jbd_lvl) - player->guild_free_jb_cnt()) > 0)
  {
    player->guild_free_jb_cnt(player->guild_free_jb_cnt() + 1);
    player->db_save_daily_clean_info();
    is_cost = true;
  }
  if (!is_cost
      && vip_module::to_get_left_guild_ju_bao_cnt(player) > 0)
  {
    vip_module::do_cost_guild_ju_bao_cnt(player);
    is_cost = true;
  }
  if (!is_cost)
  {
    const int cost = global_param_cfg::guild_jb_init_cost
      + global_param_cfg::guild_jb_cost_add
      * player->guild_cost_jb_cnt();
    const int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND, cost);
    if (ret != 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);

    player->do_lose_money(cost,
                          M_BIND_UNBIND_DIAMOND,
                          MONEY_LOSE_GUILD_JU_BAO,
                          0, 0, 0);
    player->guild_cost_jb_cnt(player->guild_cost_jb_cnt() + 1);
    player->db_save_daily_clean_info();
  }

  package_module::do_insert_item(player,
                                 PKG_PACKAGE,
                                 award_cid,
                                 award_amt,
                                 award_bind,
                                 ITEM_GOT_GUILD_JU_BAO,
                                 0,
                                 0);

  // add contrib
  int add_contrib = guild_lvl_config::instance()->ju_bao_award_contrib(jbd_lvl);
  int left_contrib = MAX_VALID_INT - mitor->second->total_contrib_;
  add_contrib > left_contrib ? add_contrib = left_contrib : 0;
  guild_module::do_got_contrib(player->guild_id(), player->id(), add_contrib);

  huo_yue_du_module::on_guild_ju_bao(player);
  cheng_jiu_module::on_guild_ju_bao(player);

  return player->send_respond_ok(RES_GUILD_JU_BAO);
}
int guild_module::clt_guild_summon_boss(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_summon_boss(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  if (time_util::wday() != global_param_cfg::guild_summon_boss_wday)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_SUMMON_WDAY);

  guild_info *gi = itor->second->base_info_;
  if (time_util::diff_days(gi->last_summon_boss_time_) <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_TODAY_HAD_SUMMON);

  const int cost = guild_lvl_config::instance()->summon_boss_cost(gi->tld_lvl_);
  if (gi->guild_resource_ < cost)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RESOURCE_LACK);

  const int boss_cid = guild_lvl_config::instance()->tu_long_dian_boss(gi->tld_lvl_);
  int ret = guild_zhu_di_scp::do_summon_tld_boss(player, boss_cid);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  gi->guild_resource_ -= cost;
  gi->last_summon_boss_time_ = time_util::now;
  guild_module::do_update_guild_2_db(gi);

  // bro to member
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  guild_module::broadcast_to_guild(player->guild_id(), NTF_GUILD_SUMMON_BOSS_OVER, 0, &mb);

  guild_module::do_notify_resource_to_clt(player, itor->second->base_info_);
  notice_module::guild_summon_boss(player->guild_id(), player->id(), player->name());
  return player->send_respond_ok(RES_GUILD_SUMMON_BOSS);
}
int guild_module::clt_guild_enter_zhu_di(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  if (player->scene_cid() == global_param_cfg::guild_zhu_di_cid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_ALREADY_IN_ZHU_DI);

  const int ret = guild_zhu_di_scp::do_enter_zhu_di(player);
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int guild_module::clt_guild_skill_list(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  out_stream os(client::send_buf, client::send_buf_len);
  guild_module::do_build_guild_skill_list(player, os);
  return player->send_request(RES_GUILD_SKILL_LIST, &os);
}
int guild_module::clt_guild_promote_skill(player_obj *player, const char *msg, const int len)
{
  int skill_cid = 0;
  in_stream is(msg, len);
  is >> skill_cid;

  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  if (!clsid::is_guild_skill(skill_cid)
      || (itor->second->base_info_->zq_lvl_
          < guild_skill_config::instance()->min_zhan_qi_lvl(skill_cid)))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  guild_skill_info *skill = guild_module::to_find_guild_skill(player, skill_cid);
  const short skill_lvl = skill == NULL ? 1 : skill->lvl_ + 1;
  const char zq_lvl = itor->second->base_info_->zq_lvl_;
  const int max_skill_lvl = guild_lvl_config::instance()->max_skill_lvl(zq_lvl);
  if (skill_lvl > max_skill_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_ZHNA_QI_LVL_LACK);

  const int cost_contrib = guild_skill_config::instance()->learn_cost(skill_cid, skill_lvl);
  if (mitor->second->contrib_ < cost_contrib)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CONTIRB_LACK);

  mitor->second->contrib_ -= cost_contrib;
  guild_module::do_update_guild_member_2_db(mitor->second);
  player->guild_promote_skill_cnt(player->guild_promote_skill_cnt() + 1);
  player->db_save_daily_clean_info();

  guild_module::do_promote_guild_skill(player, skill_cid, skill_lvl);

  guild_module::do_notify_contrib_to_clt(player, mitor->second);
  out_stream os(client::send_buf, client::send_buf_len);
  os << skill_cid
    << skill_lvl
    << player->guild_promote_skill_cnt();
  return player->send_request(RES_GUILD_PROMOTE_SKILL, &os);
}
int guild_module::clt_enter_ghz(player_obj *player, const char *, const int )
{
  int ret = ghz_module::do_enter_ghz(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int guild_module::clt_ghz_activate_shou_wei(player_obj *player, const char *msg, const int len)
{
  int cid = 0;
  in_stream is(msg, len);
  is >> cid;
  if (cid <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = ghz_module::do_activate_shou_wei(player, cid);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return player->send_respond_ok(RES_GHZ_ACTIVATE_SHOU_WEI, NULL);
}
int guild_module::clt_ghz_obtain_fighting_info(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  int ret = ghz_module::do_obtain_fighting_info(player, os);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return player->send_respond_ok(RES_GHZ_OBTAIN_FIGHTING_INFO, &os);
}
int guild_module::clt_start_guild_scp(player_obj *player, const char *msg, const int len)
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_open_scp(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  if (itor->second->base_info_->scp_lvl_ <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  if (time_util::wday() != global_param_cfg::guild_open_scp_wday)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_NO_REACH_TIME);

  if (!time_util::diff_week(itor->second->base_info_->scp_last_open_time_))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_NO_REACH_TIME);

  char dff = 0;
  in_stream is(msg, len);
  is >> dff;

  if (dff != SCP_DFF_PU_TONG
      && dff != SCP_DFF_JING_YING
      && dff != SCP_DFF_YING_XIONG)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  scp_obj *so = scp_module::construct_scp(global_param_cfg::guild_scp_cid,
                                          0,
                                          0,
                                          itor->second->base_info_->guild_id_);
  if (so == NULL || so->open() != 0)
  {
    e_log->rinfo("create guild %d scp failed!", itor->second->base_info_->guild_id_);
    delete so;
    return 0;
  }

  itor->second->scp_id_ = so->first_scene_id();
  itor->second->base_info_->scp_last_open_time_ = time_util::now;
  guild_module::do_update_guild_2_db(itor->second->base_info_);

  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_GUILD_INIT_SCP);
  mb << dff << itor->second->base_info_->scp_lvl_;
  scp_mgr::instance()->do_something(so->first_scene_id(), &mb, NULL, NULL);
  guild_module::do_notify_guild_scp_open(itor->second->base_info_->guild_id_, dff);
  return 0;
}
int guild_module::clt_enter_guild_scp(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  if (itor->second->scp_id_ == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_SCP_OVER);

  if (player->char_extra_info_->guild_scp_time_ != 0
      && time_util::diff_days(player->char_extra_info_->guild_scp_time_) == 0) 
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_SCP_TODAY_HAD_ENTER);

  int ret = 0;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_GUILD_SCP_CAN_ENTER);
  scp_mgr::instance()->do_something(itor->second->scp_id_, &mb, &ret, NULL);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = player->do_enter_scp(itor->second->scp_id_, global_param_cfg::guild_scp_cid);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  player->char_extra_info_->guild_scp_time_ = time_util::now;
  player->db_save_char_extra_info();
  return player->send_respond_ok(RES_ENTER_GUILD_SCP);
}
void guild_module::do_notify_guild_scp_open(const int guild_id, const char difficulty)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << difficulty;
  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(guild_id, NTF_GUILD_SCP_OPEN, 0, &mb);
}
void guild_module::do_notify_guild_scp_over(const int guild_id, const char result)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << result;
  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(guild_id, NTF_GUILD_SCP_OVER, 0, &mb);
}
char guild_module::to_get_scp_doing(const int guild_id, const int char_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end())
    return GSD_CANT_OPEN;

  if (itor->second->scp_id_ != 0)
    return GSD_ON_OPEN;

  guild_member_map_iter mitor = itor->second->member_map_.find(char_id);
  if (mitor == itor->second->member_map_.end())
    return GSD_CANT_OPEN;

  const int pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_open_scp(pos))
    return GSD_CANT_OPEN;

  if (itor->second->base_info_->scp_lvl_ <= 0)
    return GSD_CANT_OPEN;

  if (time_util::wday() != global_param_cfg::guild_open_scp_wday)
    return GSD_CANT_OPEN;

  if (!time_util::diff_week(itor->second->base_info_->scp_last_open_time_))
    return GSD_CANT_OPEN;
  return GSD_CAN_OPEN;
}
void guild_module::on_close_scp(const int guild_id)
{
  guild_map_iter itor = s_guild_map.find(guild_id);
  if (itor == s_guild_map.end()) return ;
  itor->second->scp_id_ = 0;
}
int guild_module::clt_summon_guild_mem(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  const char pos = mitor->second->position_;
  if (!guild_pos_config::instance()->had_right_to_summon_mem(pos))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_RIGHT_NO_HAD);

  if (!clsid::is_world_scene(player->scene_cid())
      && player->scene_cid() != global_param_cfg::guild_zhu_di_cid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MAP_CANT_GUILD_SUMMON_MEM);

  const int ret = player->is_money_enough(M_COIN, global_param_cfg::guild_summon_mem_cost);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_lose_money(global_param_cfg::guild_summon_mem_cost,
                        M_COIN,
                        MONEY_LOSE_GUILD_SUMMON_MEM,
                        0,
                        0,
                        0);

  //=
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << stream_ostr(player->name(), ::strlen(player->name()))
    << pos
    << player->scene_cid();
  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(player->guild_id(),
                                   NTF_SUMMON_GUILD_MEM,
                                   player->id(),
                                   &mb);
  return player->send_respond_ok(RES_SUMMON_GUILD_MEM);
}
int guild_module::clt_transfer_by_guild_summon(player_obj *player, const char *, const int )
{
  guild_map_iter itor = s_guild_map.find(player->guild_id());
  if (itor == s_guild_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  guild_member_map_iter mitor = itor->second->member_map_.find(player->id());
  if (mitor == itor->second->member_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_GUILD_CHAR_NO_HAD);

  player_obj *chairman = player_mgr::instance()->find(itor->second->base_info_->chairman_id_);
  if (chairman == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAN_NOT_TRANSFER);

  if (!clsid::is_world_scene(chairman->scene_cid())
      && chairman->scene_cid() != global_param_cfg::guild_zhu_di_cid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAN_NOT_TRANSFER);

  if (chairman->scene_cid() == global_param_cfg::guild_zhu_di_cid)
  {
    const int ret = guild_zhu_di_scp::do_enter_zhu_di(player);
    if (ret < 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  }else
  {
    const int ret = player->can_transfer_to(chairman->scene_cid());
    if (ret != 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);
    coord_t pos = scene_config::instance()->get_random_pos((chairman->scene_cid()),
                                                           chairman->coord_x(),
                                                           chairman->coord_y(),
                                                           GUILD_SUMMON_RADIUS);
    player->do_transfer_to(chairman->scene_cid(),
                           chairman->scene_id(),
                           pos.x_,
                           pos.y_);
  }
  return 0;
}
