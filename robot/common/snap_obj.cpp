#include "snap_obj.h"

#include <ext/hash_map>
using namespace __gnu_cxx;

snap_obj::snap_obj()
  : dir_(0),
  coord_x_(0),
  coord_y_(0),
  obj_id_(0),
  obj_cid_(0),
  scene_id_(0),
  scene_cid_(0)
{ }

/**
 * @class snap_obj_mgr_impl
 *
 * @brief
 */
class snap_obj_mgr_impl
{
public:
  typedef hash_map<int, snap_obj *> snap_obj_map_t;
  typedef hash_map<int, snap_obj *>::iterator snap_obj_map_iter;

  snap_obj_mgr_impl()
    : snap_obj_map_(512)
  { }

  void insert(const int obj_id, snap_obj *so)
  { this->snap_obj_map_.insert(std::make_pair(obj_id, so)); }

  snap_obj *find(const int obj_id)
  {
    snap_obj_map_iter iter = this->snap_obj_map_.find(obj_id);
    if (iter == this->snap_obj_map_.end()) return NULL;
    return iter->second;
  }
  int get_random_char_id()
  {
    int index = rand() % this->snap_obj_map_.size();
    for (snap_obj_map_iter iter = this->snap_obj_map_.begin();
         iter != this->snap_obj_map_.end();
         ++iter, --index)
    {
      if (index == 0)
        return iter->first;
    }
    return 0;
  }

private:
  snap_obj_map_t snap_obj_map_;
};

snap_obj_mgr::snap_obj_mgr()
  : impl_(new snap_obj_mgr_impl())
{ }
void snap_obj_mgr::insert(const int obj_id, snap_obj *so)
{ this->impl_->insert(obj_id, so); }
snap_obj* snap_obj_mgr::find(const int obj_id)
{ return this->impl_->find(obj_id); }
int snap_obj_mgr::get_random_char_id()
{ return this->impl_->get_random_char_id(); }

