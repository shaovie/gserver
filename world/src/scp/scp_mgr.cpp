#include "scp_mgr.h"
#include "sys_log.h"
#include "scp_obj.h"
#include "sub_scp_obj.h"
#include "global_macros.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

/**
 * @class scp_mgr_impl
 *
 * @brief
 */
class scp_mgr_impl
{
public:
  typedef std::map<int/*owner*/, scp_obj *> scp_owner_map_t;
  typedef std::map<int/*owner*/, scp_obj *>::iterator scp_owner_map_itor;

  typedef std::map<int/*scp id*/, sub_scp_obj *> sub_scp_map_t;
  typedef std::map<int/*scp id*/, sub_scp_obj *>::iterator sub_scp_map_itor;

public:
  scp_mgr_impl()
  {
    for (int i = MIN_SCP_ID + 1; i < MAX_SCP_ID; ++i)
      this->scp_id_list_.push_back(i);
  }
  void run(const int now)
  {
    int size = this->scp_list_.size();
    for (int i = 0; i < size; ++i)
    {
      scp_obj *so = this->scp_list_.pop_front();
      if (so->run(now) != 0)
        delete so;
      else
        this->scp_list_.push_back(so);
    }
  }

  int size() { return this->scp_list_.size(); }
  int assign_scp_id()
  {
    if (this->scp_id_list_.empty()) return -1;
    return this->scp_id_list_.pop_front();
  }
  void release_scp_id(const int scp_id)
  { this->scp_id_list_.push_back(scp_id); }

  void insert_scp(scp_obj *so)
  { this->scp_list_.push_back(so); }
  void remove_scp(scp_obj *so)
  { this->scp_list_.remove(so); }
  int insert_sub_scp(const int scp_id, sub_scp_obj *sso)
  {
    sub_scp_map_itor itor = this->sub_scp_map_.find(scp_id);
    if (itor != this->sub_scp_map_.end())
    {
      e_log->rinfo("%s : sub_scp[%d] is existed", __func__, scp_id);
      return -1;
    }
    this->sub_scp_map_.insert(std::make_pair(scp_id, sso));
    return 0;
  }
  sub_scp_obj *get_sub_scp_obj(const int scp_id)
  {
    sub_scp_map_itor itor = this->sub_scp_map_.find(scp_id);
    if (itor == this->sub_scp_map_.end()) return NULL;
    return itor->second;
  }
  void remove_sub_scp(const int scp_id)
  { this->sub_scp_map_.erase(scp_id); }

  //= binding owner
  void bind_scp_char_owner(const int char_id, scp_obj *so)
  { this->scp_char_owner_map_.insert(std::make_pair(char_id, so)); }
  scp_obj *get_scp_by_char_owner(const int char_id)
  {
    scp_owner_map_itor itor = this->scp_char_owner_map_.find(char_id);
    if (itor != this->scp_char_owner_map_.end()) return itor->second;
    return NULL;
  }
  void unbind_scp_char_owner(const int char_id)
  { this->scp_char_owner_map_.erase(char_id); }

  void bind_scp_team_owner(const int team_id, scp_obj *so)
  { this->scp_team_owner_map_.insert(std::make_pair(team_id, so)); }
  scp_obj *get_scp_by_team_owner(const int team_id)
  {
    scp_owner_map_itor itor = this->scp_team_owner_map_.find(team_id);
    if (itor != this->scp_team_owner_map_.end()) return itor->second;
    return NULL;
  }
  void unbind_scp_team_owner(const int team_id)
  { this->scp_team_owner_map_.erase(team_id); }

  void bind_scp_guild_owner(const int guild_id, scp_obj *so)
  { this->scp_guild_owner_map_.insert(std::make_pair(guild_id, so)); }
  scp_obj *get_scp_by_guild_owner(const int guild_id)
  {
    scp_owner_map_itor itor = this->scp_guild_owner_map_.find(guild_id);
    if (itor != this->scp_guild_owner_map_.end()) return itor->second;
    return NULL;
  }
  void unbind_scp_guild_owner(const int guild_id)
  { this->scp_guild_owner_map_.erase(guild_id); }
#if 0
  int enter_scp(const int scp_id, const int char_id)
  {
    sub_scp_map_itor itor = this->sub_scp_map_.find(scp_id);
    if (itor == this->sub_scp_map_.end()) return -1;

    scp_obj *so = itor->second->master_scp();
    if (so == NULL) return -1;
    return so->enter_scp(char_id, scp_id);
  }
#endif
  int exit_scp(const int scp_id, const int char_id, const int r)
  {
    sub_scp_map_itor itor = this->sub_scp_map_.find(scp_id);
    if (itor == this->sub_scp_map_.end()) return -1;

    scp_obj *so = itor->second->master_scp();
    if (so == NULL) return -1;
    return so->exit_scp(char_id, scp_id, r);
  }
  void do_something(const int scp_id, mblock *mb, int *ret, mblock *result)
  {
    sub_scp_map_itor itor = this->sub_scp_map_.find(scp_id);
    if (itor == this->sub_scp_map_.end()) return ;
    itor->second->do_something(mb, ret, result);
  }
private:
  ilist<int/*scp id*/> scp_id_list_;
  ilist<scp_obj *> scp_list_;
  sub_scp_map_t sub_scp_map_;
  scp_owner_map_t scp_team_owner_map_;
  scp_owner_map_t scp_char_owner_map_;
  scp_owner_map_t scp_guild_owner_map_;
};
// ------------------------------------------------------------------------
scp_mgr::scp_mgr() : impl_(new scp_mgr_impl()) { }
void scp_mgr::run(const int now)
{ return this->impl_->run(now); }
int scp_mgr::size()
{ return this->impl_->size(); }
int scp_mgr::assign_scp_id()
{ return this->impl_->assign_scp_id(); }
void scp_mgr::release_scp_id(int scp_id)
{ this->impl_->release_scp_id(scp_id); }
void scp_mgr::insert_scp(scp_obj *so)
{ this->impl_->insert_scp(so); }
void scp_mgr::remove_scp(scp_obj *so)
{ this->impl_->remove_scp(so); }
int scp_mgr::insert_sub_scp(const int scene_id, sub_scp_obj *sso)
{ return this->impl_->insert_sub_scp(scene_id, sso); }
void scp_mgr::remove_sub_scp(const int scene_id)
{ this->impl_->remove_sub_scp(scene_id); }
sub_scp_obj *scp_mgr::get_sub_scp_obj(const int scene_id)
{ return this->impl_->get_sub_scp_obj(scene_id); }
void scp_mgr::bind_scp_char_owner(const int char_id, scp_obj *so)
{ this->impl_->bind_scp_char_owner(char_id, so); }
scp_obj *scp_mgr::get_scp_by_char_owner(const int char_id)
{ return this->impl_->get_scp_by_char_owner(char_id); }
void scp_mgr::unbind_scp_char_owner(const int char_id)
{ this->impl_->unbind_scp_char_owner(char_id); }
void scp_mgr::bind_scp_team_owner(const int team_id, scp_obj *so)
{ this->impl_->bind_scp_team_owner(team_id, so); }
scp_obj *scp_mgr::get_scp_by_team_owner(const int team_id)
{ return this->impl_->get_scp_by_team_owner(team_id); }
void scp_mgr::unbind_scp_team_owner(const int team_id)
{ this->impl_->unbind_scp_team_owner(team_id); }
void scp_mgr::bind_scp_guild_owner(const int guild_id, scp_obj *so)
{ this->impl_->bind_scp_guild_owner(guild_id, so); }
scp_obj *scp_mgr::get_scp_by_guild_owner(const int guild_id)
{ return this->impl_->get_scp_by_guild_owner(guild_id); }
void scp_mgr::unbind_scp_guild_owner(const int guild_id)
{ this->impl_->unbind_scp_guild_owner(guild_id); }
void scp_mgr::do_something(const int scene_id, mblock *mb, int *ret, mblock *result)
{ this->impl_->do_something(scene_id, mb, ret, result); }
int scp_mgr::exit_scp(const int scp_id, const int char_id, const int r)
{ return this->impl_->exit_scp(scp_id, char_id, r); }
