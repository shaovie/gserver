#include "bao_shi_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "def.h"
#include "global_macros.h"

// Lib header
#include "array_t.h"
#include "ilist.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

/**
 * @class bao_shi_lvl_cfg_impl
 *
 * @brief
 */
class bao_shi_lvl_cfg_impl : public load_json
{
public:
  class cfg_obj
  {
  public:
    cfg_obj() :
      cost_cid_(0),
      cost_amt_(0),
      cost_money_(0),
      lvl_up_exp_(0),
      total_exp_(0),
      luck_rate_(0)
    { }
  public:
    int cost_cid_;
    int cost_amt_;
    int cost_money_;
    int lvl_up_exp_;
    int total_exp_;
    int luck_rate_;
    ilist<pair_t<int> > rand_exp_; // first:rand, second:exp
  };
public:
  bao_shi_lvl_cfg_impl() :
    max_lvl_(0),
    lvl_map_(MAX_BAO_SHI_POS_IDX_LVL + 1)
  { }
  ~bao_shi_lvl_cfg_impl()
  {
    for (int i = 0; i < this->lvl_map_.size(); ++i)
    {
      cfg_obj *obj = this->lvl_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BAO_SHI_LVL_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int lvl = ::atoi(itor.key().asCString());
      this->max_lvl_ < lvl ? this->max_lvl_ = lvl : 0;
      cfg_obj *obj = new cfg_obj();
      this->lvl_map_.insert(lvl, obj);
      obj->cost_cid_   =(*itor)["item_cid"].asInt();
      obj->cost_amt_   =(*itor)["item_cnt"].asInt();
      obj->cost_money_ =(*itor)["cost"].asInt();
      obj->lvl_up_exp_ =(*itor)["exp"].asInt();
      obj->luck_rate_  =(*itor)["lucky_rate"].asInt();

      Json::Value &award_v = (*itor)["p_info"];
      if (award_v.empty()) continue;
      char *tok_p = NULL;
      char *token = NULL;
      char bf[256] = {0};
      ::strncpy(bf, award_v.asCString(), sizeof(bf) - 1);
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        pair_t<int> ran;
        ::sscanf(token, "%d:%d", &ran.first_, &ran.second_);
        obj->rand_exp_.push_back(ran);
        obj->total_exp_ += ran.first_;
      }
    }
    return 0;
  }
  bool is_vaild_lvl(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return false;
    return true;
  }
  int lvl_cost_cid(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->cost_cid_;
  }
  int lvl_cost_amt(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->cost_amt_;
  }
  int lvl_cost_money(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->cost_money_;
  }
  int lvl_rand_add_exp(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL
        || obj->total_exp_ == 0)
      return 0;
    const int rnd = rand() % obj->total_exp_ + 1;
    int acc = 0;
    for (ilist_node<pair_t<int> > *itor = obj->rand_exp_.head();
         itor != NULL;
         itor = itor->next_)
    {
      acc += itor->value_.first_;
      pair_t<int> pair = itor->value_;
      if (acc >= rnd)
        return itor->value_.second_;
    }
    return 0;
  }
  int lvl_up_exp(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->lvl_up_exp_;
  }
  int luck_rate(const short lvl)
  {
    cfg_obj *obj = this->lvl_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->luck_rate_;
  }
  short max_lvl()
  { return this->max_lvl_; }
private:
  short max_lvl_;
  array_t<cfg_obj *> lvl_map_;
};
bao_shi_lvl_cfg::bao_shi_lvl_cfg() : impl_(new bao_shi_lvl_cfg_impl()) { }
int bao_shi_lvl_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int bao_shi_lvl_cfg::reload_config(const char *cfg_root)
{
  bao_shi_lvl_cfg_impl *tmp_impl = new bao_shi_lvl_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", BAO_SHI_LVL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
bool bao_shi_lvl_cfg::is_vaild_lvl(const short lvl)
{ return this->impl_->is_vaild_lvl(lvl); }
int bao_shi_lvl_cfg::lvl_cost_cid(const short lvl)
{ return this->impl_->lvl_cost_cid(lvl); }
int bao_shi_lvl_cfg::lvl_cost_amt(const short lvl)
{ return this->impl_->lvl_cost_amt(lvl); }
int bao_shi_lvl_cfg::lvl_cost_money(const short lvl)
{ return this->impl_->lvl_cost_money(lvl); }
int bao_shi_lvl_cfg::lvl_rand_add_exp(const short lvl)
{ return this->impl_->lvl_rand_add_exp(lvl); }
int bao_shi_lvl_cfg::lvl_up_exp(const short lvl)
{ return this->impl_->lvl_up_exp(lvl); }
int bao_shi_lvl_cfg::luck_rate(const short lvl)
{ return this->impl_->luck_rate(lvl); }
short bao_shi_lvl_cfg::max_lvl()
{ return this->impl_->max_lvl(); }
/**
 * @class bao_shi_eff_cfg_impl
 *
 * @brief
 */
class bao_shi_eff_cfg_impl : public load_json
{
public:
  bao_shi_eff_cfg_impl()
  {
    for (int i = PART_XXX; i < PART_END; ++i)
      eff_map_[i].set(0, 0);
  }
  ~bao_shi_eff_cfg_impl()
  { }
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BAO_SHI_EFF_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int pos = ::atoi(itor.key().asCString());
      if (pos <= PART_XXX
          || pos >= PART_END)
      {
        e_log->error("err pos %d ", pos);
        return -1;
      }
      this->eff_map_[pos].first_ = (*itor)["effect"].asInt();
      this->eff_map_[pos].second_ = (*itor)["val"].asInt();
      if (this->eff_map_[pos].first_ <= ATTR_T_XXX
          || this->eff_map_[pos].first_ >= ATTR_T_ITEM_CNT)
      {
        e_log->error("err id %d ", this->eff_map_[pos].first_);
        return -1;
      }
    }
    return 0;
  }
  char pos_eff_id(const char pos)
  {
    if (pos <= PART_XXX
        || pos >= PART_END)
      return ATTR_T_XXX;
    return this->eff_map_[(int)pos].first_;
  }
  int eff_init_val(const short pos)
  {
    if (pos <= PART_XXX
        || pos >= PART_END)
      return 0;
    return this->eff_map_[(int)pos].second_;
  }
private:
  pair_t<int> eff_map_[PART_END]; // first:eff_id second:eff_init_val
};
bao_shi_eff_cfg::bao_shi_eff_cfg() : impl_(new bao_shi_eff_cfg_impl()) { }
int bao_shi_eff_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int bao_shi_eff_cfg::reload_config(const char *cfg_root)
{
  bao_shi_eff_cfg_impl *tmp_impl = new bao_shi_eff_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", BAO_SHI_EFF_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
char bao_shi_eff_cfg::pos_eff_id(const char pos)
{ return this->impl_->pos_eff_id(pos); }
int bao_shi_eff_cfg::eff_init_val(const char pos)
{ return this->impl_->eff_init_val(pos); }
