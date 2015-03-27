#include "iu_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"
#include "def.h"
#include "global_param_cfg.h"
#include "item_config.h"

// Lib header
#include <map>
#include <tr1/unordered_map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class equip_strengthen_cfg_impl
 * 
 * @brief implement of equip_strengthen_cfg
 */
class equip_strengthen_cfg_impl : public load_json
{
public:
  equip_strengthen_cfg_impl() :
    strengthen_cfg_obj_map_(global_param_cfg::lvl_limit)
  { }
  ~equip_strengthen_cfg_impl()
  {
    for (int i = 0; i < this->strengthen_cfg_obj_map_.size(); ++i)
    {
      equip_strengthen_cfg_obj *p = this->strengthen_cfg_obj_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_STRENGTHEN_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int lvl = ::atoi(iter.key().asCString());
      if (lvl > global_param_cfg::lvl_limit)
        return -1;
      equip_strengthen_cfg_obj *p = new equip_strengthen_cfg_obj();
      this->strengthen_cfg_obj_map_.insert(lvl, p);

      p->cost_ = (*iter)["cost"].asInt();
      p->item_cid_ = (*iter)["item_cid"].asInt();
      p->item_cnt_ = (*iter)["item_cnt"].asInt();
      p->probability_ = (*iter)["probability"].asInt();
      p->lucky_rate_ = (*iter)["lucky_rate"].asInt();
      p->min_strengthen_cnt_ = (*iter)["min_strengthen_cnt"].asInt();
      p->min_bless_val_ = (*iter)["min_bless_val"].asInt();
      p->max_bless_val_ = (*iter)["max_bless_val"].asInt();
      p->total_bless_val_ = (*iter)["total_bless_val"].asInt();
      if (p->max_bless_val_ < p->min_bless_val_)
        return -1;
    }
    return 0;
  }
  const equip_strengthen_cfg_obj *get_strengthen_info(const short lvl)
  { return this->strengthen_cfg_obj_map_.find(lvl); }
private:
  array_t<equip_strengthen_cfg_obj *> strengthen_cfg_obj_map_;
};
equip_strengthen_cfg::equip_strengthen_cfg() : impl_(new equip_strengthen_cfg_impl()) { }
int equip_strengthen_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_strengthen_cfg::reload_config(const char *cfg_root)
{
  equip_strengthen_cfg_impl *tmp_impl = new equip_strengthen_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_STRENGTHEN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const equip_strengthen_cfg_obj *equip_strengthen_cfg::get_strengthen_info(const short lvl)
{ return this->impl_->get_strengthen_info(lvl); }
//----------------------------------------------------------------------------------------
/**
 * @class equip_fen_jie_cfg_impl
 * 
 * @brief implement of equip_fen_jie_cfg
 */
class equip_fen_jie_cfg_impl : public load_json
{
public:
  equip_fen_jie_cfg_impl() :
    fen_jie_cfg_obj_map_(ICOLOR_CNT)
  { }
  ~equip_fen_jie_cfg_impl()
  {
    for (int i = 0; i < this->fen_jie_cfg_obj_map_.size(); ++i)
    {
      equip_fen_jie_cfg_obj *p = this->fen_jie_cfg_obj_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_FEN_JIE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int color = ::atoi(iter.key().asCString());
      if (color >= ICOLOR_CNT) return -1;
      equip_fen_jie_cfg_obj *p = new equip_fen_jie_cfg_obj();
      this->fen_jie_cfg_obj_map_.insert(color, p);

      p->cost_ = (*iter)["cost"].asInt();
      p->item_cid_ = (*iter)["item_cid"].asInt();
      p->item_cnt_ = (*iter)["item_cnt"].asInt();
      if (!item_config::instance()->find(p->item_cid_)
          || p->item_cnt_ < 1)
        return -1;
    }
    return 0;
  }
  const equip_fen_jie_cfg_obj *get_fen_jie_info(const int color)
  { return this->fen_jie_cfg_obj_map_.find(color); }
private:
  array_t<equip_fen_jie_cfg_obj *> fen_jie_cfg_obj_map_;
};
equip_fen_jie_cfg::equip_fen_jie_cfg() : impl_(new equip_fen_jie_cfg_impl()) { }
int equip_fen_jie_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_fen_jie_cfg::reload_config(const char *cfg_root)
{
  equip_fen_jie_cfg_impl *tmp_impl = new equip_fen_jie_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_FEN_JIE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const equip_fen_jie_cfg_obj *equip_fen_jie_cfg::get_fen_jie_info(const int color)
{ return this->impl_->get_fen_jie_info(color); }
//----------------------------------------------------------------------------------------
/**
 * @class equip_rong_he_cfg_impl
 * 
 * @brief implement of equip_rong_he_cfg
 */
class equip_rong_he_cfg_impl : public load_json
{
public:
  equip_rong_he_cfg_impl() :
    rong_he_cfg_obj_map_(ICOLOR_CNT)
  { }
  ~equip_rong_he_cfg_impl()
  {
    for (int i = 0; i < this->rong_he_cfg_obj_map_.size(); ++i)
    {
      equip_rong_he_cfg_obj *p = this->rong_he_cfg_obj_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_RONG_HE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int color = ::atoi(iter.key().asCString());
      if (color >= ICOLOR_CNT) return -1;
      equip_rong_he_cfg_obj *p = new equip_rong_he_cfg_obj();
      this->rong_he_cfg_obj_map_.insert(color, p);

      p->cost_ = (*iter)["cost"].asInt();
      p->item_cid_ = (*iter)["item_cid"].asInt();
      p->item_cnt_ = (*iter)["item_cnt"].asInt();
      if (!item_config::instance()->find(p->item_cid_)
          || p->item_cnt_ < 1)
        return -1;
    }
    return 0;
  }
  const equip_rong_he_cfg_obj *get_rong_he_info(const int color)
  { return this->rong_he_cfg_obj_map_.find(color); }
private:
  array_t<equip_rong_he_cfg_obj *> rong_he_cfg_obj_map_;
};
equip_rong_he_cfg::equip_rong_he_cfg() : impl_(new equip_rong_he_cfg_impl()) { }
int equip_rong_he_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_rong_he_cfg::reload_config(const char *cfg_root)
{
  equip_rong_he_cfg_impl *tmp_impl = new equip_rong_he_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_RONG_HE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const equip_rong_he_cfg_obj *equip_rong_he_cfg::get_rong_he_info(const int color)
{ return this->impl_->get_rong_he_info(color); }
//----------------------------------------------------------------------------------------
/**
 * @class equip_xi_lian_config_impl
 *
 * @brief
 */
class equip_xi_lian_config_impl : public load_json
{
public:
  equip_xi_lian_config_impl() :
    equip_xi_lian_info_(ICOLOR_CNT)
  { }
  ~equip_xi_lian_config_impl()
  {
    for (int i = 0; i < this->equip_xi_lian_info_.size(); ++i)
    {
      equip_xi_lian_obj *exlo = this->equip_xi_lian_info_.find(i);
      if (exlo != NULL) delete exlo;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_XI_LIAN_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int color = ::atoi(iter.key().asCString());
      if (color >= ICOLOR_CNT) return -1;

      equip_xi_lian_obj *exlo = new equip_xi_lian_obj();
      this->equip_xi_lian_info_.insert(color, exlo);
      exlo->item_cid_   = (*iter)["item_cid"].asInt();
      exlo->item_cnt_   = (*iter)["item_cnt"].asInt();
      exlo->cost_       = (*iter)["cost"].asInt();
      exlo->min_num_    = (*iter)["min_num"].asInt();
      exlo->max_num_    = (*iter)["max_num"].asInt();
      if (exlo->min_num_ > exlo->max_num_
          || exlo->max_num_ > MAX_XI_LIAN_ATTR_NUM)
        return -1;
    }
    return 0;
  }
  equip_xi_lian_obj *get_equip_xi_lian_obj(const int color)
  { return this->equip_xi_lian_info_.find(color); }
private:
  array_t<equip_xi_lian_obj *> equip_xi_lian_info_;
};
equip_xi_lian_config::equip_xi_lian_config() :
  impl_(new equip_xi_lian_config_impl())
{ }
int equip_xi_lian_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_xi_lian_config::reload_config(const char *cfg_root)
{
  equip_xi_lian_config_impl *tmp_impl = new equip_xi_lian_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_XI_LIAN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
equip_xi_lian_obj* equip_xi_lian_config::get_equip_xi_lian_obj(const int color)
{ return this->impl_->get_equip_xi_lian_obj(color); }
//----------------------------------------------------------------------------------------
class _xl_rule_info
{
public:
  _xl_rule_info() : xl_cnt_(0), rate_(0) { }

  int xl_cnt_;
  int rate_;
};
/**
 * @class xi_lian_rule_config_impl
 *
 * @brief
 */
class xi_lian_rule_config_impl : public load_json
{
public:
  xi_lian_rule_config_impl() :
    xi_lian_rule_info_(MAX_XI_LIAN_STAR + 1)
  { }
  ~xi_lian_rule_config_impl()
  {
    for (int i = 0; i < this->xi_lian_rule_info_.size(); ++i)
    {
      _xl_rule_info *xri = this->xi_lian_rule_info_.find(i);
      if (xri != NULL) delete xri;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, XI_LIAN_RULE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int star = ::atoi(iter.key().asCString());
      if (star > MAX_XI_LIAN_STAR) return -1;

      _xl_rule_info *xri = new _xl_rule_info();
      xri->xl_cnt_ = (*iter)["floor_cnt"].asInt();
      xri->rate_   = (*iter)["p"].asInt();

      this->xi_lian_rule_info_.insert(star, xri);
    }
    return 0;
  }
  int xl_cnt(const int star)
  {
    _xl_rule_info *xri = this->xi_lian_rule_info_.find(star);
    if (xri == NULL) return 999999999;
    return xri->xl_cnt_;
  }
  int rate(const int star)
  {
    _xl_rule_info *xri = this->xi_lian_rule_info_.find(star);
    if (xri == NULL) return 0;
    return xri->rate_;
  }
private:
  array_t<_xl_rule_info *> xi_lian_rule_info_;
};
xi_lian_rule_config::xi_lian_rule_config() :
  impl_(new xi_lian_rule_config_impl())
{ }
int xi_lian_rule_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int xi_lian_rule_config::reload_config(const char *cfg_root)
{
  xi_lian_rule_config_impl *tmp_impl = new xi_lian_rule_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", XI_LIAN_RULE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int xi_lian_rule_config::xl_cnt(const int star)
{ return this->impl_->xl_cnt(star); }
int xi_lian_rule_config::rate(const int star)
{ return this->impl_->rate(star); }
//----------------------------------------------------------------------------------------
/**
 * @class xi_lian_value_config_impl
 *
 * @brief
 */
class xi_lian_value_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*suit_cid*/, xi_lian_value_obj *> xi_lian_value_map_t;
  typedef std::tr1::unordered_map<int/*suit_cid*/, xi_lian_value_obj *>::iterator xi_lian_value_map_itor;

  xi_lian_value_config_impl() : xi_lian_value_map_(1000) { }
  ~xi_lian_value_config_impl()
  {
    for (xi_lian_value_map_itor itor = this->xi_lian_value_map_.begin();
         itor != this->xi_lian_value_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, XI_LIAN_VALUE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int suit_cid = ::atoi(iter.key().asCString());

      xi_lian_value_obj *xlvo = new xi_lian_value_obj();
      this->xi_lian_value_map_.insert(std::make_pair(suit_cid, xlvo));

      Json::Value &info_v = (*iter)["info"];
      if (this->load_json(xlvo, info_v) != 0) return -1;
    }
    return 0;
  }
  int load_json(xi_lian_value_obj *xlvo, Json::Value &root)
  {
    for (Json::Value::iterator sv_iter = root.begin();
         sv_iter != root.end();
         ++sv_iter)
    {
      int attr = ::atoi(sv_iter.key().asCString());
      if (attr < ATTR_T_HP
          || attr >= ATTR_T_ITEM_CNT) return -1;

      xi_lian_value_obj::_s_value *sv = new xi_lian_value_obj::_s_value();
      xlvo->s_value_info_.insert(attr, sv);

      int size = (*sv_iter).size();
      for (int i = 0; i < size; ++i)
      {
        int star = (*sv_iter)[i]["star"].asInt();
        if (star > MAX_XI_LIAN_STAR) return -1;
        sv->value_[star]  = (*sv_iter)[i]["val"].asInt();
      }
    }
    return 0;
  }
  xi_lian_value_obj *get_xi_lian_value_obj(const int suit_cid)
  {
    xi_lian_value_map_itor itor = this->xi_lian_value_map_.find(suit_cid);
    if (itor == this->xi_lian_value_map_.end()) return NULL;
    return itor->second;
  }
private:
  xi_lian_value_map_t xi_lian_value_map_;
};
xi_lian_value_obj::xi_lian_value_obj() :
  s_value_info_(ATTR_T_ITEM_CNT)
{ }
xi_lian_value_obj::~xi_lian_value_obj()
{
  for (int i = 0; i < this->s_value_info_.size(); ++i)
  {
    xi_lian_value_obj::_s_value *sv = this->s_value_info_.find(i);
    if (sv != NULL) delete sv;
  }
}
xi_lian_value_obj::_s_value::_s_value()
{
  ::memset(this->value_, 0, sizeof(this->value_));
}
xi_lian_value_config::xi_lian_value_config() :
  impl_(new xi_lian_value_config_impl())
{ }
int xi_lian_value_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int xi_lian_value_config::reload_config(const char *cfg_root)
{
  xi_lian_value_config_impl *tmp_impl = new xi_lian_value_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", XI_LIAN_VALUE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
xi_lian_value_obj* xi_lian_value_config::get_xi_lian_value_obj(const int suit_cid)
{ return this->impl_->get_xi_lian_value_obj(suit_cid); }
//----------------------------------------------------------------------------------------
/**
 * @class equip_jin_jie_cfg_impl
 * 
 * @brief implement of equip_jin_jie_cfg
 */
class equip_jin_jie_cfg_impl : public load_json
{
public:
  typedef std::map<int/*cid*/, equip_jin_jie_cfg_obj *> jin_jie_cfg_obj_map_t;
  typedef std::map<int/*cid*/, equip_jin_jie_cfg_obj *>::iterator jin_jie_cfg_obj_map_itor;

  equip_jin_jie_cfg_impl() { }
  ~equip_jin_jie_cfg_impl()
  {
    for (jin_jie_cfg_obj_map_itor itor = this->jin_jie_cfg_obj_map_.begin();
         itor != this->jin_jie_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_JIN_JIE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int equip_cid = ::atoi(iter.key().asCString());
      if (!item_config::instance()->find(equip_cid))
        return -1;
      equip_jin_jie_cfg_obj *p = new equip_jin_jie_cfg_obj();
      this->jin_jie_cfg_obj_map_.insert(std::make_pair(equip_cid, p));

      p->cost_ = (*iter)["cost"].asInt();
      p->to_equip_cid_ = (*iter)["result_cid"].asInt();
      if (!item_config::instance()->find(p->to_equip_cid_))
        return -1;
      Json::Value &info = (*iter)["info"];
      for(Json::Value::iterator i_iter = info.begin();
          i_iter != info.end();
          ++i_iter)
      {
        int cid = (*i_iter)["material_cid"].asInt();
        if (!item_config::instance()->find(cid))
          return -1;
        p->cai_liao_info_.push_back(pair_t<int>(cid, (*i_iter)["material_cnt"].asInt()));
      }
    }
    return 0;
  }
  equip_jin_jie_cfg_obj *get_jin_jie_info(const int equip_cid)
  {
    jin_jie_cfg_obj_map_itor itor = this->jin_jie_cfg_obj_map_.find(equip_cid);
    if (itor == this->jin_jie_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
private:
  jin_jie_cfg_obj_map_t jin_jie_cfg_obj_map_;
};
equip_jin_jie_cfg::equip_jin_jie_cfg() : impl_(new equip_jin_jie_cfg_impl()) { }
int equip_jin_jie_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_jin_jie_cfg::reload_config(const char *cfg_root)
{
  equip_jin_jie_cfg_impl *tmp_impl = new equip_jin_jie_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_JIN_JIE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
equip_jin_jie_cfg_obj *equip_jin_jie_cfg::get_jin_jie_info(const int cid)
{ return this->impl_->get_jin_jie_info(cid); }
