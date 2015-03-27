#include "effect_obj_mgr.h"
#include "effect_obj.h"
#include "global_macros.h"

// Lib header
#include <map>
#include "time_value.h"

/**
 * @class effect_obj_mgr_impl
 * 
 * @brief
 */
class effect_obj_mgr_impl
{
  typedef std::map<int, effect_obj *> effect_obj_map_t;
  typedef std::map<int, effect_obj *>::iterator effect_obj_map_itor;
public:
  effect_obj_mgr_impl() : effect_obj_id_(EFFECT_ID_BASE) { }
  int assign_effect_obj_id()
  {
    if (this->effect_obj_id_ >= (EFFECT_ID_BASE + EFFECT_ID_RANGE - 1))
      this->effect_obj_id_ = EFFECT_ID_BASE;
    return ++this->effect_obj_id_;
  }
  void do_timeout(const time_value &now)
  {
    effect_obj_map_itor itor = this->effect_obj_map_.begin();
    for (; itor != this->effect_obj_map_.end(); )
    {
      effect_obj *eo = itor->second;
      if (eo->do_ai(now) != 0)
      {
        eo->do_exit_scene();
        eo->release();
        this->effect_obj_map_.erase(itor++);
      }else
        itor++;
    }
  }
  void insert(effect_obj *eo)
  { this->effect_obj_map_.insert(std::make_pair(eo->id(), eo)); }
private:
  int effect_obj_id_;
  effect_obj_map_t effect_obj_map_;
};
// ---------------------------------------------------------------------
effect_obj_mgr::effect_obj_mgr() : impl_(new effect_obj_mgr_impl()) { }
void effect_obj_mgr::do_timeout(const time_value &now)
{ this->impl_->do_timeout(now); }
void effect_obj_mgr::insert(effect_obj *seo)
{ this->impl_->insert(seo); }
int effect_obj_mgr::assign_effect_obj_id()
{ return this->impl_->assign_effect_obj_id(); }
