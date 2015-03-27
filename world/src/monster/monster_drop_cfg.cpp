#include "monster_drop_cfg.h"
#include "load_json.h"
#include "item_config.h"
#include "sys_log.h"
#include "date_time.h"

// Lib header
#include <tr1/unordered_map>

// monster drop config log
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class monster_drop_cfg_impl
 *
 * @brief
 */
class monster_drop_cfg_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, monster_drop_obj *> mst_drop_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, monster_drop_obj *>::iterator mst_drop_obj_map_itor;

  monster_drop_cfg_impl() : mst_drop_obj_map_(2048) { }
  ~monster_drop_cfg_impl()
  {
    for (mst_drop_obj_map_itor itor = this->mst_drop_obj_map_.begin();
         itor != this->mst_drop_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, MONSTER_DROP_CFG_PATH, root) != 0)
      return -1;
    
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      monster_drop_obj *mdo = new monster_drop_obj();
      this->mst_drop_obj_map_.insert(std::make_pair(cid, mdo));
      if (this->load_json(mdo, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(monster_drop_obj *mdo, Json::Value &root)
  {
    for(Json::Value::iterator b_iter = root.begin();
        b_iter != root.end();
        ++b_iter)
    {
      monster_drop_obj::_drop_box *mdb = new monster_drop_obj::_drop_box();
      mdo->drop_box_list_.push_back(mdb);
      if (this->load_drop_box(mdb, *b_iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_drop_box(monster_drop_obj::_drop_box *mdb, Json::Value &root)
  {
    if (root.empty()) return -1;

    mdb->box_cid_ = root["box_cid"].asInt();
    mdb->box_rate_ = root["box_rate"].asInt();
    Json::Value &time_limit_v = root["time_limit"];
    if (!time_limit_v.empty())
    {
      char bf [1024] = {0};
      ::strncpy(bf, time_limit_v.asCString(), sizeof(bf));

      char *tok_p = NULL;
      char *token = NULL;
      int cnt = 0;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        if (++cnt > 5)
        {
          e_log->error("drop box %d config too much time limit!", mdb->box_cid_);
          return -1;
        }
        char *p = ::strchr(token, '~');
        if (p == NULL) return -1;
        *p = '\0';
        p += 1;
        date_time dt;
        pair_t<int> time_pair;
        time_pair.first_ = dt.str_to_datetime(token);
        time_pair.second_ = dt.str_to_datetime(p);
        mdb->limit_time_list_.push_back(time_pair);
      }
    }
    return 0;
  }
  monster_drop_obj *get_monster_drop_obj(const int cid)
  {
    mst_drop_obj_map_itor itor = this->mst_drop_obj_map_.find(cid);
    if (itor == this->mst_drop_obj_map_.end()) return NULL;
    return itor->second;
  }
private:
  mst_drop_obj_map_t mst_drop_obj_map_;
};
monster_drop_cfg::monster_drop_cfg() : impl_(new monster_drop_cfg_impl()) { }
int monster_drop_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int monster_drop_cfg::reload_config(const char *cfg_root)
{
  monster_drop_cfg_impl *tmp_impl = new monster_drop_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", MONSTER_DROP_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const monster_drop_obj* monster_drop_cfg::get_monster_drop_obj(const int cid)
{ return this->impl_->get_monster_drop_obj(cid); }
/**
 * @class box_item_cfg_impl
 *
 * @brief
 */
class box_item_cfg_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, box_item_obj *> box_item_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, box_item_obj *>::iterator box_item_obj_map_itor;

  box_item_cfg_impl() : box_item_obj_map_(2048) { }
  ~box_item_cfg_impl()
  {
    for (box_item_obj_map_itor itor = this->box_item_obj_map_.begin();
         itor != this->box_item_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, DROP_BOX_CFG_PATH, root) != 0)
      return -1;
    
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int box = ::atoi(iter.key().asCString());
      box_item_obj *bio = new box_item_obj();
      this->box_item_obj_map_.insert(std::make_pair(box, bio));
      if (this->load_json(bio, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(box_item_obj *bio, Json::Value &r)
  {
    bio->box_type_ = r["box_type"].asInt();

    Json::Value &root = r["info"];
    for (Json::Value::iterator i_iter = root.begin();
         i_iter != root.end();
         ++i_iter)
    {
      Json::Value &item_v = *i_iter;
      box_item_obj::_drop_item *di = new box_item_obj::_drop_item();
      bio->drop_item_list_.push_back(di);

      di->item_cid_ = item_v["item_cid"].asInt();
      if (!item_config::instance()->find(di->item_cid_))
      {
        e_log->error("drop item %d not found in item config!", di->item_cid_);
        return -1;
      }
      di->min_cnt_ = item_v["min_cnt"].asInt();
      di->max_cnt_ = item_v["max_cnt"].asInt();
      if (di->max_cnt_ < di->min_cnt_)
      {
        e_log->rinfo("%d max cnt %d:%d is invalid!", di->item_cid_,
                     di->max_cnt_, di->min_cnt_); 
        return -1;
      }
      di->bind_type_ = item_v["bind_type"].asInt();
      di->pick_notice_ = item_v["pick_notify"].asBool();
      di->drop_rate_ = item_v["drop_rate"].asInt();
    }
    return 0;
  }
  box_item_obj *get_box_item_obj(const int cid)
  {
    box_item_obj_map_itor itor = this->box_item_obj_map_.find(cid);
    if (itor == this->box_item_obj_map_.end()) return NULL;
    return itor->second;
  }
private:
  box_item_obj_map_t box_item_obj_map_;
};
box_item_cfg::box_item_cfg() : impl_(new box_item_cfg_impl()) { }
int box_item_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int box_item_cfg::reload_config(const char *cfg_root)
{
  box_item_cfg_impl *tmp_impl = new box_item_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", DROP_BOX_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const box_item_obj* box_item_cfg::get_box_item_obj(const int cid)
{ return this->impl_->get_box_item_obj(cid); }
