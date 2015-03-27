#include "vip_config.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "global_macros.h"
#include "title_cfg.h"
#include "def.h"

// Lib header
#include "array_t.h"
#include "ilist.h"

// Defines

#define WDAY_AMT 7
#define MAX_PURCHASE_CNT   64

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

/**
 * @class vip_config_impl
 *
 * @brief
 */
class vip_config_impl : public load_json
{
public:
  class cfg_obj
  {
  public:
    cfg_obj() :
      lvl_up_exp_(0),
      trade_npc_cid_(0),
      award_title_(0),
      effect_array_(VIP_EFF_END)
    { }
    ~cfg_obj()
    {
      for (int i = 0; i < this->effect_array_.size(); ++i)
      {
        pair_t<int> *obj = this->effect_array_.find(i);
        if (obj != NULL) delete obj;
      }
    }
  public:
    int lvl_up_exp_;
    int trade_npc_cid_;
    int award_title_;
    ilist<item_amount_bind_t> lvl_award_list_;
    array_t<pair_t<int> *> effect_array_;
    item_amount_bind_t login_awards_[WDAY_AMT + 1];
  };
public:
  vip_config_impl() :
    max_vip_lvl_(0),
    obj_map_(MAX_VIP_LVL + 1)
  { }
  ~vip_config_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      cfg_obj *obj = this->obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, VIP_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int lvl = ::atoi(itor.key().asCString());
      if (lvl <= 0 || lvl > MAX_VIP_LVL)
      {
        e_log->error("lvl error %d ", lvl);
        return -1;
      }
      this->max_vip_lvl_ < lvl ? this->max_vip_lvl_ = lvl : 0;

      cfg_obj *obj = new cfg_obj();
      this->obj_map_.insert(lvl, obj);
      obj->lvl_up_exp_ = (*itor)["exp"].asInt();
      obj->trade_npc_cid_ = (*itor)["npc_cid"].asInt();
      obj->award_title_ = (*itor)["title"].asInt();
      if (obj->award_title_ != 0
          && title_cfg::instance()->get_title_cfg_obj(obj->award_title_) == NULL)
      {
        e_log->rinfo("not found title %d", obj->award_title_);
        return -1;
      }

      {
        Json::Value &award_v = (*itor)["item_cid"];
        if (award_v.empty()) continue;
        char *tok_p = NULL;
        char *token = NULL;
        char bf[512] = {0};
        ::strncpy(bf, award_v.asCString(), sizeof(bf) - 1);
        for (token = ::strtok_r(bf, ";", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ";", &tok_p))
        {
          item_amount_bind_t item;
          int bind = 0;
          if (::sscanf(token, "%d,%d,%d", &item.cid_, &item.amount_, &bind) != 3)
            return -1;
          item.bind_ = bind;
          obj->lvl_award_list_.push_back(item);
          if (!item_config::instance()->find(item.cid_))
          {
            e_log->error("item_cid error %d ", item.cid_);
            return -1;
          }
        }
      }
      {
        Json::Value &award_v = (*itor)["item_login_cid"];
        if (award_v.empty()) continue;
        char *tok_p = NULL;
        char *token = NULL;
        char bf[512] = {0};
        ::strncpy(bf, award_v.asCString(), sizeof(bf) - 1);
        int i = 0;
        for (token = ::strtok_r(bf, ";", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ";", &tok_p))
        {
          ++i;
          if (i > WDAY_AMT)
          {
            e_log->error("day amt error %d ", i);
            return -1;
          }
          int bind = 0;
          if (::sscanf(token, "%d,%d,%d", &obj->login_awards_[i].cid_, &obj->login_awards_[i].amount_, &bind) != 3)
            return -1;
          obj->login_awards_[i].bind_ = bind;
          if (!item_config::instance()->find(obj->login_awards_[i].cid_))
          {
            e_log->error("item_cid error %d ", obj->login_awards_[i].cid_);
            return -1;
          }
        }
      }

      Json::Value &effect_v = (*itor)["info"];
      if (effect_v.empty()) continue;
      for(Json::Value::iterator itor2 = effect_v.begin();
          itor2 != effect_v.end();
          ++itor2)
      {
        int effect_id = ::atoi(itor2.key().asCString());
        if (effect_id <= 0 || effect_id >= VIP_EFF_END)
        {
          e_log->error("effect_id error %d ", effect_id);
          return -1;
        }
        pair_t<int> *arg = new pair_t<int>();
        obj->effect_array_.insert(effect_id, arg);
        arg->first_ = (*itor2)["param1"].asInt();
        arg->second_ = (*itor2)["param2"].asInt();
      }
    }
    return 0;
  }
  int lvl_up_exp(const char lvl)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->lvl_up_exp_;
  }
  int trade_npc_cid(const char lvl)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->trade_npc_cid_;
  }
  bool find_trade_npc(const int npc_cid)
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      cfg_obj *obj = this->obj_map_.find(i);
      if (obj != NULL
          && obj->trade_npc_cid_ == npc_cid)
        return true;
    }
    return false;
  }
  int max_vip_lvl()
  { return this->max_vip_lvl_; }
  int effect_val1(const char lvl, const char effect_id)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    pair_t<int> *arg = obj->effect_array_.find(effect_id);
    if (arg == NULL) return 0;
    return arg->first_;
  }
  int effect_val2(const char lvl, const char effect_id)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    pair_t<int> *arg = obj->effect_array_.find(effect_id);
    if (arg == NULL) return 0;
    return arg->second_;
  }
  ilist<item_amount_bind_t> *lvl_award_list(const char lvl)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return NULL;
    return &obj->lvl_award_list_;
  }
  item_amount_bind_t *login_award(const char lvl, const char day)
  {
    if (day <= 0 || day > WDAY_AMT) return NULL;
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return NULL;
    return &obj->login_awards_[(int)day];
  }
  int award_title(const char lvl)
  {
    cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->award_title_;
  }
private:
  char max_vip_lvl_;
  array_t<cfg_obj *> obj_map_;
};
vip_config::vip_config() : impl_(new vip_config_impl()) { }
int vip_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int vip_config::reload_config(const char *cfg_root)
{
  vip_config_impl *tmp_impl = new vip_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", VIP_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int vip_config::lvl_up_exp(const char lvl)
{ return this->impl_->lvl_up_exp(lvl); }
int vip_config::trade_npc_cid(const char lvl)
{ return this->impl_->trade_npc_cid(lvl); }
bool vip_config::find_trade_npc(const int npc_cid)
{ return this->impl_->find_trade_npc(npc_cid); }
char vip_config::max_vip_lvl()
{ return this->impl_->max_vip_lvl(); }
int vip_config::effect_val1(const char lvl, const char effect_id)
{ return this->impl_->effect_val1(lvl, effect_id); }
int vip_config::effect_val2(const char lvl, const char effect_id)
{ return this->impl_->effect_val2(lvl, effect_id); }
ilist<item_amount_bind_t> *vip_config::lvl_award_list(const char lvl)
{ return this->impl_->lvl_award_list(lvl); }
item_amount_bind_t *vip_config::login_award(const char lvl, const char day)
{ return this->impl_->login_award(lvl, day); }
int vip_config::award_title(const char lvl)
{ return this->impl_->award_title(lvl); }
/**
 * @class vip_purchase_cfg_impl
 *
 * @brief
 */
class vip_purchase_cfg_impl : public load_json
{
public:
  class vip_purchase_info
  {
  public:
    int ti_li_cost_;
    int ti_li_cnt_;
    int coin_cost_;
    int coin_cnt_;
  };
  vip_purchase_cfg_impl() :
    obj_map_(MAX_PURCHASE_CNT)
  { }
  ~vip_purchase_cfg_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      vip_purchase_info *obj = this->obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, VIP_PURCHASE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int cnt = ::atoi(itor.key().asCString());
      if (cnt > MAX_PURCHASE_CNT) return -1;
      Json::Value &jv = *itor;
      vip_purchase_info *vp = new vip_purchase_info();
      this->obj_map_.insert(cnt, vp);

      vp->ti_li_cost_ = jv["tili_cost"].asInt();
      vp->ti_li_cnt_ = jv["tili_cnt"].asInt();
      vp->coin_cost_ = jv["coin_cost"].asInt();
      vp->coin_cnt_ = jv["coin_cnt"].asInt();
    }
    return 0;
  }
  int ti_li(const int cnt, int &cost, int &amount)
  {
    vip_purchase_info *vp = this->obj_map_.find(cnt);
    if (vp == NULL) return -1;
    cost = vp->ti_li_cost_;
    amount = vp->ti_li_cnt_;
    return 0;
  }
  int coin(const int cnt, int &cost, int &amount)
  {
    vip_purchase_info *vp = this->obj_map_.find(cnt);
    if (vp == NULL) return -1;
    cost = vp->coin_cost_;
    amount = vp->coin_cnt_;
    return 0;
  }
private:
  array_t<vip_purchase_info *> obj_map_;
};
vip_purchase_cfg::vip_purchase_cfg() : impl_(new vip_purchase_cfg_impl()) { }
int vip_purchase_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int vip_purchase_cfg::reload_config(const char *cfg_root)
{
  vip_purchase_cfg_impl *tmp_impl = new vip_purchase_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", VIP_PURCHASE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int vip_purchase_cfg::ti_li(const int cnt, int &cost, int &amount)
{ return this->impl_->ti_li(cnt, cost, amount); }
int vip_purchase_cfg::coin(const int cnt, int &cost, int &amount)
{ return this->impl_->coin(cnt, cost, amount); }
