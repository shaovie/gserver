#include "monster_mgr.h"
#include "monster_obj.h"
#include "mblock_pool.h"
#include "global_macros.h"
#include "ilist.h"

// Lib header
#include <set>
#include <map>
#include "reactor.h"
#include "time_value.h"
#include "ev_handler.h"
#include <tr1/unordered_map>

/**
 * @class mst_loop_timer
 *
 * @brief
 */
class mst_loop_timer : public ev_handler
{
public:
  void insert(monster_obj *obj)
  { this->monster_obj_list_.push_back(obj); }

  virtual int handle_timeout(const time_value &now)
  {
    int size = this->monster_obj_list_.size();
    for (int i = 0; i < size; ++i)
    {
      monster_obj *p = this->monster_obj_list_.pop_front();
      if (p->do_timeout(now) != 0)
        p->do_close();
      else
        this->monster_obj_list_.push_back(p);
    }
    return 0;
  }
private:
  ilist<monster_obj *> monster_obj_list_;
};
/**
 * @class monster_mgr_impl
 *
 * @brief implement of monster_mgr
 */
class monster_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int/*mst id*/, monster_obj *> monster_obj_map_t;
  typedef std::tr1::unordered_map<int/*mst id*/, monster_obj *>::iterator monster_obj_map_itor;

  typedef std::set<int/*mst id*/> mst_id_set_t;
  typedef std::set<int/*mst id*/>::iterator mst_id_set_itor;
  typedef std::set<int/*obj id*/> obj_set_t;
  typedef std::set<int/*obj id*/>::iterator obj_set_itor;
  typedef std::map<int/*scene id*/, obj_set_t> scene_obj_set_t;
  typedef std::map<int/*scene id*/, obj_set_t>::iterator scene_obj_set_itor;

  monster_mgr_impl() :
    monster_id_(MONSTER_ID_BASE),
    monster_obj_map_(3000)
  { }
  int init(reactor *r)
  {
    for (size_t i = 0;
         i < sizeof(this->mst_loop_timer_)/sizeof(this->mst_loop_timer_[0]);
         ++i)
    {
      this->mst_loop_timer_[i].set_reactor(r);
      if (r->schedule_timer(&this->mst_loop_timer_[i],
                            time_value(0, (rand() % 500 + 100)*1000),
                            time_value(0, 30*1000)) == -1)
        return -1;
    }
    return 0;
  }
  //=
  int assign_monster_id()
  {
    int mst_id = -1;
    while (true)
    {
      mst_id = ++this->monster_id_;
      if (mst_id >= (MONSTER_ID_BASE + MONSTER_ID_RANGE))
      {
        this->monster_id_ = MONSTER_ID_BASE + 10000;
        continue;
      }
      if (this->exist_mst_id_set_.find(mst_id) == this->exist_mst_id_set_.end())
      {
        this->exist_mst_id_set_.insert(mst_id);
        return mst_id;
      }
    }
  }
  void release_monster_id(const int id)
  {
    if (id < MONSTER_ID_BASE
        || id >= (MONSTER_ID_BASE + MONSTER_ID_RANGE))
      return ;
    this->exist_mst_id_set_.erase(id);
  }
  void attach_timer(monster_obj *p)
  {
    int idx = p->id() % (sizeof(this->mst_loop_timer_)/sizeof(this->mst_loop_timer_[0]));
    this->mst_loop_timer_[idx].insert(p);
  }
  void insert(const int id, monster_obj *obj)
  { this->monster_obj_map_.insert(std::make_pair(id, obj)); }
  monster_obj *find(const int id)
  {
    monster_obj_map_itor itor = this->monster_obj_map_.find(id);
    return itor == this->monster_obj_map_.end() ? NULL : itor->second;
  }
  void remove(const int id)
  { this->monster_obj_map_.erase(id); }
  int size()
  { return this->monster_obj_map_.size(); }

  void birth_to_scene(const int scene_id, const int obj_id)
  {
    scene_obj_set_itor itor = this->scene_birth_obj_set_.find(scene_id);
    if (itor != this->scene_birth_obj_set_.end())
      itor->second.insert(obj_id);
    else
    {
      obj_set_t ost;
      ost.insert(obj_id);
      this->scene_birth_obj_set_.insert(std::make_pair(scene_id, ost));
    }
  }
  void destroy_in_scene(const int scene_id, const int obj_id)
  {
    scene_obj_set_itor itor = this->scene_birth_obj_set_.find(scene_id);
    if (itor != this->scene_birth_obj_set_.end())
      itor->second.erase(obj_id);
  }
  void destroy_scene_monster(const int scene_id)
  {
    scene_obj_set_itor itor = this->scene_birth_obj_set_.find(scene_id);
    if (itor == this->scene_birth_obj_set_.end()) return ;

    static ilist<int/*obj_id*/> destroy_list;
    for (obj_set_itor pos = itor->second.begin();
         pos != itor->second.end();
         ++pos)
      destroy_list.push_back(*pos);

    while (!destroy_list.empty())
    {
      int obj_id = destroy_list.pop_front();
      monster_obj *mst = this->find(obj_id);
      if (mst != NULL)
        mst->to_destroy();
    }
  }
private:
  int monster_id_;
  monster_obj_map_t monster_obj_map_;
  mst_loop_timer mst_loop_timer_[3];
  mst_id_set_t exist_mst_id_set_;
  scene_obj_set_t scene_birth_obj_set_;
};
//------------------------------------------------------
monster_mgr::monster_mgr() : impl_(new monster_mgr_impl()) { }
int monster_mgr::init(reactor *r)
{ return this->impl_->init(r); }
void monster_mgr::insert(const int id, monster_obj *obj)
{ this->impl_->insert(id, obj); }
monster_obj *monster_mgr::find(const int id)
{ return this->impl_->find(id); }
void monster_mgr::remove(const int id)
{ return this->impl_->remove(id); }
int monster_mgr::size()
{ return this->impl_->size(); }
int monster_mgr::assign_monster_id()
{ return this->impl_->assign_monster_id(); }
void monster_mgr::release_monster_id(const int id)
{ this->impl_->release_monster_id(id); }
void monster_mgr::attach_timer(monster_obj *p)
{ this->impl_->attach_timer(p); }
void monster_mgr::birth_to_scene(const int scene_id, const int obj_id)
{ this->impl_->birth_to_scene(scene_id, obj_id); }
void monster_mgr::destroy_in_scene(const int scene_id, const int obj_id)
{ this->impl_->destroy_in_scene(scene_id, obj_id); }
void monster_mgr::destroy_scene_monster(const int scene_id)
{ this->impl_->destroy_scene_monster(scene_id); }
