#include "dxc_config.h"
#include "load_json.h"
#include "item_config.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class dxc_config_impl
 *
 * @brief
 */
class dxc_config_impl : public load_json
{
public:
  typedef std::map<int/*scene_cid*/, dxc_cfg_obj *> dxc_cfg_map_t;
  typedef std::map<int/*scene_cid*/, dxc_cfg_obj *>::iterator dxc_cfg_map_itor;
  dxc_config_impl() { }
  ~dxc_config_impl()
  {
    for (dxc_cfg_map_itor itor = this->dxc_cfg_map_.begin();
         itor != this->dxc_cfg_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, DXC_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      dxc_cfg_obj *p = new dxc_cfg_obj();
      this->dxc_cfg_map_.insert(std::make_pair(scene_cid, p));
      if (this->load_json(p, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(dxc_cfg_obj *p, Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int grade = ::atoi(iter.key().asCString());
      if (grade > 0 && grade < DXC_END)
        p->zhanli_limit_[grade] = (*iter)["zhanli"].asInt();
      else return -1;
    }
    return 0;
  }
  const dxc_cfg_obj *get_dxc_cfg_obj(const int scene_cid)
  {
    dxc_cfg_map_itor itor = this->dxc_cfg_map_.find(scene_cid);
    if (itor == this->dxc_cfg_map_.end()) return NULL;
    return itor->second;
  }
private:
  dxc_cfg_map_t dxc_cfg_map_;
};
dxc_cfg_obj::dxc_cfg_obj()
{
  for (unsigned int i = 0;
       i < sizeof(this->zhanli_limit_)/sizeof(this->zhanli_limit_[0]);
       ++i)
    this->zhanli_limit_[i] = 9999999;
}
dxc_config::dxc_config() : impl_(new dxc_config_impl()) { }
int dxc_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int dxc_config::reload_config(const char *cfg_root)
{
  dxc_config_impl *tmp_impl = new dxc_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", DXC_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const dxc_cfg_obj *dxc_config::get_dxc_cfg_obj(const int scene_cid)
{ return this->impl_->get_dxc_cfg_obj(scene_cid); }
