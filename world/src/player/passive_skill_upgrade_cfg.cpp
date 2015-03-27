#include "passive_skill_upgrade_cfg.h"
#include "global_param_cfg.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"
#include "clsid.h"
#include "util.h"
#include "def.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class passive_skill_upgrade_cfg_impl
 * 
 * @brief implement of passive_skill_upgrade_cfg
 */
class passive_skill_upgrade_cfg_impl : public load_json
{
public:
  passive_skill_upgrade_cfg_impl() :
    upgrade_cfg_obj_map_(global_param_cfg::lvl_limit)
  { }
  ~passive_skill_upgrade_cfg_impl()
  {
    for (int i = 0; i < this->upgrade_cfg_obj_map_.size(); ++i)
    {
      passive_skill_upgrade_cfg_obj *p = this->upgrade_cfg_obj_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, PASSIVE_SKILL_UPGRADE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int lvl = ::atoi(iter.key().asCString());
      if (lvl > global_param_cfg::lvl_limit)
        return -1;
      passive_skill_upgrade_cfg_obj *p = new passive_skill_upgrade_cfg_obj();
      this->upgrade_cfg_obj_map_.insert(lvl, p);

      p->cost_ = (*iter)["cost"].asInt();
      p->item_cid_ = (*iter)["item_cid"].asInt();
      p->item_cnt_ = (*iter)["item_cnt"].asInt();
      p->probability_ = (*iter)["probability"].asInt();
      p->lucky_rate_ = (*iter)["lucky_rate"].asInt();
      p->min_upgrade_cnt_ = (*iter)["min_upgrade_cnt"].asInt();
      p->min_bless_val_ = (*iter)["min_bless_val"].asInt();
      p->max_bless_val_ = (*iter)["max_bless_val"].asInt();
      p->total_bless_val_ = (*iter)["total_bless_val"].asInt();
      if (p->max_bless_val_ < p->min_bless_val_)
        return -1;
    }
    return 0;
  }
  const passive_skill_upgrade_cfg_obj *get_upgrade_info(const short lvl)
  { return this->upgrade_cfg_obj_map_.find(lvl); }
private:
  array_t<passive_skill_upgrade_cfg_obj *> upgrade_cfg_obj_map_;
};
passive_skill_upgrade_cfg::passive_skill_upgrade_cfg() : impl_(new passive_skill_upgrade_cfg_impl()) { }
int passive_skill_upgrade_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int passive_skill_upgrade_cfg::reload_config(const char *cfg_root)
{
  passive_skill_upgrade_cfg_impl *tmp_impl = new passive_skill_upgrade_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", PASSIVE_SKILL_UPGRADE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const passive_skill_upgrade_cfg_obj *passive_skill_upgrade_cfg::get_upgrade_info(const short lvl)
{ return this->impl_->get_upgrade_info(lvl); }
