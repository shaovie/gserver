#include "world_svc_mgr.h"
#include "world_svc.h"
#include "sys_log.h"

// Lib header
#include <map>
#include <string>

static ilog_obj *s_log = sys_log::instance()->get_ilog("world_svc");
static ilog_obj *e_log = err_log::instance()->get_ilog("world_svc");

class group_info
{
public:
  typedef std::map<int /*id*/, world_svc *> world_svc_map_t;
  typedef std::map<int /*id*/, world_svc *>::iterator world_svc_map_itor;
  world_svc_map_t world_svc_map_;
};

class world_svc_mgr_impl
{
public:
  world_svc *find(const char *group, const int id)
  {
    group_map_itor itor = this->group_map_.find(std::string(group));
    if (itor == this->group_map_.end()) return NULL;

    group_info::world_svc_map_itor w_iter = itor->second->world_svc_map_.find(id);
    if (w_iter == itor->second->world_svc_map_.end())
      return NULL;

    return w_iter->second;
  }
  world_svc *random(const char *group)
  {
    group_map_itor itor = this->group_map_.find(std::string(group));
    if (itor == this->group_map_.end()
        || itor->second->world_svc_map_.empty())
      return NULL;

    int choose = rand() % itor->second->world_svc_map_.size();
    group_info::world_svc_map_itor w_iter = itor->second->world_svc_map_.begin();
    while (choose --) ++w_iter;

    return w_iter->second;
  }
  void insert(const char *group, const int id, world_svc *ws)
  {
    group_info *info = NULL;
    group_map_itor itor = this->group_map_.find(std::string(group));
    if (itor != this->group_map_.end())
      info = itor->second;
    else
    {
      info = new group_info;
      this->group_map_.insert(std::make_pair(std::string(group), info));
    }

    std::pair<group_info::world_svc_map_itor, bool> ret
      = info->world_svc_map_.insert(std::make_pair(id, ws));
    if (!ret.second)
      e_log->rinfo("insert world[group=%s id=%d] failed!", group, id);
  }
  void remove(const char *group, const int id)
  {
    group_map_itor itor = this->group_map_.find(std::string(group));
    if (itor == this->group_map_.end()
        || itor->second->world_svc_map_.erase(id) != 1)
      e_log->rinfo("remove world[group=%s id=%d] failed!", group, id);
  }
  int world_svc_cnt()
  {
    int svc_sum = 0;
    for (group_map_itor itor = this->group_map_.begin();
         itor != this->group_map_.end();
         ++itor)
      svc_sum += itor->second->world_svc_map_.size();
    return svc_sum;
  }

  void dump(const int now)
  {
    static int last = now;
    if (now - last < 5) return ;
    last = now;

    s_log->debug("There is %d worlds!", this->world_svc_cnt());
    for (group_map_itor itor = this->group_map_.begin();
         itor != this->group_map_.end();
         ++ itor)
    {
      group_info *info = itor->second;
      s_log->debug("-- Platform %s has %d worlds",
                   itor->first.c_str(), info->world_svc_map_.size());
      for (group_info::world_svc_map_itor w_itor = info->world_svc_map_.begin();
           w_itor != info->world_svc_map_.end();
           ++ w_itor)
        s_log->debug("   -- World ID %d", w_itor->second->svc_id());
    }
  }
private:
  typedef std::map<std::string /*group*/, group_info *> group_map_t;
  typedef std::map<std::string /*group*/, group_info *>::iterator group_map_itor;
  group_map_t group_map_;
};
// ---------------------------------------------------------------
world_svc_mgr::world_svc_mgr() : impl_(new world_svc_mgr_impl()) { }
world_svc *world_svc_mgr::find(const char *group, const int id)
{ return this->impl_->find(group, id); }
void world_svc_mgr::insert(const char *group, const int id, world_svc *ws)
{ this->impl_->insert(group, id, ws); }
void world_svc_mgr::remove(const char *group, const int id)
{ this->impl_->remove(group, id); }
int world_svc_mgr::world_svc_cnt()
{ return this->impl_->world_svc_cnt(); }
void world_svc_mgr::dump(const int now)
{ return this->impl_->dump(now); }
world_svc *world_svc_mgr::random(const char *group)
{ return this->impl_->random(group); }
