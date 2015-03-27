#include "lvl_param_cfg.h"
#include "global_param_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header
#include <stdlib.h>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class lvl_param_cfg_impl
 *
 * @brief implement of lvl_param_cfg
 */
class lvl_param_cfg_impl : public load_json
{
public:
  class lvl_param_info
  {
  public:
    int     worship_coin_;
    int     guangai_coin_;
    int     guangai_exp_;
    int     rob_coin_;
    int     rob_exp_;
    double  attr_param_;
    int64_t lvl_up_exp_;
    int64_t worship_exp_;
    int64_t mission_exp_;
  };

  lvl_param_cfg_impl() : lvl_param_info_(NULL) { }
  ~lvl_param_cfg_impl()
  {
    if (this->lvl_param_info_ != NULL)
      delete []this->lvl_param_info_;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LVL_PARAM_CFG_PATH, root) != 0)
      return -1;

    this->lvl_param_info_ = new lvl_param_info[global_param_cfg::lvl_limit + 1];
    int max_lvl = 0;
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int lvl = ::atoi(iter.key().asCString());
      if (lvl > global_param_cfg::lvl_limit) continue;
      if (lvl > max_lvl) max_lvl = lvl;

      this->lvl_param_info_[lvl].lvl_up_exp_ = ::atol(((*iter)["exp"]).asCString());
      this->lvl_param_info_[lvl].attr_param_ = ((*iter)["attr_param"]).asDouble();
      this->lvl_param_info_[lvl].worship_exp_ =  ((*iter)["mobai_exp"]).asInt();
      this->lvl_param_info_[lvl].worship_coin_ = ((*iter)["mobai_coin"]).asInt();
      this->lvl_param_info_[lvl].guangai_exp_ =  ((*iter)["guangai_exp"]).asInt();
      this->lvl_param_info_[lvl].guangai_coin_ = ((*iter)["guangai_coin"]).asInt();
      this->lvl_param_info_[lvl].rob_exp_ = ((*iter)["rob_exp"]).asInt();
      this->lvl_param_info_[lvl].rob_coin_ = ((*iter)["rob_coin"]).asInt();
      this->lvl_param_info_[lvl].mission_exp_ = ((*iter)["mission_exp"]).asInt();
    }
    if (global_param_cfg::lvl_limit > max_lvl)
    {
      e_log->rinfo("max lvl %d is error for global lvl_limit %d",
                   max_lvl, global_param_cfg::lvl_limit);
      return -1;
    }

    return 0;
  }
  int64_t lvl_up_exp(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return -1;
    return this->lvl_param_info_[lvl].lvl_up_exp_;
  }
  double attr_param(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0.0;
    return this->lvl_param_info_[lvl].attr_param_;
  }
  int64_t worship_exp(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].worship_exp_;
  }
  int64_t mission_exp(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].mission_exp_;
  }
  int worship_coin(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].worship_coin_;
  }
  int guangai_exp(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].guangai_exp_;
  }
  int guangai_coin(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].guangai_coin_;
  }
  int rob_exp(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].rob_exp_;
  }
  int rob_coin(const short lvl)
  {
    if (lvl < 0 || lvl > global_param_cfg::lvl_limit)
      return 0;
    return this->lvl_param_info_[lvl].rob_coin_;
  }
  lvl_param_info *lvl_param_info_;
};
lvl_param_cfg::lvl_param_cfg() : impl_(new lvl_param_cfg_impl()) { }
int lvl_param_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int lvl_param_cfg::reload_config(const char *cfg_root)
{
  lvl_param_cfg_impl *tmp_impl = new lvl_param_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LVL_PARAM_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int64_t lvl_param_cfg::lvl_up_exp(const short lvl)
{ return this->impl_->lvl_up_exp(lvl); }
double lvl_param_cfg::attr_param(const short lvl)
{ return this->impl_->attr_param(lvl); }
int lvl_param_cfg::worship_coin(const short lvl)
{ return this->impl_->worship_coin(lvl); }
int64_t lvl_param_cfg::worship_exp(const short lvl)
{ return this->impl_->worship_exp(lvl); }
int64_t lvl_param_cfg::mission_exp(const short lvl)
{ return this->impl_->mission_exp(lvl); }
int lvl_param_cfg::guangai_exp(const short lvl)
{ return this->impl_->guangai_exp(lvl); }
int lvl_param_cfg::guangai_coin(const short lvl)
{ return this->impl_->guangai_coin(lvl); }
int lvl_param_cfg::rob_exp(const short lvl)
{ return this->impl_->rob_exp(lvl); }
int lvl_param_cfg::rob_coin(const short lvl)
{ return this->impl_->rob_coin(lvl); }
