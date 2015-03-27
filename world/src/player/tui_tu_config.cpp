#include "tui_tu_config.h"
#include "load_json.h"
#include "item_config.h"
#include "monster_drop_cfg.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class tui_tu_config_impl
 *
 * @brief
 */
class tui_tu_config_impl : public load_json
{
public:
  typedef std::map<int/*scene_cid*/, tui_tu_cfg_obj *> tui_tu_cfg_map_t;
  typedef std::map<int/*scene_cid*/, tui_tu_cfg_obj *>::iterator tui_tu_cfg_map_itor;
  tui_tu_config_impl() { }
  ~tui_tu_config_impl()
  {
    for (tui_tu_cfg_map_itor itor = this->tui_tu_cfg_map_.begin();
         itor != this->tui_tu_cfg_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TUI_TU_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      tui_tu_cfg_obj *p = new tui_tu_cfg_obj();
      p->scene_cid_ = scene_cid;
      this->tui_tu_cfg_map_.insert(std::make_pair(scene_cid, p));
      if (this->load_json(p, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(tui_tu_cfg_obj *p, Json::Value &root)
  {
    p->next_scene_cid_ = root["next_cid"].asInt();
    p->score_ = root["score"].asInt();
    p->limit_time_ = root["limit_time"].asInt();
    if (p->limit_time_ <= 0)
    {
      e_log->rinfo("limit time  <= 0");  // avoid  SIGFPE;
      return -1;
    }
    p->tili_cost_ = root["tili_cost"].asInt();
    p->first_exp_award_ = root["first_exp_award"].asInt();
    p->free_turn_cnt_ = root["free_turn_cnt"].asInt();
    p->turn_all_cost_ = root["turn_all_cost"].asInt();
    int bind = 0;
    if (::sscanf(root["first_award"].asCString(), "%d,%d,%d",
                 &(p->first_item_award_.cid_),
                 &(p->first_item_award_.amount_),
                 &bind) != 3)
      return -1;
    p->first_item_award_.bind_ = bind;
    if (!item_config::instance()->find(p->first_item_award_.cid_)
        || (!item_config::item_is_money(p->first_item_award_.cid_)
            && p->first_item_award_.amount_ > item_config::instance()->dj_upper_limit(p->first_item_award_.cid_)
           )
       )
      return -1;
    p->every_exp_award_ = root["every_exp"].asInt();
    p->boss_cid_ = root["boss_cid"].asInt();
    p->turn_award_ = root["turn_award"].asInt();
    p->enter_x_ = root["pos_x"].asInt();
    p->enter_y_ = root["pos_y"].asInt();
    return 0;
  }
  tui_tu_cfg_obj *get_tui_tu_cfg_obj(const int scene_cid)
  {
    tui_tu_cfg_map_itor itor = this->tui_tu_cfg_map_.find(scene_cid);
    if (itor == this->tui_tu_cfg_map_.end()) return NULL;
    return itor->second;
  }
  int ti_li_cost(const int scene_cid)
  {
    tui_tu_cfg_map_itor itor = this->tui_tu_cfg_map_.find(scene_cid);
    if (itor == this->tui_tu_cfg_map_.end()) return 0;
    return itor->second->tili_cost_;
  }
private:
  tui_tu_cfg_map_t tui_tu_cfg_map_;
};
tui_tu_config::tui_tu_config() : impl_(new tui_tu_config_impl()) { }
int tui_tu_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int tui_tu_config::reload_config(const char *cfg_root)
{
  tui_tu_config_impl *tmp_impl = new tui_tu_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TUI_TU_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
tui_tu_cfg_obj *tui_tu_config::get_tui_tu_cfg_obj(const int scene_cid)
{ return this->impl_->get_tui_tu_cfg_obj(scene_cid); }
int tui_tu_config::ti_li_cost(const int scene_cid)
{ return this->impl_->ti_li_cost(scene_cid); }
/**
 * @class tui_tu_chapter_config_impl
 *
 * @brief
 */
class tui_tu_chapter_config_impl : public load_json
{
public:
  typedef std::map<int/*idx*/, tui_tu_chapter_cfg_obj *> tui_tu_chapter_cfg_map_t;
  typedef std::map<int/*idx*/, tui_tu_chapter_cfg_obj *>::iterator tui_tu_chapter_cfg_map_itor;
  typedef std::map<int/*scene_cid*/, int/*idx*/> tui_tu_chapter_idx_map_t;
  typedef std::map<int/*scene_cid*/, int/*idx*/ >::iterator tui_tu_chapter_idx_map_itor;
  tui_tu_chapter_config_impl() { }
  ~tui_tu_chapter_config_impl()
  {
    for (tui_tu_chapter_cfg_map_itor itor = this->tui_tu_chapter_cfg_map_.begin();
         itor != this->tui_tu_chapter_cfg_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TUI_TU_CHAPTER_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int idx = ::atoi(iter.key().asCString());
      tui_tu_chapter_cfg_obj *p = new tui_tu_chapter_cfg_obj();
      this->tui_tu_chapter_cfg_map_.insert(std::make_pair(idx, p));
      if (this->load_json(p, *iter, idx) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(tui_tu_chapter_cfg_obj *p, Json::Value &root, const int idx)
  {
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = (*iter)["scene_cid"].asInt();
      p->scene_cid_list_.push_back(scene_cid);
      this->tui_tu_chapter_idx_map_.insert(std::make_pair(scene_cid, idx));
    }
    if (p->scene_cid_list_.empty())
      return -1;
    return 0;
  }
  tui_tu_chapter_cfg_obj *get_tui_tu_chapter_cfg_obj(const int idx)
  {
    tui_tu_chapter_cfg_map_itor itor = this->tui_tu_chapter_cfg_map_.find(idx);
    if (itor == this->tui_tu_chapter_cfg_map_.end()) return NULL;
    return itor->second;
  }
  int get_chapter_idx(const int scene_cid)
  {
    tui_tu_chapter_idx_map_itor itor = this->tui_tu_chapter_idx_map_.find(scene_cid);
    if (itor == this->tui_tu_chapter_idx_map_.end()) return -1;
    return itor->second;
  }
private:
  tui_tu_chapter_cfg_map_t tui_tu_chapter_cfg_map_;
  tui_tu_chapter_idx_map_t tui_tu_chapter_idx_map_;
};
tui_tu_chapter_config::tui_tu_chapter_config() : impl_(new tui_tu_chapter_config_impl()) { }
int tui_tu_chapter_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int tui_tu_chapter_config::reload_config(const char *cfg_root)
{
  tui_tu_chapter_config_impl *tmp_impl = new tui_tu_chapter_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TUI_TU_CHAPTER_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
tui_tu_chapter_cfg_obj *tui_tu_chapter_config::get_tui_tu_chapter_cfg_obj(const int idx)
{ return this->impl_->get_tui_tu_chapter_cfg_obj(idx); }
int tui_tu_chapter_config::get_chapter_idx(const int scene_cid)
{ return this->impl_->get_chapter_idx(scene_cid); }
/**
 * @class tui_tu_jy_chapter_config_impl
 *
 * @brief
 */
class tui_tu_jy_chapter_config_impl : public load_json
{
public:
  typedef std::map<int/*idx*/, tui_tu_chapter_cfg_obj *> tui_tu_chapter_cfg_map_t;
  typedef std::map<int/*idx*/, tui_tu_chapter_cfg_obj *>::iterator tui_tu_chapter_cfg_map_itor;
  typedef std::map<int/*scene_cid*/, int/*idx*/> tui_tu_chapter_idx_map_t;
  typedef std::map<int/*scene_cid*/, int/*idx*/ >::iterator tui_tu_chapter_idx_map_itor;
  tui_tu_jy_chapter_config_impl() { }
  ~tui_tu_jy_chapter_config_impl()
  {
    for (tui_tu_chapter_cfg_map_itor itor = this->tui_tu_chapter_cfg_map_.begin();
         itor != this->tui_tu_chapter_cfg_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TUI_TU_JY_CHAPTER_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int idx = ::atoi(iter.key().asCString());
      tui_tu_chapter_cfg_obj *p = new tui_tu_chapter_cfg_obj();
      this->tui_tu_chapter_cfg_map_.insert(std::make_pair(idx, p));
      if (this->load_json(p, *iter, idx) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(tui_tu_chapter_cfg_obj *p, Json::Value &root, const int idx)
  {
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = (*iter)["scene_cid"].asInt();
      p->scene_cid_list_.push_back(scene_cid);
      this->tui_tu_chapter_idx_map_.insert(std::make_pair(scene_cid, idx));
    }
    if (p->scene_cid_list_.empty())
      return -1;
    return 0;
  }
  tui_tu_chapter_cfg_obj *get_tui_tu_chapter_cfg_obj(const int idx)
  {
    tui_tu_chapter_cfg_map_itor itor = this->tui_tu_chapter_cfg_map_.find(idx);
    if (itor == this->tui_tu_chapter_cfg_map_.end()) return NULL;
    return itor->second;
  }
  int get_chapter_idx(const int scene_cid)
  {
    tui_tu_chapter_idx_map_itor itor = this->tui_tu_chapter_idx_map_.find(scene_cid);
    if (itor == this->tui_tu_chapter_idx_map_.end()) return -1;
    return itor->second;
  }
private:
  tui_tu_chapter_cfg_map_t tui_tu_chapter_cfg_map_;
  tui_tu_chapter_idx_map_t tui_tu_chapter_idx_map_;
};
tui_tu_jy_chapter_config::tui_tu_jy_chapter_config() : impl_(new tui_tu_jy_chapter_config_impl()) { }
int tui_tu_jy_chapter_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int tui_tu_jy_chapter_config::reload_config(const char *cfg_root)
{
  tui_tu_jy_chapter_config_impl *tmp_impl = new tui_tu_jy_chapter_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TUI_TU_JY_CHAPTER_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
tui_tu_chapter_cfg_obj *tui_tu_jy_chapter_config::get_tui_tu_chapter_cfg_obj(const int idx)
{ return this->impl_->get_tui_tu_chapter_cfg_obj(idx); }
int tui_tu_jy_chapter_config::get_chapter_idx(const int scene_cid)
{ return this->impl_->get_chapter_idx(scene_cid); }
/**
 * @class tui_tu_turn_config_impl
 *
 * @brief
 */
class tui_tu_turn_config_impl : public load_json
{
public:
  typedef std::map<int/*scene_cid*/, tui_tu_turn_cfg_obj *> tui_tu_turn_cfg_map_t;
  typedef std::map<int/*scene_cid*/, tui_tu_turn_cfg_obj *>::iterator tui_tu_turn_cfg_map_itor;
  tui_tu_turn_config_impl() { }
  ~tui_tu_turn_config_impl()
  {
    for (tui_tu_turn_cfg_map_itor itor = this->tui_tu_turn_cfg_map_.begin();
         itor != this->tui_tu_turn_cfg_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TUI_TU_TURN_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      tui_tu_turn_cfg_obj *p = new tui_tu_turn_cfg_obj();
      p->scene_cid_ = scene_cid;
      this->tui_tu_turn_cfg_map_.insert(std::make_pair(scene_cid, p));
      if (this->load_json(p, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(tui_tu_turn_cfg_obj *p, Json::Value &root)
  {
    int r = 0;
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      pair_t<int> v;
      v.first_  = (*iter)["box_p"].asInt();
      r += v.first_;
      v.second_ = (*iter)["box_id"].asInt();
      p->box_list_.push_back(v);
      if (box_item_cfg::instance()->get_box_item_obj(v.second_) == NULL)
      {
        e_log->rinfo("box %d not found!", v.second_);
        return -1;
      }
    }
    if (p->box_list_.size() > MAX_TUI_TU_TURN_ITEM_AMOUNT
       || r != 10000)
    {
      e_log->rinfo("tui tu turn %d box invalid!", p->scene_cid_);
      return -1;
    }
    return 0;
  }
  tui_tu_turn_cfg_obj *get_tui_tu_turn_cfg_obj(const int scene_cid)
  {
    tui_tu_turn_cfg_map_itor itor = this->tui_tu_turn_cfg_map_.find(scene_cid);
    if (itor == this->tui_tu_turn_cfg_map_.end()) return NULL;
    return itor->second;
  }
private:
  tui_tu_turn_cfg_map_t tui_tu_turn_cfg_map_;
};
tui_tu_turn_config::tui_tu_turn_config() : impl_(new tui_tu_turn_config_impl()) { }
int tui_tu_turn_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int tui_tu_turn_config::reload_config(const char *cfg_root)
{
  tui_tu_turn_config_impl *tmp_impl = new tui_tu_turn_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TUI_TU_TURN_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
tui_tu_turn_cfg_obj *tui_tu_turn_config::get_tui_tu_turn_cfg_obj(const int scene_cid)
{ return this->impl_->get_tui_tu_turn_cfg_obj(scene_cid); }
