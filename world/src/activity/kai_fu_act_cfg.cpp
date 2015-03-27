#include "kai_fu_act_cfg.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class kai_fu_act_cfg_impl
 *
 * @brief implement of kai_fu_act_cfg
 */
class kai_fu_act_cfg_impl : public load_json
{
public:
  typedef std::map<int/*act id*/, ilist<kai_fu_act_cfg_obj *> *> kai_fu_act_cfg_map_t;
  typedef std::map<int/*act id*/, ilist<kai_fu_act_cfg_obj *> *>::iterator kai_fu_act_cfg_map_itor;

  kai_fu_act_cfg_impl() { }
  ~kai_fu_act_cfg_impl()
  {
    for (kai_fu_act_cfg_map_itor itor = this->kai_fu_act_cfg_map_.begin();
         itor != this->kai_fu_act_cfg_map_.end();
         ++itor)
    {
      while (!itor->second->empty())
        delete itor->second->pop_front();
      delete itor->second;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, KAI_FU_ACT_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int act_id = ::atoi(iter.key().asCString());
      ilist<kai_fu_act_cfg_obj *> *pl = new ilist<kai_fu_act_cfg_obj *>();
      this->kai_fu_act_cfg_map_.insert(std::make_pair(act_id, pl));
      Json::Value &sub_v = *iter;
      for (Json::Value::iterator sub_iter = sub_v.begin();
           sub_iter != sub_v.end();
           ++sub_iter)
      {
        kai_fu_act_cfg_obj *p = new kai_fu_act_cfg_obj();
        pl->push_back(p);
        p->param_ = (*sub_iter)["param"].asInt();
        p->open_ = (*sub_iter)["open"].asBool();

        {
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
        {
          Json::Value &award_v = (*sub_iter)["award_2"];
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
            p->award_list_2_.push_back(item);
            if (!item_config::instance()->find(item.cid_))
              return -1;
          }
        }
      }
    }
    return 0;
  }
  ilist<kai_fu_act_cfg_obj *> *get_cfg(const int id)
  {
    kai_fu_act_cfg_map_itor itor = this->kai_fu_act_cfg_map_.find(id);
    if (itor == this->kai_fu_act_cfg_map_.end()) return NULL;
    return itor->second;
  }
private:
  kai_fu_act_cfg_map_t kai_fu_act_cfg_map_;
};
// -------------------------------------------------------------------------
kai_fu_act_cfg::kai_fu_act_cfg() : impl_(new kai_fu_act_cfg_impl()) { }
int kai_fu_act_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int kai_fu_act_cfg::reload_config(const char *cfg_root)
{
  kai_fu_act_cfg_impl *tmp_impl = new kai_fu_act_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", KAI_FU_ACT_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<kai_fu_act_cfg_obj *> *kai_fu_act_cfg::get_cfg(const int id)
{ return this->impl_->get_cfg(id); }
