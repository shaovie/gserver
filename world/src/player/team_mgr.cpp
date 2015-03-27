#include "team_mgr.h"

// Lib header
#include <tr1/unordered_map>

void team_info::add_member(const int mem_id)
{
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (this->mem_id_[i] == 0)
    {
      this->mem_id_[i] = mem_id;
      break;
    }
  }
}
void team_info::del_member(const int mem_id)
{
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (this->mem_id_[i] == mem_id)
    {
      this->mem_id_[i] = 0;
      break;
    }
  }
}
bool team_info::have_player(const int mem_id)
{
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
    if (this->mem_id_[i] == mem_id)
      return true;

  return false;
}
int team_info::member_count()
{
  int count = 0;
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
    if (this->mem_id_[i] != 0)
      ++count;

  return count;
}

/**
 * @class team_mgr_impl
 * 
 * @brief implement of team_mgr
 */
class team_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int, team_info*> team_map_t;
  typedef std::tr1::unordered_map<int, team_info*>::iterator team_map_itor;

  team_mgr_impl() :
    team_id_pool_(1),
    team_map_(512)
  { }
  ~team_mgr_impl()
  {
    for (team_map_itor iter = this->team_map_.begin();
         iter != this->team_map_.end();
         ++iter)
      delete iter->second;
    this->team_map_.clear();
  }

  int assign_id()
  { return this->team_id_pool_++; }
  void insert(const int team_id, team_info *ti)
  { this->team_map_.insert(std::make_pair(team_id, ti)); }
  void remove(const int team_id)
  { this->team_map_.erase(team_id); }
  team_info* find(const int team_id)
  {
    team_map_itor iter = this->team_map_.find(team_id);
    if (iter == this->team_map_.end()) return NULL;
    return iter->second;
  }
  team_info* find_by_char(const int char_id)
  {
    team_map_itor  iter = this->team_map_.begin();
    for (; iter != this->team_map_.end(); ++iter)
    {
      if (iter->second->have_player(char_id))
        return iter->second;
    }
    return NULL;
  }
  bool is_leader(const int team_id, const int char_id)
  {
    team_info *team = this->find(team_id);
    if (team == NULL) return false;
    return (team->leader() == char_id);
  }
private:
  int team_id_pool_;
  team_map_t team_map_;
};

//------------------------------------------------------
team_mgr::team_mgr() : impl_(new team_mgr_impl()) { }
int team_mgr::assign_id()
{ return this->impl_->assign_id(); }
void team_mgr::insert(const int team_id, team_info *ti)
{ this->impl_->insert(team_id, ti); }
void team_mgr::remove(const int team_id)
{ this->impl_->remove(team_id); }
team_info* team_mgr::find(const int team_id)
{ return this->impl_->find(team_id); }
team_info* team_mgr::find_by_char(const int char_id)
{ return this->impl_->find_by_char(char_id); }
bool team_mgr::is_leader(const int team_id, const int char_id)
{ return this->impl_->is_leader(team_id, char_id); }
