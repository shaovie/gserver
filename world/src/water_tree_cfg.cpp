#include "water_tree_cfg.h"
#include "global_macros.h"
#include "item_config.h"
#include "load_json.h"
#include "def.h"
#include "array_t.h"
#include "sys_log.h"
#include "global_param_cfg.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

water_tree_obj::water_tree_obj()
  : exp_(0),
  attr_param_(0),
  cai_liao_cid_(0),
  cai_liao_cnt_(0),
  bind_diamond_(0)
{ }
water_tree_obj::~water_tree_obj()
{
  while (!this->random_award_.empty())
    delete this->random_award_.pop_front();
}

/**
 * @class water_tree_cfg_impl
 *
 * @brief
 */
class water_tree_cfg_impl : public load_json
{
public:
  water_tree_cfg_impl() :
    water_tree_obj_map_(global_param_cfg::guangai_lvl_limit + 1)
  { }
  ~water_tree_cfg_impl()
  {
    for (int i = 0; i < this->water_tree_obj_map_.size(); ++i)
    {
      water_tree_obj *wto = this->water_tree_obj_map_.find(i);
      if (wto != NULL) delete wto;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, WATER_TREE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int lvl = ::atoi(iter.key().asCString());
      if (lvl > global_param_cfg::guangai_lvl_limit)
        return -1;
      water_tree_obj *wto = new water_tree_obj();
      this->water_tree_obj_map_.insert(lvl, wto);

      wto->exp_           = (*iter)["exp"].asInt();
      wto->attr_param_    = (*iter)["attr_param"].asInt();
      wto->cai_liao_cid_  = (*iter)["item_cid"].asInt();
      wto->cai_liao_cnt_  = (*iter)["item_cnt"].asInt();
      wto->bind_diamond_  = (*iter)["bind_zuanshi"].asInt();
      wto->role_reward_.cid_    = (*iter)["role_reward_cid"].asInt();
      wto->role_reward_.amount_ = (*iter)["role_reward_cnt"].asInt();
      wto->role_reward_.bind_   = (*iter)["role_reward_bind"].asInt();

      char *tok_p = NULL;
      char *token = NULL;
      char bf[512] = {0};
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        water_tree_random_award *ra = new water_tree_random_award();
        wto->random_award_.push_back(ra);
        int bind = 0;
        if (::sscanf(token, "%d,%d,%d,%d", &ra->award_.cid_, &ra->award_.amount_, &bind, &ra->rate_) != 4)
          return -1;
        ra->award_.bind_ = bind;
        if (!item_config::instance()->find(ra->award_.cid_))
          return -1;
      }
    }

    return 0;
  }
  water_tree_obj *get_water_tree_obj(const int lvl)
  { return this->water_tree_obj_map_.find(lvl); }
private:
  array_t<water_tree_obj *> water_tree_obj_map_;
};
water_tree_cfg::water_tree_cfg() : impl_(new water_tree_cfg_impl()) { }
int water_tree_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int water_tree_cfg::reload_config(const char *cfg_root)
{
  water_tree_cfg_impl *tmp_impl = new water_tree_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", WATER_TREE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
water_tree_obj *water_tree_cfg::get_water_tree_obj(const int lvl)
{ return this->impl_->get_water_tree_obj(lvl); }

