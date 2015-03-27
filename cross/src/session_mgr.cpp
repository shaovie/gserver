#include "session_mgr.h"

// Lib header
#include <tr1/unordered_map>

class session_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int, world_svc *> world_svc_map_t;
  typedef std::tr1::unordered_map<int, world_svc *>::iterator world_svc_map_itor;

  session_mgr_impl() :
    last_sid_(1),
    world_svc_map_(1024)
  { }

  void insert(const int sid, world_svc *po)
  { this->world_svc_map_.insert(std::make_pair(sid, po)); }

  void remove(const int sid)
  { this->world_svc_map_.erase(sid); }

  world_svc *find(const int sid)
  {
    world_svc_map_itor itor = this->world_svc_map_.find(sid);
    return itor == this->world_svc_map_.end() ? NULL : itor->second;
  }
  world_svc *random()
  {
    if (this->world_svc_map_.empty()) return NULL;
    int choose = rand() % this->world_svc_map_.size();
    world_svc_map_itor itor = this->world_svc_map_.begin();
    while (choose --) ++itor;
    return itor->second;
  }

  int alloc_session_id()
  {
    if (this->last_sid_ > 1000000000)
      this->last_sid_ = 1;
    return this->last_sid_++;
  }
private:
  int last_sid_;
  world_svc_map_t world_svc_map_;
};
//=
session_mgr::session_mgr()
: impl_(new session_mgr_impl())
{ }
int session_mgr::alloc_session_id()
{ return this->impl_->alloc_session_id(); }
void session_mgr::insert(const int sid, world_svc *po)
{ this->impl_->insert(sid, po); }
void session_mgr::remove(const int sid)
{ this->impl_->remove(sid); }
world_svc *session_mgr::find(const int sid)
{ return this->impl_->find(sid); }
world_svc *session_mgr::random()
{ return this->impl_->random(); }
