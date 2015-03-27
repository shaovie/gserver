#include "dropped_item_mgr.h"
#include "global_param_cfg.h"
#include "dropped_item.h"
#include "global_macros.h"
#include "time_util.h"
#include "scene_mgr.h"
#include "message.h"
#include "client.h"

// Lib header
#include <tr1/unordered_map>

/**
 * @class dropped_item_mgr_impl
 * 
 * @brief implement of dropped_item_mgr
 */
class dropped_item_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int/*item id*/, dropped_item *> dropped_item_map_t;
  typedef std::tr1::unordered_map<int/*item id*/, dropped_item *>::iterator dropped_item_map_itor;

  dropped_item_mgr_impl()
    : dropped_item_id_(DROPPED_ITEM_ID_BASE),
      dropped_item_map_(10000)
  { }
  //=
  int assign_dropped_item_id()
  {
    if (this->dropped_item_id_ >= (DROPPED_ITEM_ID_BASE + DROPPED_ITEM_ID_RANGE - 1))
      this->dropped_item_id_ = DROPPED_ITEM_ID_BASE;
    return ++this->dropped_item_id_;
  }
  void insert(const int id, dropped_item *obj)
  { this->dropped_item_map_.insert(std::make_pair(id, obj)); }
  dropped_item *find(const int id)
  {
    dropped_item_map_itor itor = this->dropped_item_map_.find(id);
    return itor == this->dropped_item_map_.end() ? NULL : itor->second;
  }
  void remove(const int id)
  { this->dropped_item_map_.erase(id); }

  void do_timeout()
  {
    dropped_item_map_itor itor = this->dropped_item_map_.begin();
    while (itor != this->dropped_item_map_.end())
    {
      if (time_util::now - itor->second->dropped_time_
          > global_param_cfg::dropped_item_destroy_time)
      {
        itor->second->do_exit_scene();
        dropped_item_pool::instance()->release(itor->second);
        this->dropped_item_map_.erase(itor++);
      }else if (time_util::now - itor->second->dropped_time_
                > global_param_cfg::dropped_item_free_time)
      {
        itor->second->owner_id_ = 0;

        // broadcast
        mblock mb(client::send_buf, client::send_buf_len);
        proto_head *ph = (proto_head *)mb.rd_ptr();
        mb.wr_ptr(sizeof(proto_head));
        mb << itor->second->id() << itor->second->owner_id_;
        ph->set(0, NTF_BROADCAST_DROPPED_ITEM_OWNER_ID, 0, mb.length());
        scene_mgr::instance()->broadcast(itor->second->scene_id(),
                                         0,
                                         false,
                                         itor->second->coord_x(),
                                         itor->second->coord_y(),
                                         &mb);

        itor++;
      }else
        itor++;
    }
  }
private:
  int dropped_item_id_;
  dropped_item_map_t dropped_item_map_;
};
//------------------------------------------------------
dropped_item_mgr::dropped_item_mgr()
: impl_(new dropped_item_mgr_impl())
{ }
void dropped_item_mgr::insert(const int id, dropped_item *obj)
{ this->impl_->insert(id, obj); }
dropped_item *dropped_item_mgr::find(const int id)
{ return this->impl_->find(id); }
void dropped_item_mgr::remove(const int id)
{ return this->impl_->remove(id); }
int dropped_item_mgr::assign_dropped_item_id()
{ return this->impl_->assign_dropped_item_id(); }
void dropped_item_mgr::do_timeout()
{ this->impl_->do_timeout(); }
