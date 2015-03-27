#include "request_mgr.h"
#include "world_svc.h"
#include "world_svc_mgr.h"
#include "global_macros.h"

// Lib header
#include <tr1/unordered_map>

class request_mgr_impl
{
  struct ws_info
  {
    char group_[MAX_GROUP_NAME_LEN + 1];
    int svc_id_;
    int data_;
  };
public:
  typedef std::tr1::unordered_map<int, ws_info *> ws_map_t;
  typedef std::tr1::unordered_map<int, ws_info *>::iterator ws_map_itor;

  request_mgr_impl() :
    last_sid_(1),
    ws_map_(1024)
  { }

  int store(const world_svc *ws, const int data)
  {
    ws_info *info = new ws_info();
    ::strncpy(info->group_, wx->group(), sizeof(info->group_) - 1);
    info->svc_id_ = ws->svc_id();
    info->data_ = data;

    int sid = this->alloc_request_id();
    this->ws_map_.insert(std::make_pair(sid, info));
    return sid;
  }

  world_svc *get(const int sid, int &data)
  {
    ws_map_itor iter = this->ws_map_.find(sid);
    if (iter == this->ws_map_.end()) return NULL;

    data = iter->second->data_;
    world_svc *ws = world_svc_mgr::instance()->find(iter->second->group_,
                                                    iter->second->svc_id_);
    delete iter->second;
    this->ws_map_.erase(iter);
    return ws;
  }

  int alloc_request_id()
  {
    if (this->last_sid_ > 2000000000)
      this->last_sid_ = 1;
    return this->last_sid_++;
  }
private:
  int last_sid_;
  ws_map_t ws_map_;
};
//=
request_mgr::request_mgr()
: impl_(new request_mgr_impl())
{ }
int request_mgr::store(const world_svc *po, const int data)
{ return this->impl_->store(po, data); }
world_svc *request_mgr::get(const int sid, int &data)
{ return this->impl_->get(sid, data); }
