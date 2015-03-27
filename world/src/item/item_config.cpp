#include "item_config.h"
#include "load_json.h"
#include "global_macros.h"
#include "item_effect_def.h"
#include "global_param_cfg.h"
#include "sys_log.h"
#include "clsid.h"
#include "def.h"

// Lib header
#include <tr1/unordered_map>

// item config log
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

bool item_config::item_is_money(const int cid)
{
  if (cid == global_param_cfg::item_coin_cid
      || cid == global_param_cfg::item_diamond_cid
      || cid == global_param_cfg::item_bind_diamond_cid)
    return true;
  return false;
}
/**
 * @class item_config_impl
 *
 * @brief
 */
class item_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, item_cfg_obj *> item_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, item_cfg_obj *>::iterator item_cfg_obj_map_iter;
  typedef std::tr1::unordered_map<int/*cid*/, equip_cfg_obj *> equip_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, equip_cfg_obj *>::iterator equip_cfg_obj_map_iter;
  typedef std::tr1::unordered_map<int/*cid*/, fa_bao_cfg_obj *> fa_bao_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, fa_bao_cfg_obj *>::iterator fa_bao_cfg_obj_map_iter;

  item_config_impl() :
    item_cfg_obj_map_(2048),
    equip_cfg_obj_map_(2048),
    fa_bao_cfg_obj_map_(128)
  { }
  ~item_config_impl()
  {
    for (item_cfg_obj_map_iter itor = this->item_cfg_obj_map_.begin();
         itor != this->item_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
    for (equip_cfg_obj_map_iter itor = this->equip_cfg_obj_map_.begin();
         itor != this->equip_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
    for (fa_bao_cfg_obj_map_iter itor = this->fa_bao_cfg_obj_map_.begin();
         itor != this->fa_bao_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    // consume item
    {
      Json::Value root;
      if (this->load_json_cfg(cfg_root, ITEM_CFG_PATH, root) != 0)
        return -1;

      for(Json::Value::iterator iter = root.begin();
          iter != root.end();
          ++iter)
      {
        int cid = ::atoi(iter.key().asCString());
        item_cfg_obj *ico = new item_cfg_obj();
        this->item_cfg_obj_map_.insert(std::make_pair(cid, ico));
        if (this->load_item_json(cid, *iter, ico) != 0)
          return -1;
      }
    }
    // equip
    {
      Json::Value root;
      if (this->load_json_cfg(cfg_root, EQUIP_CFG_PATH, root) != 0)
        return -1;

      for(Json::Value::iterator iter = root.begin();
          iter != root.end();
          ++iter)
      {
        int cid = ::atoi(iter.key().asCString());
        equip_cfg_obj *eco = new equip_cfg_obj();
        this->equip_cfg_obj_map_.insert(std::make_pair(cid, eco));
        if (this->load_equip_json(cid, *iter, eco) != 0)
          return -1;
      }
    }
    // fa bao
    {
      Json::Value root;
      if (this->load_json_cfg(cfg_root, FA_BAO_CFG_PATH, root) != 0)
        return -1;

      for(Json::Value::iterator iter = root.begin();
          iter != root.end();
          ++iter)
      {
        int cid = ::atoi(iter.key().asCString());
        fa_bao_cfg_obj *fbco = new fa_bao_cfg_obj();
        this->fa_bao_cfg_obj_map_.insert(std::make_pair(cid, fbco));
        if (this->load_fa_bao_json(cid, *iter, fbco) != 0)
          return -1;
      }
    }
    return 0;
  }
  int load_item_json(const int item_cid, Json::Value &root, item_cfg_obj *ico)
  {
    ico->use_lvl_ = root["use_lvl"].asInt();
    ico->price_ = root["price"].asInt();
    ico->can_sell_ = root["can_sell"].asBool();
    ico->can_use_ = root["can_use"].asBool();
    ico->color_ = root["color"].asInt();
    ico->cd_ = root["cd"].asInt();
    ico->dj_upper_limit_ = root["dj_upper_limit"].asInt();
    if (ico->dj_upper_limit_ < 1)
    {
      e_log->error("item %d dj_upper_limit erro!", item_cid);
      return -1;
    }
    ico->buff_id_ = root["buf_id"].asInt();
    ico->value_ = root["value"].asInt();
    ico->market_classify_ = root["market_classify"].asInt();
    Json::Value &effect_v = root["effect"];
    if (!effect_v.empty() && (effect_v.asString().length() > 0))
      ico->effect_ = item_effect::match_id(effect_v.asCString());
    ::strncpy(ico->name_, root["name"].asCString(), sizeof(ico->name_) - 1);
    return 0;
  }
  int load_equip_json(const int /*item_cid*/, Json::Value &root, equip_cfg_obj *eco)
  {
    eco->wash_id_ = root["wash_id"].asInt();
    eco->suit_id_ = root["suit_id"].asInt();
    eco->career_ = root["career"].asInt();
    eco->use_lvl_ = root["use_lvl"].asInt();
    eco->color_ = root["color"].asInt();
    eco->strength_limit_ = root["strength_limit"].asInt();
    eco->part_ = root["part"].asInt();
    if (eco->part_ < PART_ZHU_WU
        || eco->part_ >= PART_END)
      return -1;
    eco->attr_float_range_             = root["attr_float_range"].asInt();
    eco->attr_val_[ATTR_T_HP]          = root["sheng_ming"].asInt();
    eco->attr_val_[ATTR_T_MP]          = root["fa_li"].asInt();
    eco->attr_val_[ATTR_T_GONG_JI]     = root["gong_ji"].asInt();
    eco->attr_val_[ATTR_T_FANG_YU]     = root["fang_yu"].asInt();
    eco->attr_val_[ATTR_T_MING_ZHONG]  = root["ming_zhong"].asInt();
    eco->attr_val_[ATTR_T_SHAN_BI]     = root["shan_bi"].asInt();
    eco->attr_val_[ATTR_T_BAO_JI]      = root["bao_ji"].asInt();
    eco->attr_val_[ATTR_T_KANG_BAO]    = root["kang_bao"].asInt();
    eco->attr_val_[ATTR_T_SHANG_MIAN]  = root["shang_mian"].asInt();
    ::strncpy(eco->name_, root["name"].asCString(), sizeof(eco->name_) - 1);
    return 0;
  }
  int load_fa_bao_json(const int item_cid, Json::Value &root, fa_bao_cfg_obj *fbco)
  {
    fbco->use_lvl_ = root["use_lvl"].asInt();
    fbco->color_ = root["color"].asInt();
    fbco->part_ = root["part"].asInt();
    if (fbco->part_ < 1
        || fbco->part_ > FA_BAO_PART_CNT)
      return -1;
    fbco->price_ = root["cost"].asInt();
    fbco->dj_upper_limit_ = root["dj_upper_limit"].asInt();
    if (fbco->dj_upper_limit_ < 1)
    {
      e_log->error("item %d dj_upper_limit erro!", item_cid);
      return -1;
    }
    fbco->hp_recover_ = root["sheng_ming_recover"].asInt();
    fbco->mp_recover_ = root["fa_li_recover"].asInt();
    fbco->add_extra_hurt_ = root["add_extra_hurt"].asInt();
    fbco->reduce_hurt_ = root["reduce_hurt"].asInt();
    fbco->attr_rate_[ATTR_T_HP]          = root["sheng_ming"].asInt();
    fbco->attr_rate_[ATTR_T_MP]          = root["fa_li"].asInt();
    fbco->attr_rate_[ATTR_T_GONG_JI]     = root["gong_ji"].asInt();
    fbco->attr_rate_[ATTR_T_FANG_YU]     = root["fang_yu"].asInt();
    fbco->attr_rate_[ATTR_T_MING_ZHONG]  = root["ming_zhong"].asInt();
    fbco->attr_rate_[ATTR_T_SHAN_BI]     = root["shan_bi"].asInt();
    fbco->attr_rate_[ATTR_T_BAO_JI]      = root["bao_ji"].asInt();
    fbco->attr_rate_[ATTR_T_KANG_BAO]    = root["kang_bao"].asInt();
    fbco->attr_rate_[ATTR_T_SHANG_MIAN]  = root["shang_mian"].asInt();
    ::strncpy(fbco->name_, root["name"].asCString(), sizeof(fbco->name_) - 1);
    fbco->market_classify_ = root["market_classify"].asInt();
    return 0;
  }
  bool find(const int item_cid)
  {
    item_cfg_obj_map_iter itor = this->item_cfg_obj_map_.find(item_cid);
    if (itor != this->item_cfg_obj_map_.end())
      return true;
    else
    {
      equip_cfg_obj_map_iter eitor = this->equip_cfg_obj_map_.find(item_cid);
      if (eitor != this->equip_cfg_obj_map_.end())
        return true;
      else
      {
        fa_bao_cfg_obj_map_iter fitor = this->fa_bao_cfg_obj_map_.find(item_cid);
        if (fitor != this->fa_bao_cfg_obj_map_.end())
          return true;
      }
    }
    return false;
  }
  item_cfg_obj *get_item(const int item_cid)
  {
    item_cfg_obj_map_iter itor = this->item_cfg_obj_map_.find(item_cid);
    if (itor == this->item_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
  equip_cfg_obj *get_equip(const int equip_cid)
  {
    equip_cfg_obj_map_iter itor = this->equip_cfg_obj_map_.find(equip_cid);
    if (itor == this->equip_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
  fa_bao_cfg_obj *get_fa_bao(const int cid)
  {
    fa_bao_cfg_obj_map_iter itor = this->fa_bao_cfg_obj_map_.find(cid);
    if (itor == this->fa_bao_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
  int dj_upper_limit(const int item_cid)
  {
    fa_bao_cfg_obj *fbco = this->get_fa_bao(item_cid);
    if (fbco == NULL)
    {
      item_cfg_obj *ico = this->get_item(item_cid);
      if (ico == NULL) return 1;
      return ico->dj_upper_limit_;
    }
    return fbco->dj_upper_limit_;
  }
  int market_classify(const int item_cid)
  {
    fa_bao_cfg_obj *fbco = this->get_fa_bao(item_cid);
    if (fbco == NULL)
    {
      item_cfg_obj *ico = this->get_item(item_cid);
      if (ico == NULL) return 1;
      return ico->market_classify_;
    }
    return fbco->market_classify_;
  }
private:
  item_cfg_obj_map_t item_cfg_obj_map_;
  equip_cfg_obj_map_t equip_cfg_obj_map_;
  fa_bao_cfg_obj_map_t fa_bao_cfg_obj_map_;
};
// ------------------------------- item config ------------------------------
item_config::item_config()
: impl_(new item_config_impl())
{ }
int item_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int item_config::reload_config(const char *cfg_root)
{
  item_config_impl *tmp_impl = new item_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s or %s or %s failed!", ITEM_CFG_PATH, EQUIP_CFG_PATH, FA_BAO_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
bool item_config::find(const int item_cid)
{ return this->impl_->find(item_cid); }
item_cfg_obj *item_config::get_item(const int item_cid)
{ return this->impl_->get_item(item_cid); }
equip_cfg_obj *item_config::get_equip(const int item_cid)
{ return this->impl_->get_equip(item_cid); }
fa_bao_cfg_obj *item_config::get_fa_bao(const int item_cid)
{ return this->impl_->get_fa_bao(item_cid); }
int item_config::dj_upper_limit(const int item_cid)
{ return this->impl_->dj_upper_limit(item_cid); }
int item_config::market_classify(const int item_cid)
{ return this->impl_->market_classify(item_cid); }
