#include "scp_config.h"
#include "global_macros.h"
#include "load_json.h"
#include "scene_config.h"
#include "sys_log.h"
#include "clsid.h"

// Lib header
#include <map>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

scp_cfg_obj::~scp_cfg_obj()
{
  while (!this->sub_scp_list_.empty())
  {
    sub_scp_cfg_obj *ssco = this->sub_scp_list_.pop_front();
    delete ssco;
  }
}
/**
 * @class scp_config_impl
 * 
 * @brief implement of scp_config
 */
class scp_config_impl : public load_json
{
public:
  typedef std::map<int, scp_cfg_obj *> scp_cfg_obj_map_t;
  typedef std::map<int, scp_cfg_obj *>::iterator scp_cfg_obj_map_itor;
  scp_config_impl() { }
  ~scp_config_impl()
  {
    for (scp_cfg_obj_map_itor itor = this->scp_cfg_obj_map_.begin();
         itor != this->scp_cfg_obj_map_.end();
         ++itor)
    { delete itor->second; }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SCP_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      if (!clsid::is_scp_scene(scene_cid))
      {
        e_log->wning("scp %d scene cid is error!", scene_cid);
        continue;
      }
      scp_cfg_obj *sco = new scp_cfg_obj();
      this->scp_cfg_obj_map_.insert(std::make_pair(scene_cid, sco));
      sco->scene_cid_ = scene_cid;
      if (this->load_json(*iter, sco) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(Json::Value &root, scp_cfg_obj *sco)
  {
    sco->scp_type_ = root["scp_type"].asInt();
    sco->control_type_ = root["control_type"].asInt();
    sco->is_valid_ = root["is_valid"].asBool();
    sco->enter_cnt_ = root["enter_cnt"].asInt();

    Json::Value &info_v = root["info"];
    if (info_v.empty())
    {
      e_log->error("scp %d not info", sco->scene_cid_);
      return -1;
    }
    for (Json::Value::iterator iter = info_v.begin();
         iter != info_v.end();
         ++iter)
    {
      sub_scp_cfg_obj *ssco = new sub_scp_cfg_obj();
      ssco->scene_cid_ = (*iter)["scene_cid"].asInt();
      ssco->enter_x_   = (*iter)["pos_x"].asInt();
      ssco->enter_y_   = (*iter)["pos_y"].asInt();
      ssco->last_time_ = (*iter)["last_time"].asInt();
      sco->sub_scp_list_.push_back(ssco);
      if (!scene_config::instance()->can_move(ssco->scene_cid_,
                                              ssco->enter_x_,
                                              ssco->enter_y_))
      {
        e_log->error("scp %d enter pos is can not move!", ssco->scene_cid_);
        return -1;
      }
    }
    if (sco->sub_scp_list_.empty())
    {
      e_log->error("scp %d no sub scp!", sco->scene_cid_);
      return -1;
    }
    return 0;
  }
  scp_cfg_obj *get_scp_cfg_obj(const int scene_cid)
  {
    scp_cfg_obj_map_itor itor = this->scp_cfg_obj_map_.find(scene_cid);
    if (itor == this->scp_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
  coord_t get_enter_pos(const int scene_cid)
  {
    for (scp_cfg_obj_map_itor itor = this->scp_cfg_obj_map_.begin();
         itor != this->scp_cfg_obj_map_.end();
         ++itor)
    {
      scp_cfg_obj *sco = itor->second;
      for (ilist_node<sub_scp_cfg_obj *> *sub_itor = sco->sub_scp_list_.head();
           sub_itor != NULL;
           sub_itor = sub_itor->next_)
      {
        if (sub_itor->value_->scene_cid_ == scene_cid)
          return coord_t(sub_itor->value_->enter_x_, sub_itor->value_->enter_y_);
      }
    }
    return coord_t(-1, -1);
  }
  int get_scp_enter_times(const int scene_cid)
  {
    scp_cfg_obj *sco = this->get_scp_cfg_obj(scene_cid);
    if (sco == NULL) return 0;
    return sco->enter_cnt_;
  }
  int control_type(const int scene_cid)
  {
    scp_cfg_obj *sco = this->get_scp_cfg_obj(scene_cid);
    if (sco == NULL) return scp_cfg_obj::SCP_CLT_IMPL;
    return sco->control_type_;
  }
  int scp_type(const int scene_cid)
  {
    scp_cfg_obj *sco = this->get_scp_cfg_obj(scene_cid);
    if (sco == NULL) return -1;
    return sco->scp_type_;
  }
  int last_time(const int master_cid, const int scene_cid)
  {
    scp_cfg_obj *sco = this->get_scp_cfg_obj(master_cid);
    if (sco == NULL) return -1;
    for (ilist_node<sub_scp_cfg_obj *> *sub_itor = sco->sub_scp_list_.head();
         sub_itor != NULL;
         sub_itor = sub_itor->next_)
    {
      if (sub_itor->value_->scene_cid_ == scene_cid)
        return sub_itor->value_->last_time_;
    }
    return -1;
  }
private:
  scp_cfg_obj_map_t scp_cfg_obj_map_;
};
// -------------------------------------------------------------------------
scp_config::scp_config() : impl_(new scp_config_impl()) { }
int scp_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int scp_config::reload_config(const char *cfg_root)
{
  scp_config_impl *tmp_impl = new scp_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SCP_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
scp_cfg_obj *scp_config::get_scp_cfg_obj(const int scene_cid)
{ return this->impl_->get_scp_cfg_obj(scene_cid); }
coord_t scp_config::get_enter_pos(const int scene_cid)
{ return this->impl_->get_enter_pos(scene_cid); }
int scp_config::get_scp_enter_times(const int scene_cid)
{ return this->impl_->get_scp_enter_times(scene_cid); }
int scp_config::control_type(const int scene_cid)
{ return this->impl_->control_type(scene_cid); }
int scp_config::scp_type(const int scene_cid)
{ return this->impl_->scp_type(scene_cid); }
int scp_config::last_time(const int master_cid, const int scene_cid)
{ return this->impl_->last_time(master_cid, scene_cid); }
