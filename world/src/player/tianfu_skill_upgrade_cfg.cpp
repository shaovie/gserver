#include "tianfu_skill_upgrade_cfg.h"
#include "global_param_cfg.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class tianfu_skill_upgrade_cfg_impl
 * 
 * @brief implement of tianfu_skill_upgrade_cfg
 */
class tianfu_skill_upgrade_cfg_impl : public load_json
{
public:
  tianfu_skill_upgrade_cfg_impl() :
    upgrade_cfg_obj_map_(global_param_cfg::tianfu_lvl_limit + 1)
  { }
  ~tianfu_skill_upgrade_cfg_impl()
  {
    for (int i = 0; i < this->upgrade_cfg_obj_map_.size(); ++i)
    {
      tianfu_skill_upgrade_cfg_obj *p = this->upgrade_cfg_obj_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TIANFU_SKILL_UPGRADE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int lvl = ::atoi(iter.key().asCString());
      if (lvl > global_param_cfg::tianfu_lvl_limit)
        return -1;
      tianfu_skill_upgrade_cfg_obj *p = new tianfu_skill_upgrade_cfg_obj();
      this->upgrade_cfg_obj_map_.insert(lvl, p);

      p->cost_ = (*iter)["cost"].asInt();
      p->exp_  = (*iter)["exp"].asInt();
      p->item_cid_ = (*iter)["item_cid"].asInt();
      p->item_cnt_ = (*iter)["item_cnt"].asInt();
      p->item_exp_ = (*iter)["item_exp"].asInt();
    }
    return 0;
  }
  const tianfu_skill_upgrade_cfg_obj *get_upgrade_info(const short lvl)
  { return this->upgrade_cfg_obj_map_.find(lvl); }
private:
  array_t<tianfu_skill_upgrade_cfg_obj *> upgrade_cfg_obj_map_;
};
tianfu_skill_upgrade_cfg::tianfu_skill_upgrade_cfg() : impl_(new tianfu_skill_upgrade_cfg_impl()) { }
int tianfu_skill_upgrade_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int tianfu_skill_upgrade_cfg::reload_config(const char *cfg_root)
{
  tianfu_skill_upgrade_cfg_impl *tmp_impl = new tianfu_skill_upgrade_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TIANFU_SKILL_UPGRADE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const tianfu_skill_upgrade_cfg_obj *tianfu_skill_upgrade_cfg::get_upgrade_info(const short lvl)
{ return this->impl_->get_upgrade_info(lvl); }
