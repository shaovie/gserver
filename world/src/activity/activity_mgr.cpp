#include "activity_mgr.h"
#include "activity_module.h"
#include "activity_cfg.h"
#include "activity_obj.h"
#include "array_t.h"
#include "sys.h"
#include "error.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *e_log = err_log::instance()->get_ilog("act");

/**
 * @class activity_mgr_impl
 *
 * @brief
 */
class activity_mgr_impl
{
  typedef std::map<int/*act id*/, activity_obj *> act_obj_map_t;
  typedef std::map<int/*act id*/, activity_obj *>::iterator act_obj_map_itor;
public:
  activity_mgr_impl() { }
  int init()
  {
    ilist<activity_cfg_obj *> *acol = activity_cfg::instance()->get_act_cfg_obj_list();
    if (acol == NULL) return 0;
    for (ilist_node<activity_cfg_obj *> *itor = acol->head();
         itor != NULL;
         itor = itor->next_)
    {
      activity_obj *ao = activity_module::construct(itor->value_->act_id_);
      if (ao != NULL && ao->init() == 0)
      {
        this->act_obj_map_.insert(std::make_pair(itor->value_->act_id_, ao));
        this->act_obj_list_.push_back(ao);
      }else
        delete ao;
    }
    return 0;
  }
  activity_obj *find(const int act_id)
  {
    act_obj_map_itor itor = this->act_obj_map_.find(act_id);
    if (itor == this->act_obj_map_.end()) return NULL;
    return itor->second;
  }
  bool is_opened(const int act_id)
  {
    act_obj_map_itor itor = this->act_obj_map_.find(act_id);
    if (itor == this->act_obj_map_.end()) return false;
    return itor->second->is_opened();
  }
  void run(const int now)
  {
    ilist_node<activity_obj *> *itor = this->act_obj_list_.head();
    for (; itor != NULL; itor = itor->next_)
      itor->value_->run(now);
    return ;
  }
private:
  ilist<activity_obj *> act_obj_list_;
  act_obj_map_t act_obj_map_;
};
// ------------------------------------------------------------------
activity_mgr::activity_mgr() : impl_(new activity_mgr_impl()) { }
int activity_mgr::init()
{ return this->impl_->init(); }
void activity_mgr::run(const int now)
{ this->impl_->run(now); }
activity_obj *activity_mgr::find(const int act_id)
{ return this->impl_->find(act_id); }
bool activity_mgr::is_opened(const int act_id)
{ return this->impl_->is_opened(act_id); }
