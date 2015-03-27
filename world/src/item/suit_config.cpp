#include "suit_config.h"
#include "load_json.h"
#include "global_macros.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header
#include <tr1/unordered_map>

#define MAX_SUIT_COUNT         10

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class suit_cfg_obj 
 *
 * @brief
 */
class suit_cfg_obj
{
public:
  suit_cfg_obj() :
    suit_attr_info_obj_map_(MAX_SUIT_COUNT + 1)
  { }
  
  ~suit_cfg_obj()
  {
    for (int i = 0; i < this->suit_attr_info_obj_map_.size(); ++i)
    {
      suit_attr_info *sai = this->suit_attr_info_obj_map_.find(i);
      if (sai != NULL) delete sai;
    }
  }
  int load_json(Json::Value &v)
  {
    Json::Value info = v["info"];
    if (info.empty())
      return -1;
    for (Json::Value::iterator iter = info.begin();
         iter != info.end();
         ++iter)
    {
      int suit_count = ::atoi(iter.key().asCString());
      suit_attr_info *sai = new suit_attr_info();
      
      sai->attr_val_[ATTR_T_HP]          = (*iter)["sheng_ming"].asInt();
      sai->attr_val_[ATTR_T_GONG_JI]     = (*iter)["gong_ji"].asInt();
      sai->attr_val_[ATTR_T_FANG_YU]     = (*iter)["fang_yu"].asInt();
      sai->attr_val_[ATTR_T_MING_ZHONG]  = (*iter)["ming_zhong"].asInt();
      sai->attr_val_[ATTR_T_SHAN_BI]     = (*iter)["shan_bi"].asInt();
      sai->attr_val_[ATTR_T_BAO_JI]      = (*iter)["bao_ji"].asInt();
      sai->attr_val_[ATTR_T_KANG_BAO]    = (*iter)["kang_bao"].asInt();
      sai->attr_val_[ATTR_T_SHANG_MIAN]  = (*iter)["shang_mian"].asInt();

      this->suit_attr_info_obj_map_.insert(suit_count, sai);
    }
    return 0;
  }
public:
  array_t<suit_attr_info *> suit_attr_info_obj_map_; 
};

/**
 * @class suit_config_impl
 *
 * @brief
 */
class suit_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*suit_cid*/, suit_cfg_obj *> suit_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*suit_cid*/, suit_cfg_obj *>::iterator suit_cfg_obj_map_iter;

  suit_config_impl() :
    suit_cfg_obj_map_(128)
  { }
  ~suit_config_impl()
  {
    for (suit_cfg_obj_map_iter iter = this->suit_cfg_obj_map_.begin();
         iter != this->suit_cfg_obj_map_.end();
         ++iter)
      delete iter->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SUIT_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int suit_id = ::atoi(iter.key().asCString());
      suit_cfg_obj *sco = new suit_cfg_obj();
      this->suit_cfg_obj_map_.insert(std::make_pair(suit_id, sco));

      if (sco->load_json(*iter) != 0)
        return -1;
    }
    return 0;
  }
  suit_attr_info *get_suit_attr_info(const int suit_id, const char count)
  {
    suit_cfg_obj_map_iter iter = this->suit_cfg_obj_map_.find(suit_id); 
    if (iter == this->suit_cfg_obj_map_.end())
      return NULL;
    return iter->second->suit_attr_info_obj_map_.find(count);
  }
private:
  suit_cfg_obj_map_t suit_cfg_obj_map_;  
};
// --------------------------------suit config-------------------------------
suit_attr_info::suit_attr_info()
{ ::memset(this->attr_val_, 0, sizeof(this->attr_val_)); }
suit_config::suit_config()
  : impl_(new suit_config_impl())
{ }
int suit_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int suit_config::reload_config(const char *cfg_root)
{
  suit_config_impl *tmp_impl = new suit_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SUIT_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
suit_attr_info *suit_config::get_suit_attr_info(const int suit_id, const char count)
{ return this->impl_->get_suit_attr_info(suit_id, count); }
