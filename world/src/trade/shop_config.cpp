#include "shop_config.h"
#include "item_config.h"
#include "load_json.h"
#include "global_macros.h"
#include "sys_log.h"
#include "def.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

#define MAX_GOODS_OBJ_AMT    500
#define MAX_SHOP_CFG_OBJ_AMT 64

/**
 * @class shop_cfg_obj
 *
 * @brief
 */
class shop_cfg_obj
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, goods_obj *> goods_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, goods_obj *>::iterator goods_obj_map_iter;
public:
  shop_cfg_obj() :
    goods_obj_map_(MAX_GOODS_OBJ_AMT)
  { }
  ~shop_cfg_obj()
  {
    for (goods_obj_map_iter itor = this->goods_obj_map_.begin();
         itor != this->goods_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_json(Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      Json::Value &v = *iter;
      int group_id = ::atoi(iter.key().asCString());
      goods_obj *go = new goods_obj();
      this->goods_obj_map_.insert(std::make_pair(group_id, go));

      go->item_cid_ = v["item_cid"].asInt();
      go->item_cnt_ = v["item_cnt"].asInt();
      go->bind_type_ = v["sell_type"].asInt();
      go->price_type_ = v["price_type"].asInt();
      if (!item_config::instance()->find(go->item_cid_))
      {
        e_log->rinfo("item not found!", go->item_cid_);
        return -1;
      }
      if (go->price_type_ != M_DIAMOND
          && go->price_type_ != M_BIND_DIAMOND
          && go->price_type_ != M_COIN
          && go->price_type_ != M_BIND_UNBIND_DIAMOND
          && go->price_type_ != V_JING_JI_SCORE
          && go->price_type_ != V_XSZC_HONOR
          )
        return -1;
      go->price_ = v["price"].asInt();
      if (go->price_ < 0) return -1;
      go->limit_cnt_ = v["limit_cnt"].asInt();
    }
    return 0;
  }
  const goods_obj *get_goods(const int item_cid)
  {
    goods_obj_map_iter iter = this->goods_obj_map_.find(item_cid);
    if (iter == this->goods_obj_map_.end()) return NULL;
    return iter->second;
  }
public:
  goods_obj_map_t goods_obj_map_;
};
/**
 * @class shop_config_impl
 *
 * @brief
 */
class shop_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, shop_cfg_obj *> shop_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, shop_cfg_obj *>::iterator shop_cfg_obj_map_iter;
public:
  shop_config_impl() :
    shop_cfg_obj_map_(MAX_SHOP_CFG_OBJ_AMT)
  { }
  ~shop_config_impl()
  {
    for (shop_cfg_obj_map_iter itor = this->shop_cfg_obj_map_.begin();
         itor != this->shop_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SHOP_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int npc_cid = ::atoi(iter.key().asCString());
      shop_cfg_obj *so = new shop_cfg_obj();
      this->shop_cfg_obj_map_.insert(std::make_pair(npc_cid, so));
      if (so->load_json(*iter) != 0)
        return -1;
    }
    return 0;
  }
  const goods_obj *get_goods(const int npc_cid, const int group_id)
  {
    shop_cfg_obj_map_iter iter = this->shop_cfg_obj_map_.find(npc_cid);
    if (iter == this->shop_cfg_obj_map_.end()) return NULL;
    return iter->second->get_goods(group_id);
  }
  int price_type(const int npc_cid, const int group_id)
  {
    const goods_obj *go = this->get_goods(npc_cid, group_id);
    if (go == NULL) return -1;
    return go->price_type_;
  }
private:
  shop_cfg_obj_map_t shop_cfg_obj_map_;
};
shop_config::shop_config() : impl_(new shop_config_impl()) { }
int shop_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int shop_config::reload_config(const char *cfg_root)
{
  shop_config_impl *tmp_impl = new shop_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SHOP_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const goods_obj *shop_config::get_goods(const int npc_cid, const int group_id)
{ return this->impl_->get_goods(npc_cid, group_id); }
int shop_config::price_type(const int npc_cid, const int group_id)
{ return this->impl_->price_type(npc_cid, group_id); }
