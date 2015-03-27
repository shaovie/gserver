#include "gift_config.h"
#include "load_json.h"
#include "global_macros.h"
#include "sys_log.h"
#include "item_config.h"
#include "ilist.h"
#include "array_t.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class gift_config_impl
 *
 * @brief
 */
class gift_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, gift_obj *> gift_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, gift_obj *>::iterator gift_obj_map_itor;

  gift_config_impl() :
    gift_obj_map_(256)
  { }
  ~gift_config_impl()
  {
    for (gift_obj_map_itor itor = this->gift_obj_map_.begin();
         itor != this->gift_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GIFT_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      gift_obj *go = new gift_obj();
      this->gift_obj_map_.insert(std::make_pair(cid, go));
      if (this->load_json(*iter, go) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(Json::Value &root, gift_obj *go)
  {
    Json::Value &info = root["info"];
    for (Json::Value::iterator iter = info.begin();
         iter != info.end();
         ++iter)
    {
      Json::Value &v = *iter;
      gift_info *gi = new gift_info();
      go->gift_list_.push_back(gi);
      gi->bind_ = v["bind"].asInt();
      gi->item_cid_ = v["item_cid"].asInt();
      gi->min_amt_ = v["min_item_cnt"].asInt();
      gi->max_amt_ = v["max_item_cnt"].asInt();
      gi->p_ = v["p"].asInt();  
      if (!item_config::instance()->find(gi->item_cid_)
          || (gi->bind_ != BIND_TYPE
              && gi->bind_ != UNBIND_TYPE)
          || (gi->min_amt_ > gi->max_amt_))
      {
        e_log->rinfo("item %d not found or bind is error!", gi->item_cid_);
        return -1;
      }
    }
    return 0;
  }
  gift_obj *get_gift(const int gift_cid)
  {
    gift_obj_map_itor itor = this->gift_obj_map_.find(gift_cid);
    if (itor == this->gift_obj_map_.end()) return NULL;
    return itor->second;
  }
private:
  gift_obj_map_t gift_obj_map_;  
};
gift_config::gift_config() : impl_(new gift_config_impl()) { }
int gift_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int gift_config::reload_config(const char *cfg_root)
{
  gift_config_impl *tmp_impl = new gift_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GIFT_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
gift_obj *gift_config::get_gift(const int gift_cid) 
{ return this->impl_->get_gift(gift_cid); }

/**
 * @class gift_byvip_config_impl
 *
 * @brief
 */
class gift_byvip_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, array_t<gift_obj *> *> gift_obj_byvip_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, array_t<gift_obj *> *>::iterator gift_obj_byvip_map_itor;

  gift_byvip_config_impl() :
    gift_obj_byvip_map_(256)
  { }
  ~gift_byvip_config_impl()
  {
    for (gift_obj_byvip_map_itor itor = this->gift_obj_byvip_map_.begin();
         itor != this->gift_obj_byvip_map_.end();
         ++itor)
    {
      for (int i = 0; i < itor->second->size(); ++i)
        delete itor->second->find(i);
      delete itor->second;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GIFT_BYVIP_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      array_t<gift_obj *> *array = new array_t<gift_obj *>(MAX_VIP_LVL + 1);
      this->gift_obj_byvip_map_.insert(std::make_pair(cid, array));

      for (Json::Value::iterator vipiter = (*iter).begin();
           vipiter != (*iter).end();
           ++vipiter)
      {
        int vip = ::atoi(vipiter.key().asCString());
        gift_obj *go = new gift_obj();
        if (array->insert(vip, go) != 0)
        {
          delete go;
          return -1;
        }
        if (this->load_json(*vipiter, go) != 0)
          return -1;
      }
    }
    return 0;
  }
  int load_json(Json::Value &root, gift_obj *go)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      Json::Value &v = *iter;
      gift_info *gi = new gift_info();
      go->gift_list_.push_back(gi);
      gi->bind_ = v["bind"].asInt();
      gi->item_cid_ = v["item_cid"].asInt();
      gi->min_amt_ = v["min_item_cnt"].asInt();
      gi->max_amt_ = v["max_item_cnt"].asInt();
      gi->p_ = v["p"].asInt();  
      if (!item_config::instance()->find(gi->item_cid_)
          || (gi->bind_ != BIND_TYPE
              && gi->bind_ != UNBIND_TYPE)
          || (gi->min_amt_ > gi->max_amt_))
      {
        e_log->rinfo("item %d not found or bind is error!", gi->item_cid_);
        return -1;
      }
    }
    return 0;
  }
  gift_obj *get_gift(const int gift_cid, const int vip)
  {
    gift_obj_byvip_map_itor itor = this->gift_obj_byvip_map_.find(gift_cid);
    if (itor == this->gift_obj_byvip_map_.end()) return NULL;
    return itor->second->find(vip);
  }
private:
  gift_obj_byvip_map_t gift_obj_byvip_map_;
};
gift_byvip_config::gift_byvip_config() : impl_(new gift_byvip_config_impl()) { }
int gift_byvip_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int gift_byvip_config::reload_config(const char *cfg_root)
{
  gift_byvip_config_impl *tmp_impl = new gift_byvip_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GIFT_BYVIP_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
gift_obj *gift_byvip_config::get_gift(const int gift_cid, const int vip) 
{ return this->impl_->get_gift(gift_cid, vip); }
