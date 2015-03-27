#include "ltime_act_mgr.h"
#include "ltime_act_obj.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *e_log = err_log::instance()->get_ilog("act");

/**
 * @class ltime_act_mgr_impl
 *
 * @brief
 */
class ltime_act_mgr_impl
{
  typedef std::map<int/*act id*/, ltime_act_obj *> act_obj_map_t;
  typedef std::map<int/*act id*/, ltime_act_obj *>::iterator act_obj_map_itor;
public:
  ltime_act_mgr_impl() { }
  void insert(const int act_id, ltime_act_obj *lao)
  { this->act_obj_map_.insert(std::make_pair(act_id, lao)); }
  ltime_act_obj *find(const int act_id)
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
    for (act_obj_map_itor itor = this->act_obj_map_.begin();
         itor != this->act_obj_map_.end();
         ++itor)
      itor->second->run(now);
  }
  void on_enter_game(player_obj *player)
  {
    for (act_obj_map_itor itor = this->act_obj_map_.begin();
         itor != this->act_obj_map_.end();
         ++itor)
      itor->second->on_enter_game(player);
  }
private:
  act_obj_map_t act_obj_map_;
};
// ------------------------------------------------------------------
ltime_act_mgr::ltime_act_mgr() : impl_(new ltime_act_mgr_impl()) { }
void ltime_act_mgr::insert(const int act_id, ltime_act_obj *lao)
{ return this->impl_->insert(act_id, lao); }
void ltime_act_mgr::run(const int now)
{ this->impl_->run(now); }
ltime_act_obj *ltime_act_mgr::find(const int act_id)
{ return this->impl_->find(act_id); }
bool ltime_act_mgr::is_opened(const int act_id)
{ return this->impl_->is_opened(act_id); }
void ltime_act_mgr::on_enter_game(player_obj *player)
{ this->impl_->on_enter_game(player); }
