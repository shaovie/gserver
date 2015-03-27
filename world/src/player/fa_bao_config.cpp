#include "fa_bao_config.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header
#include <tr1/unordered_map>

//= define
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class fa_bao_dj_cfg_impl
 *
 * @brief
 */
class fa_bao_dj_cfg_impl : public load_json
{
public:
  fa_bao_dj_cfg_impl() :
    fa_bao_dj_map_(FA_BAO_MAX_DJ + 1)
  { }
  ~fa_bao_dj_cfg_impl()
  {
    for (int i = 0; i < this->fa_bao_dj_map_.size(); ++i)
    {
      fa_bao_dj_cfg_obj *p = this->fa_bao_dj_map_.find(i);
      delete p;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, FA_BAO_DENG_JIE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int dj = ::atoi(iter.key().asCString());
      fa_bao_dj_cfg_obj *p = new fa_bao_dj_cfg_obj();
      this->fa_bao_dj_map_.insert(dj, p);
      if (dj > FA_BAO_MAX_DJ) return -1;
      Json::Value &jv = *iter;
      char bf[256] = {0};
      ::strncpy(bf, jv["part"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      int part = 1;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        if (part > FA_BAO_PART_CNT) return -1;
        p->part_fa_bao_cid_[part++] = ::atoi(token);
      }
      p->hp_recover_ = jv["sheng_ming_recover"].asInt();
      p->mp_recover_ = jv["fa_li_recover"].asInt();
      p->add_extra_hurt_ = jv["add_extra_hurt"].asInt();
      p->reduce_hurt_ = jv["reduce_hurt"].asInt();
      p->attr_rate_[ATTR_T_HP]          = jv["sheng_ming"].asInt();
      p->attr_rate_[ATTR_T_MP]          = jv["fa_li"].asInt();
      p->attr_rate_[ATTR_T_GONG_JI]     = jv["gong_ji"].asInt();
      p->attr_rate_[ATTR_T_FANG_YU]     = jv["fang_yu"].asInt();
      p->attr_rate_[ATTR_T_MING_ZHONG]  = jv["ming_zhong"].asInt();
      p->attr_rate_[ATTR_T_SHAN_BI]     = jv["shan_bi"].asInt();
      p->attr_rate_[ATTR_T_BAO_JI]      = jv["bao_ji"].asInt();
      p->attr_rate_[ATTR_T_KANG_BAO]    = jv["kang_bao"].asInt();
      p->attr_rate_[ATTR_T_SHANG_MIAN]  = jv["shang_mian"].asInt();
    }
    return 0;
  }
  fa_bao_dj_cfg_obj *get_dj_info(const int dj)
  { return this->fa_bao_dj_map_.find(dj); }
private:
  array_t<fa_bao_dj_cfg_obj *> fa_bao_dj_map_;
};
fa_bao_dj_cfg::fa_bao_dj_cfg()
: impl_(new fa_bao_dj_cfg_impl())
{ }
int fa_bao_dj_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int fa_bao_dj_cfg::reload_config(const char *cfg_root)
{
  fa_bao_dj_cfg_impl *tmp_impl = new fa_bao_dj_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", FA_BAO_DENG_JIE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
fa_bao_dj_cfg_obj *fa_bao_dj_cfg::get_dj_info(const int part)
{ return this->impl_->get_dj_info(part); }
// -------------------------------------he cheng-----------------------------------
/**
 * @class fa_bao_he_cheng_cfg_impl
 *
 * @brief
 */
class fa_bao_he_cheng_cfg_impl : public load_json
{
  typedef std::tr1::unordered_map<int/*item_cid*/, fa_bao_he_cheng_cfg_obj *> fa_bao_he_cheng_map_t;
  typedef std::tr1::unordered_map<int/*item_cid*/, fa_bao_he_cheng_cfg_obj *>::iterator fa_bao_he_cheng_map_iter;
public:
  fa_bao_he_cheng_cfg_impl()
  { }
  ~fa_bao_he_cheng_cfg_impl()
  {
    fa_bao_he_cheng_map_iter itor = this->fa_bao_he_cheng_map_.begin();
    for (; itor != this->fa_bao_he_cheng_map_.end(); ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, FA_BAO_HE_CHENG_CFG_PATH, root) != 0)
      return -1;
    
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      if (!item_config::instance()->find(cid))
      {
        e_log->error("fa_bao_he_cheng[%d] is nonexistent!", cid);
        return -1;
      }
      fa_bao_he_cheng_cfg_obj *p = new fa_bao_he_cheng_cfg_obj();
      this->fa_bao_he_cheng_map_.insert(std::make_pair(cid, p));
      if (this->load_json(*iter, p) != 0)
        return -1;
    }
    return 0;
  }

  int load_json(Json::Value &v, fa_bao_he_cheng_cfg_obj *p)
  {
    p->cost_ = v["cost"].asInt();
    if (p->cost_ < 0)
    {
      e_log->error("the cost is error!");
      return -1;
    }
    Json::Value &material_v = v["info"];
    if (material_v.empty())
    {
      e_log->error("the material is empty in fa_bao_he_cheng!");
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

      p->material_.push_back(pair_t<int>(clsid, count));
    }
    return 0;
  }
  
  const fa_bao_he_cheng_cfg_obj* get_fa_bao_he_cheng_cfg(const int item_cid)
  { 
    fa_bao_he_cheng_map_iter itor = this->fa_bao_he_cheng_map_.find(item_cid);
    if (itor == this->fa_bao_he_cheng_map_.end())
      return NULL;
    return itor->second;
  }
private:
  fa_bao_he_cheng_map_t fa_bao_he_cheng_map_;
};
// ------------------------------- fa_bao_he_cheng config ------------------------------
fa_bao_he_cheng_cfg::fa_bao_he_cheng_cfg()
  : impl_(new fa_bao_he_cheng_cfg_impl())
{ }
int fa_bao_he_cheng_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int fa_bao_he_cheng_cfg::reload_config(const char *cfg_root)
{
  fa_bao_he_cheng_cfg_impl *tmp_impl = new fa_bao_he_cheng_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", FA_BAO_HE_CHENG_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const fa_bao_he_cheng_cfg_obj *fa_bao_he_cheng_cfg::get_he_cheng_info(const int item_cid)
{ return this->impl_->get_fa_bao_he_cheng_cfg(item_cid); }
