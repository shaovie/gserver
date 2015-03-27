#include "product_cfg.h"
#include "load_json.h"
#include "global_macros.h"
#include "item_config.h"
#include "sys_log.h"

// Lib header
#include <tr1/unordered_map>

//= define
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class product_cfg_impl
 *
 * @brief
 */
class product_cfg_impl : public load_json
{
  typedef std::tr1::unordered_map<int/*item_cid*/, product *> product_map_t;
  typedef std::tr1::unordered_map<int/*item_cid*/, product *>::iterator product_map_iter;
public:
  product_cfg_impl()
  { }
  ~product_cfg_impl()
  {
    product_map_iter itor = this->product_map_.begin();
    for (; itor != this->product_map_.end(); ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, PRODUCT_CFG_PATH, root) != 0)
      return -1;
    
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (!item_config::instance()->find(id))
      {
        e_log->error("product[%d] is nonexistent!", id);
        return -1;
      }
      product *p = new product();
      this->product_map_.insert(std::make_pair(id, p));
      if (this->load_json(*iter, p) != 0)
        return -1;
    }
    return 0;
  }

  int load_json(Json::Value &v, product *pct)
  {
    pct->notify_ = v["notice"].asBool();
    pct->expend_ = v["cost"].asInt();
    pct->bind_ = v["bind"].asInt();
    if (pct->bind_ != UNBIND_TYPE && pct->bind_ != BIND_TYPE)
      return -1;
    if (pct->expend_ < 0)
    {
      e_log->error("the cost is error!");
      return -1;
    }
    Json::Value &material_v = v["info"];
    if (material_v.empty())
    {
      e_log->error("the material is empty in product!");
      return -1;
    }
    for (Json::Value::iterator iter = material_v.begin();
         iter != material_v.end();
         ++iter)
    {
      int clsid = ((*iter)["materials_cid"]).asInt();
      if (!item_config::instance()->find(clsid))
      {
        e_log->error("material[%d] is nonexistent!", clsid);
        return -1;
      }
      int count = ((*iter)["materials_cnt"]).asInt();

      pct->material_.push_back(pair_t<int>(clsid, count));
    }
    return 0;
  }
  
  const product* get_product(const int item_cid)
  { 
    product_map_iter itor = this->product_map_.find(item_cid);
    if (itor == this->product_map_.end())
      return NULL;
    return itor->second;
  }
private:
  product_map_t product_map_;
};
// ------------------------------- product config ------------------------------
product_cfg::product_cfg()
  : impl_(new product_cfg_impl())
{ }
int product_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int product_cfg::reload_config(const char *cfg_root)
{
  product_cfg_impl *tmp_impl = new product_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", PRODUCT_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const product* product_cfg::get_product(const int item_cid)
{ return this->impl_->get_product(item_cid); }
