#include "activity_cfg.h"
#include "scene_config.h"
#include "ghz_module.h"
#include "xszc_activity_obj.h"
#include "award_ti_li_act.h"
#include "kai_fu_act_obj.h"
#include "array_t.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class activity_cfg_impl
 *
 * @brief implement of activity_cfg
 */
class activity_cfg_impl : public load_json
{
public:
  activity_cfg_impl() : act_cfg_obj_map_(ACTIVITY_CNT) { }
  ~activity_cfg_impl()
  {
    int s = this->act_cfg_obj_map_.size();
    for (int i = 0; i < s; ++i)
    {
      activity_cfg_obj *aco = this->act_cfg_obj_map_.find(i);
      if (aco != NULL) delete aco;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, ACTIVITY_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (id < 0 || id >= ACTIVITY_CNT)
      {
        e_log->error("activity id %d is invalid %s!", id, ACTIVITY_CFG_PATH);
        return -1;
      }
      activity_cfg_obj *aco = new activity_cfg_obj();
      this->act_cfg_obj_list_.push_back(aco);
      this->act_cfg_obj_map_.insert(id, aco);
      aco->act_id_ = id;
      if (this->load_json(*iter, aco) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(Json::Value &root, activity_cfg_obj *aco)
  {
    aco->is_valid_ = root["is_valid"].asBool();
    aco->lvl_ = root["lvl"].asInt();
    Json::Value &week_v = root["week"];
    if (!week_v.empty())
    {
      char *tok_p = NULL;
      char *token = NULL;
      char bf[256] = {0};
      ::strncpy(bf, week_v.asCString(), sizeof(bf) - 1);
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        int w = ::atoi(token);
        if (w > 7 || w < 1)
        {
          e_log->error("activity %d week %d is invalid!", aco->act_id_, w);
          return -1;
        }
        aco->week_list_.push_back(w);
      }
    }
    if (aco->week_list_.empty())
    {
      e_log->error("activity %d week is empty!", aco->act_id_);
      return -1;
    }
    Json::Value &time_v = root["time"];
    if (!time_v.empty())
    {
      char *tok_p = NULL;
      char *token = NULL;
      char bf[256] = {0};
      ::strncpy(bf, time_v.asCString(), sizeof(bf) - 1);
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        char *p = ::strchr(token, '-');
        if (p == NULL)
        {
          e_log->error("activity %d time [%s] is invalid",
                       aco->act_id_,
                       token);
          return -1;
        }
        *p = '\0';
        p += 1;
        itime f_it;
        ::sscanf(token, "%d:%d", &f_it.hour_, &f_it.min_);
        itime s_it;
        ::sscanf(p, "%d:%d", &s_it.hour_, &s_it.min_);
        aco->time_list_.push_back(pair_t<itime>(f_it, s_it));
      }
    }
    if (aco->time_list_.empty())
    {
      e_log->error("activity %d time is empty!", aco->act_id_);
      return -1;
    }
    Json::Value &value_v = root["value"];
    if (!value_v.empty())
    {
      if (aco->act_id_ == ACTIVITY_GHZ)
      {
        if (ghz_module::parse_cfg_param(value_v.asCString()) != 0)
          return -1;
      }else if (aco->act_id_ == ACTIVITY_AWARD_TI_LI)
      {
        if (award_ti_li_act::parse_cfg_param(value_v.asCString()) != 0)
          return -1;
      }else if (aco->act_id_ == ACTIVITY_XSZC)
      {
        if (xszc_activity_obj::parse_cfg_param(value_v.asCString()) != 0)
          return -1;
      }else if (aco->act_id_ == ACTIVITY_KAI_FU)
      {
        if (kai_fu_act_obj::parse_cfg_param(value_v.asCString()) != 0)
          return -1;
      }
    }
    return 0;
  }
  activity_cfg_obj *get_act_cfg_obj(const int activity_id)
  { return this->act_cfg_obj_map_.find(activity_id); }

  int open_lvl(const int act_id)
  {
    activity_cfg_obj *aco = this->act_cfg_obj_map_.find(act_id);
    if (aco == NULL) return 1;
    return aco->lvl_;
  }
  ilist<activity_cfg_obj *> *get_act_cfg_obj_list()
  { return &(this->act_cfg_obj_list_); }
private:
  ilist<activity_cfg_obj *> act_cfg_obj_list_;
  array_t<activity_cfg_obj *> act_cfg_obj_map_;
};
// -------------------------------------------------------------------------
activity_cfg::activity_cfg() : impl_(new activity_cfg_impl()) { }
int activity_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int activity_cfg::reload_config(const char *cfg_root)
{
  activity_cfg_impl *tmp_impl = new activity_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", ACTIVITY_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
activity_cfg_obj *activity_cfg::get_act_cfg_obj(const int act_id)
{ return this->impl_->get_act_cfg_obj(act_id); }
int activity_cfg::open_lvl(const int act_id)
{ return this->impl_->open_lvl(act_id); }
ilist<activity_cfg_obj *> *activity_cfg::get_act_cfg_obj_list()
{ return this->impl_->get_act_cfg_obj_list(); }
