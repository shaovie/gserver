#include "buy_ti_li_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

/**
 * @class buy_ti_li_cfg_impl
 *
 * @brief
 */
class buy_ti_li_cfg_impl : public load_json
{
public:
  buy_ti_li_cfg_impl() :
    max_vip_lvl_(0),
    obj_map_(MAX_VIP_LVL + 1)
  { }
  ~buy_ti_li_cfg_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      cfg_obj *obj = this->obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TI_LI_PURCHASE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int cnt = ::atoi(itor.key().asCString());
    }
    return 0;
  }
private:
  char max_vip_lvl_;
  array_t<cfg_obj *> obj_map_;
};
buy_ti_li_cfg::buy_ti_li_cfg() : impl_(new buy_ti_li_cfg_impl()) { }
int buy_ti_li_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int buy_ti_li_cfg::reload_config(const char *cfg_root)
{
  buy_ti_li_cfg_impl *tmp_impl = new buy_ti_li_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TI_LI_PURCHASE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int buy_ti_li_cfg::lvl_up_exp(const char lvl)
{ return this->impl_->lvl_up_exp(lvl); }
