#include "ltime_act_cfg.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

class sub_recharge_cfg_list
{
public:
  ~sub_recharge_cfg_list()
  {
    while (!this->l_.empty())
      delete this->l_.pop_front();
  }
  ilist<ltime_recharge_cfg_obj *> l_;
};
/**
 * @class ltime_recharge_cfg_impl
 *
 * @brief implement of ltime_recharge_cfg
 */
class ltime_recharge_cfg_impl : public load_json
{
public:
  typedef std::map<int/*act id*/, sub_recharge_cfg_list *> recharge_cfg_map_t;
  typedef std::map<int/*act id*/, sub_recharge_cfg_list *>::iterator recharge_cfg_map_itor;

  ltime_recharge_cfg_impl() { }
  ~ltime_recharge_cfg_impl()
  {
    for (recharge_cfg_map_itor itor = this->recharge_cfg_map_.begin();
         itor != this->recharge_cfg_map_.end();
         ++itor)
      delete itor->second;
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LTIME_RECHARGE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int act_id = ::atoi(iter.key().asCString());
      sub_recharge_cfg_list *pl = new sub_recharge_cfg_list();
      this->recharge_cfg_map_.insert(std::make_pair(act_id, pl));
      Json::Value &sub_v = *iter;
      for (Json::Value::iterator sub_iter = sub_v.begin();
           sub_iter != sub_v.end();
           ++sub_iter)
      {
        ltime_recharge_cfg_obj *p = new ltime_recharge_cfg_obj();
        pl->l_.push_back(p);
        p->sub_id_ = ::atoi(sub_iter.key().asCString());
        p->param_1_ = (*sub_iter)["param1"].asInt();
        Json::Value &award_v = (*sub_iter)["award"];
        if (award_v.empty()) continue;
        char *tok_p = NULL;
        char *token = NULL;
        char bf[256] = {0};
        ::strncpy(bf, award_v.asCString(), sizeof(bf) - 1);
        for (token = ::strtok_r(bf, ";", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ";", &tok_p))
        {
          item_amount_bind_t item;
          int bind = 0;
          ::sscanf(token, "%d,%d,%d", &item.cid_, &item.amount_, &bind);
          item.bind_ = bind;
          p->award_list_.push_back(item);
          if (!item_config::instance()->find(item.cid_))
            return -1;
        }
      }
    }
    return 0;
  }
  ilist<ltime_recharge_cfg_obj *> *get_recharge_cfg(const int id)
  {
    recharge_cfg_map_itor itor = this->recharge_cfg_map_.find(id);
    if (itor == this->recharge_cfg_map_.end())
      return NULL;
    return &itor->second->l_;
  }
private:
  recharge_cfg_map_t recharge_cfg_map_;
};
// -------------------------------------------------------------------------
ltime_recharge_cfg::ltime_recharge_cfg() : impl_(new ltime_recharge_cfg_impl()) { }
int ltime_recharge_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int ltime_recharge_cfg::reload_config(const char *cfg_root)
{
  ltime_recharge_cfg_impl *tmp_impl = new ltime_recharge_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LTIME_RECHARGE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<ltime_recharge_cfg_obj *> *ltime_recharge_cfg::get_recharge_cfg(const int id)
{ return this->impl_->get_recharge_cfg(id); }
