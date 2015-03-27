#include "db_session_mgr.h"

// Lib header
#include <tr1/unordered_map>

class db_session_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int, player_obj *> player_obj_map_t;
  typedef std::tr1::unordered_map<int, player_obj *>::iterator player_obj_map_itor;

  db_session_mgr_impl() :
    last_sid_(1),
    player_obj_map_(4096)
  { }

  void insert(const int sid, player_obj *po)
  { this->player_obj_map_.insert(std::make_pair(sid, po)); }

  void remove(const int sid)
  { this->player_obj_map_.erase(sid); }

  player_obj *find(const int sid)
  {
    player_obj_map_itor itor = this->player_obj_map_.find(sid);
    return itor == this->player_obj_map_.end() ? NULL : itor->second;
  }

  int alloc_session_id()
  {
    if (this->last_sid_ > 1000000000)
      this->last_sid_ = 1;
    return this->last_sid_++;
  }
private:
  int last_sid_;
  player_obj_map_t player_obj_map_;
};
//=
db_session_mgr::db_session_mgr()
: impl_(new db_session_mgr_impl())
{ }
int db_session_mgr::alloc_session_id()
{ return this->impl_->alloc_session_id(); }
void db_session_mgr::insert(const int sid, player_obj *po)
{ this->impl_->insert(sid, po); }
void db_session_mgr::remove(const int sid)
{ this->impl_->remove(sid); }
player_obj *db_session_mgr::find(const int sid)
{ return this->impl_->find(sid); }
