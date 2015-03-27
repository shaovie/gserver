#include "recharge_config.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class recharge_config_impl
 *
 * @brief
 */
class recharge_config_impl : public load_json
{
public:
  recharge_config_impl() :
    recharge_cfg_obj_arr_(MAX_RC_TYPE_CNT + 1)
  { }
  ~recharge_config_impl()
  {
    int size = this->recharge_cfg_obj_arr_.size();
    for (int i = 0; i < size; ++i)
      delete this->recharge_cfg_obj_arr_.find(i);
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, RECHARGE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      recharge_cfg_obj *ico = new recharge_cfg_obj();
      if (this->recharge_cfg_obj_arr_.insert(id, ico) != 0)
        return -1;
      ico->diamond_ = (*iter)["diamond"].asInt();
      ico->b_diamond_1_ = (*iter)["bind_diamond_1"].asInt();
      ico->limit_cnt_ = (*iter)["limit_cnt"].asInt();
      ico->b_diamond_2_ = (*iter)["bind_diamond_2"].asInt();
      ico->charge_rmb_ = (*iter)["charge_rmb"].asInt();
      if (ico->diamond_ < 0 
          || ico->diamond_ > MONEY_UPPER_LIMIT
          || ico->b_diamond_1_ < 0
          || ico->b_diamond_1_ > MONEY_UPPER_LIMIT
          || ico->b_diamond_2_ < 0
          || ico->b_diamond_2_ > MONEY_UPPER_LIMIT)
        return -1;
    }
    return 0;
  }
  recharge_cfg_obj *get_recharge_obj(const int recharge_id)
  { return this->recharge_cfg_obj_arr_.find(recharge_id); }
private:
  array_t<recharge_cfg_obj *> recharge_cfg_obj_arr_;
};
// ------------------------------- recharge config ------------------------------
recharge_config::recharge_config()
: impl_(new recharge_config_impl())
{ }
int recharge_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int recharge_config::reload_config(const char *cfg_root)
{
  recharge_config_impl *tmp_impl = new recharge_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", RECHARGE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
recharge_cfg_obj *recharge_config::get_recharge_obj(const int recharge_cid)
{ return this->impl_->get_recharge_obj(recharge_cid); }
