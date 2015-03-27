#include "strength_addition_cfg.h"
#include "global_param_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

sa_attr::sa_attr()
{
  ::memset(this->attr_val_, 0, sizeof(this->attr_val_));
}
/**
 * @class strength_addition_cfg_impl
 *
 * @brief
 */
class strength_addition_cfg_impl : public load_json
{
public:
  strength_addition_cfg_impl() :
    sa_attr_list_(global_param_cfg::lvl_limit + 1)
  { }
  ~strength_addition_cfg_impl()
  {
    for (int i = 0; i < this->sa_attr_list_.size(); ++i)
      delete this->sa_attr_list_.find(i);
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, STRENGTH_ADDITION_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int count = ::atoi(iter.key().asCString());
      if (count > global_param_cfg::lvl_limit) return -1;
      sa_attr *saa = new sa_attr();
      this->sa_attr_list_.insert(count, saa);

      saa->attr_val_[ATTR_T_HP] = (*iter)["sheng_ming"].asInt();
      saa->attr_val_[ATTR_T_GONG_JI] = (*iter)["gong_ji"].asInt();
      saa->attr_val_[ATTR_T_FANG_YU] = (*iter)["fang_yu"].asInt();
      saa->attr_val_[ATTR_T_SHANG_MIAN] = (*iter)["shang_mian"].asInt();
    }
    return 0;
  }
  sa_attr *get_sa_attr(const int count)
  {
    for (int i = count; i > 0; --i)
    {
      sa_attr *saa = this->sa_attr_list_.find(i);
      if (saa != NULL) return saa;
    }
    return NULL;
  }
private:
  array_t<sa_attr *> sa_attr_list_;
};
strength_addition_cfg::strength_addition_cfg() :
  impl_(new strength_addition_cfg_impl())
{ }
int strength_addition_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int strength_addition_cfg::reload_config(const char *cfg_root)
{
  strength_addition_cfg_impl *tmp_impl = new strength_addition_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", STRENGTH_ADDITION_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
sa_attr *strength_addition_cfg::get_sa_attr(const int count)
{ return this->impl_->get_sa_attr(count); }

