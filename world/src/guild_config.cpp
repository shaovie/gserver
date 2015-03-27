#include "guild_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"
#include "guild_member_info.h"

// Lib header
#include "ilist.h"
#include "array_t.h"
#include <tr1/unordered_map>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

#define GUILD_MAX_LVL 10
#define GUILD_MAX_POS 3
#define GUILD_SKILL_AMT 10
#define GUILD_SCP_YI_BO_MAX_REFRESH_CNT 50

/**
 * @class guild_lvl_config_impl
 *
 * @brief
 */
class guild_lvl_config_impl : public load_json
{
public:
  class guild_lvl_cfg_obj
  {
  public:
    guild_lvl_cfg_obj() :
      free_day_jb_cnt_(0),
      max_day_jb_cnt_(0),
      mem_amt_(0),
      dt_lvl_up_cost_(0),
      zq_lvl_up_cost_(0),
      tld_lvl_up_cost_(0),
      jbd_lvl_up_cost_(0),
      ju_bao_award_contrib_(0),
      ju_bao_award_item_(0),
      tu_long_dian_boss_(0),
      summon_boss_cost_(0),
      max_skill_lvl_(0),
      max_day_learn_skill_cnt_(0),
      scp_lvl_up_cost_(0),
      scp_award_per_(0)
    { }
  public:
    char free_day_jb_cnt_;
    char max_day_jb_cnt_;
    short mem_amt_;
    int dt_lvl_up_cost_;
    int zq_lvl_up_cost_;
    int tld_lvl_up_cost_;
    int jbd_lvl_up_cost_;
    int ju_bao_award_contrib_;
    int ju_bao_award_item_;
    int tu_long_dian_boss_;
    int summon_boss_cost_;
    int max_skill_lvl_;
    int max_day_learn_skill_cnt_;
    int scp_lvl_up_cost_;
    int scp_award_per_;
  };
public:
  guild_lvl_config_impl() :
    obj_map_(GUILD_MAX_LVL + 1)
  { }
  ~guild_lvl_config_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      guild_lvl_cfg_obj *obj = this->obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GUILD_LVL_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int lvl = ::atoi(itor.key().asCString());
      if (lvl <= 0 || lvl > GUILD_MAX_LVL)
      {
        e_log->error("lvl error %d ", lvl);
        return -1;
      }
      guild_lvl_cfg_obj *obj = new guild_lvl_cfg_obj();
      this->obj_map_.insert(lvl, obj);
      obj->mem_amt_                 = (*itor)["limit_count"].asInt();
      obj->dt_lvl_up_cost_          = (*itor)["dt_lvl_up_cost"].asInt();
      obj->zq_lvl_up_cost_          = (*itor)["zq_lvl_up_cost"].asInt();
      obj->tld_lvl_up_cost_         = (*itor)["tld_lvl_up_cost"].asInt();
      obj->jbd_lvl_up_cost_         = (*itor)["jbd_lvl_up_cost"].asInt();
      obj->free_day_jb_cnt_         = (*itor)["free_day_jb_cnt"].asInt();
      obj->max_day_jb_cnt_          = (*itor)["max_day_jb_cnt"].asInt();
      obj->ju_bao_award_item_       = (*itor)["ju_bao_award_item"].asInt();
      obj->ju_bao_award_contrib_    = (*itor)["ju_bao_award_contrib"].asInt();
      obj->tu_long_dian_boss_       = (*itor)["tu_long_dian_boss"].asInt();
      obj->summon_boss_cost_        = (*itor)["zhao_huan_cost"].asInt();
      obj->max_day_learn_skill_cnt_ = (*itor)["learn_times"].asInt();
      obj->max_skill_lvl_           = (*itor)["limit_skill_lvl"].asInt();
      obj->scp_lvl_up_cost_         = (*itor)["fb_lvl_up_cost"].asInt();
      obj->scp_award_per_           = (*itor)["fb_award_factor"].asInt();
    }
    return 0;
  }
  int mem_capacity(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->mem_amt_;
  }
  int dt_lvl_up_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->dt_lvl_up_cost_;
  }
  int zq_lvl_up_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->zq_lvl_up_cost_;
  }
  int tld_lvl_up_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->tld_lvl_up_cost_;
  }
  int jbd_lvl_up_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->jbd_lvl_up_cost_;
  }
  char free_day_jb_cnt(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->free_day_jb_cnt_;
  }
  char max_day_jb_cnt(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->max_day_jb_cnt_;
  }
  int ju_bao_award_contrib(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->ju_bao_award_contrib_;
  }
  int ju_bao_award_item(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->ju_bao_award_item_;
  }
  int tu_long_dian_boss(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->tu_long_dian_boss_;
  }
  int summon_boss_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->summon_boss_cost_;
  }
  int max_day_learn_skill_cnt(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->max_day_learn_skill_cnt_;
  }
  bool is_lvl_valid(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return false;
    return true;
  }
  int max_skill_lvl(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->max_skill_lvl_;
  }
  int scp_lvl_up_cost(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return MAX_INVALID_INT;
    return obj->scp_lvl_up_cost_;
  }
  int scp_award_per(const char lvl)
  {
    guild_lvl_cfg_obj *obj = this->obj_map_.find(lvl);
    if (obj == NULL) return 0;
    return obj->scp_award_per_;
  }
private:
  array_t<guild_lvl_cfg_obj *> obj_map_;
};
guild_lvl_config::guild_lvl_config() : impl_(new guild_lvl_config_impl()) { }
int guild_lvl_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int guild_lvl_config::reload_config(const char *cfg_root)
{
  guild_lvl_config_impl *tmp_impl = new guild_lvl_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GUILD_LVL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int  guild_lvl_config::mem_capacity(const char lvl)
{ return this->impl_->mem_capacity(lvl); }
int guild_lvl_config::dt_lvl_up_cost(const char lvl)
{ return this->impl_->dt_lvl_up_cost(lvl); }
int guild_lvl_config::zq_lvl_up_cost(const char lvl)
{ return this->impl_->zq_lvl_up_cost(lvl); }
int guild_lvl_config::tld_lvl_up_cost(const char lvl)
{ return this->impl_->tld_lvl_up_cost(lvl); }
int guild_lvl_config::jbd_lvl_up_cost(const char lvl)
{ return this->impl_->jbd_lvl_up_cost(lvl); }
char guild_lvl_config::free_day_jb_cnt(const char lvl)
{ return this->impl_->free_day_jb_cnt(lvl); }
char guild_lvl_config::max_day_jb_cnt(const char lvl)
{ return this->impl_->max_day_jb_cnt(lvl); }
int guild_lvl_config::ju_bao_award_contrib(const char lvl)
{ return this->impl_->ju_bao_award_contrib(lvl); }
int guild_lvl_config::ju_bao_award_item(const char lvl)
{ return this->impl_->ju_bao_award_item(lvl); }
int guild_lvl_config::tu_long_dian_boss(const char lvl)
{ return this->impl_->tu_long_dian_boss(lvl); }
int guild_lvl_config::summon_boss_cost(const char lvl)
{ return this->impl_->summon_boss_cost(lvl); }
int guild_lvl_config::max_day_learn_skill_cnt(const char lvl)
{ return this->impl_->max_day_learn_skill_cnt(lvl); }
bool guild_lvl_config::is_lvl_valid(const char lvl)
{ return this->impl_->is_lvl_valid(lvl); }
int guild_lvl_config::max_skill_lvl(const char lvl)
{ return this->impl_->max_skill_lvl(lvl); }
int guild_lvl_config::scp_lvl_up_cost(const char lvl)
{ return this->impl_->scp_lvl_up_cost(lvl); }
int guild_lvl_config::scp_award_per(const char lvl)
{ return this->impl_->scp_award_per(lvl); }

/**
 * @class guild_pos_config_impl
 *
 * @brief
 */
class guild_pos_config_impl : public load_json
{
public:
  enum power
  {
    POWER_AGREE_JION      = 1, // 同意加入
    POWER_SET_APPLY       = 2, // 设置加入条件
    POWER_SET_PURPOSE     = 3, // 设置公告
    POWER_UP_BULIDING_LVL = 4, // 提升建筑物等级
    POWER_SUMMON_BOSS     = 5, // 召唤神兽
    POWER_OPEN_SCP        = 6, // 开启副本
    POWER_SUMMON_MEM      = 7, // 召唤成员
    POWER_END,                 // 高职位直接能设置、开除低职位
  };
  class guild_pos_cfg_obj
  {
  public:
    guild_pos_cfg_obj() :
      pos_amt_(0)
    { }
  public:
    char pos_amt_;
    ilist<int> power_list_;
  };
public:
  guild_pos_config_impl() :
    obj_map_(GUILD_MAX_POS + 1)
  { }
  ~guild_pos_config_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      guild_pos_cfg_obj *obj = obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GUILD_POS_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int pos = ::atoi(itor.key().asCString());
      if (pos <= guild_member_info::POS_NULL
          || pos >= guild_member_info::POS_END)
      {
        e_log->error("pos error %d ", pos);
        return -1;
      }
      guild_pos_cfg_obj *obj = new guild_pos_cfg_obj();
      this->obj_map_.insert(pos, obj);
      obj->pos_amt_  = (*itor)["count"].asInt();
      Json::Value &power_v = (*itor)["power"];
      if (!power_v.empty())
      {
        char *tok_p = NULL;
        char *token = NULL;
        char bf[128] = {0};
        ::strncpy(bf, power_v.asCString(), sizeof(bf) - 1);
        for (token = ::strtok_r(bf, ",", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ",", &tok_p))
          obj->power_list_.push_back(::atoi(token));
      }
    }
    return 0;
  }
  bool had_right_to_agree_join(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_AGREE_JION);
  }
  bool had_right_to_set_pos(const char tar_pos, const char pos)
  { return pos < tar_pos; }
  bool had_right_to_expel_mem(const char tar_pos, const char pos)
  { return pos < tar_pos; }
  bool had_right_to_set_apply(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_SET_APPLY);
  }
  bool had_right_to_set_purpose(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_SET_PURPOSE);
  }
  bool pos_full(const char pos, const char amt)
  {
    if (pos == guild_member_info::POS_HY)
      return false;
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return true;
    return amt >= obj->pos_amt_;
  }
  bool had_right_to_up_buliding_lvl(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_UP_BULIDING_LVL);
  }
  bool had_right_to_summon_boss(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_SUMMON_BOSS);
  }
  bool had_right_to_open_scp(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_OPEN_SCP);
  }
  bool had_right_to_summon_mem(const char pos)
  {
    guild_pos_cfg_obj *obj = this->obj_map_.find(pos);
    if (obj == NULL) return false;
    return obj->power_list_.find(POWER_SUMMON_MEM);
  }
private:
  array_t<guild_pos_cfg_obj *> obj_map_;
};
guild_pos_config::guild_pos_config() : impl_(new guild_pos_config_impl()) { }
int guild_pos_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int guild_pos_config::reload_config(const char *cfg_root)
{
  guild_pos_config_impl *tmp_impl = new guild_pos_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GUILD_POS_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
bool guild_pos_config::had_right_to_agree_join(const char pos)
{ return this->impl_->had_right_to_agree_join(pos); }
bool guild_pos_config::had_right_to_set_pos(const char tar_pos, const char pos)
{ return this->impl_->had_right_to_set_pos(tar_pos, pos); }
bool guild_pos_config::had_right_to_expel_mem(const char tar_pos, const char pos)
{ return this->impl_->had_right_to_expel_mem(tar_pos, pos); }
bool guild_pos_config::had_right_to_set_apply(const char pos)
{ return this->impl_->had_right_to_set_apply(pos); }
bool guild_pos_config::had_right_to_set_purpose(const char pos)
{ return this->impl_->had_right_to_set_purpose(pos); }
bool guild_pos_config::pos_full(const char pos, const char amt)
{ return this->impl_->pos_full(pos, amt); }
bool guild_pos_config::had_right_to_up_buliding_lvl(const char pos)
{ return this->impl_->had_right_to_up_buliding_lvl(pos); }
bool guild_pos_config::had_right_to_summon_boss(const char pos)
{ return this->impl_->had_right_to_summon_boss(pos); }
bool guild_pos_config::had_right_to_open_scp(const char pos)
{ return this->impl_->had_right_to_open_scp(pos); }
bool guild_pos_config::had_right_to_summon_mem(const char pos)
{ return this->impl_->had_right_to_summon_mem(pos); }

/**
 * @class guild_skill_config
 *
 * @brief
 */
class guild_skill_config_impl : public load_json
{
public:
  class guild_skill_cfg_obj
  {
  public:
    guild_skill_cfg_obj() :
      zhan_qi_lvl_(0),
      effect_id_(0),
      effect_val_ini_(0),
      effect_val_add_(0),
      cost_ini_(0),
      cost_add_(0)
    { }
  public:
    char zhan_qi_lvl_;
    int effect_id_;
    int effect_val_ini_;
    int effect_val_add_;
    int cost_ini_;
    int cost_add_;
  };
public:
  typedef std::tr1::unordered_map<int/*cid*/, guild_skill_cfg_obj *> obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, guild_skill_cfg_obj *>::iterator obj_map_iter;
public:
  guild_skill_config_impl() :
    obj_map_(GUILD_SKILL_AMT)
  { }
  ~guild_skill_config_impl()
  {
    for (obj_map_iter itor = this->obj_map_.begin();
         itor != this->obj_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GUILD_SKILL_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int skill_cid = ::atoi(itor.key().asCString());
      guild_skill_cfg_obj *obj = new guild_skill_cfg_obj();
      this->obj_map_.insert(std::make_pair(skill_cid, obj));
      obj->zhan_qi_lvl_    = (*itor)["zq_lvl"].asInt();
      obj->effect_id_      = (*itor)["effect"].asInt();
      obj->effect_val_ini_ = (*itor)["val_ini"].asInt();
      obj->effect_val_add_ = (*itor)["val_add"].asInt();
      obj->cost_ini_       = (*itor)["cost_ini"].asInt();
      obj->cost_add_       = (*itor)["cost_add"].asInt();
    }
    return 0;
  }
  int effect_id(const int skill_cid)
  {
    obj_map_iter itor = this->obj_map_.find(skill_cid);
    if (itor == this->obj_map_.end()) return 0;
    return itor->second->effect_id_;
  }
  int effect_val(const int skill_cid, const short skill_lvl)
  {
    obj_map_iter itor = this->obj_map_.find(skill_cid);
    if (itor == this->obj_map_.end()) return 0;
    return itor->second->effect_val_ini_ + (skill_lvl - 1) * itor->second->effect_val_add_;
  }
  int min_zhan_qi_lvl(const int skill_cid)
  {
    obj_map_iter itor = this->obj_map_.find(skill_cid);
    if (itor == this->obj_map_.end()) return 0;
    return itor->second->zhan_qi_lvl_;
  }
  int learn_cost(const int skill_cid, const short skill_lvl)
  {
    obj_map_iter itor = this->obj_map_.find(skill_cid);
    if (itor == this->obj_map_.end()) return 0;
    return itor->second->cost_ini_ + (skill_lvl - 1) * itor->second->cost_add_;
  }
private:
  obj_map_t obj_map_;
};
guild_skill_config::guild_skill_config() : impl_(new guild_skill_config_impl()) { }
int guild_skill_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int guild_skill_config::reload_config(const char *cfg_root)
{
  guild_skill_config_impl *tmp_impl = new guild_skill_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GUILD_SKILL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int guild_skill_config::effect_id(const int skill_cid)
{ return this->impl_->effect_id(skill_cid); }
int guild_skill_config::effect_val(const int skill_cid, const short skill_lvl)
{ return this->impl_->effect_val(skill_cid, skill_lvl); }
int guild_skill_config::min_zhan_qi_lvl(const int skill_cid)
{ return this->impl_->min_zhan_qi_lvl(skill_cid); }
int guild_skill_config::learn_cost(const int skill_cid, const short skill_lvl)
{ return this->impl_->learn_cost(skill_cid, skill_lvl); }
/**
 * @class guild_scp_config
 *
 * @brief
 */
class guild_scp_config_impl : public load_json
{
public:
  class cfg_obj
  {
  public:
    cfg_obj() :
      one_time_(0),
      all_time_(0),
      award_resource_(0),
      award_contrib_(0),
      mst_info_(GUILD_SCP_YI_BO_MAX_REFRESH_CNT + 1)
    { }
  public:
    int one_time_;
    int all_time_;
    int award_resource_;
    int award_contrib_;
    array_t<pair_t<int> *> mst_info_;
    ilist<coord_t> mst_refresh_pos_;
  };
public:
  guild_scp_config_impl() :
    yi_bo_info_(SCP_DFF_END)
  { }
  ~guild_scp_config_impl()
  {
    for (int i = 0; i < this->yi_bo_info_.size(); ++i)
    {
      array_t<cfg_obj *> *obj = this->yi_bo_info_.find(i);
      if (obj == NULL) continue;
      for (int j = 0; j < obj->size(); ++j)
      {
        cfg_obj *sub_obj = obj->find(j);
        if (sub_obj == NULL) continue;
        for (int k = 0; k < sub_obj->mst_info_.size(); ++k)
        {
          pair_t<int> *ss_obj = sub_obj->mst_info_.find(k);
          if (ss_obj != NULL) delete ss_obj;
        }
        delete sub_obj;
      }
      delete obj;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GUILD_SCP_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int scp_dff = ::atoi(itor.key().asCString());
      if (scp_dff >= SCP_DFF_END)
      {
        e_log->error("scp_dff error %d ", scp_dff);
        return -1;
      }
      array_t<cfg_obj *> *array = new array_t<cfg_obj *>(GUILD_SCP_YI_BO_MAX_REFRESH_CNT + 1);
      this->yi_bo_info_.insert(scp_dff, array);

      for(Json::Value::iterator itor2 = (*itor).begin();
          itor2 != (*itor).end();
          ++itor2)
      {
        int scp_bo_shu = ::atoi(itor2.key().asCString());
        if (scp_bo_shu > GUILD_SCP_YI_BO_MAX_REFRESH_CNT)
        {
          e_log->error("scp_bo_shu error %d ", scp_bo_shu);
          return -1;
        }
        cfg_obj *obj = new cfg_obj();
        array->insert(scp_bo_shu, obj);
        obj->one_time_       = (*itor2)["type_time"].asInt();
        obj->all_time_       = (*itor2)["time"].asInt();
        obj->award_resource_ = (*itor2)["guild_zijin"].asInt();
        obj->award_contrib_  = (*itor2)["guild_gongxian"].asInt();

        {
          Json::Value &info_v = (*itor2)["mst_info"];
          if (info_v.empty()) continue;
          char *tok_p = NULL;
          char *token = NULL;
          char bf[256] = {0};
          ::strncpy(bf, info_v.asCString(), sizeof(bf) - 1);
          int i = 0;
          for (token = ::strtok_r(bf, ";", &tok_p);
               token != NULL;
               token = ::strtok_r(NULL, ";", &tok_p))
          {
            ++i;
            if (i > GUILD_SCP_YI_BO_MAX_REFRESH_CNT)
            {
              e_log->error("mst bo shu error %d ", i);
              return -1;
            }
            int cid = 0, amt = 0;
            if (::sscanf(token, "%d,%d", &cid, &amt) != 2)
              return -1;
            obj->mst_info_.insert(i, new pair_t<int>(cid, amt));
          }
        }
        {
          Json::Value &info_v = (*itor2)["pos_info"];
          if (info_v.empty()) continue;
          char *tok_p = NULL;
          char *token = NULL;
          char bf[256] = {0};
          ::strncpy(bf, info_v.asCString(), sizeof(bf) - 1);
          int i = 0;
          for (token = ::strtok_r(bf, ";", &tok_p);
               token != NULL;
               token = ::strtok_r(NULL, ";", &tok_p))
          {
            ++i;
            if (i > GUILD_SCP_YI_BO_MAX_REFRESH_CNT)
            {
              e_log->error("mst bo shu error %d ", i);
              return -1;
            }
            int x = 0, y = 0;
            if (::sscanf(token, "%d,%d", &x, &y) != 2)
              return -1;
            obj->mst_refresh_pos_.push_back(coord_t(x, y));
          }
        }
        {
          Json::Value &info_v = (*itor2)["target_pos"];
          if (info_v.empty()) continue;
          int x = 0, y = 0;
          if (::sscanf(info_v.asCString(), "%d,%d", &x, &y) != 2)
            return -1;
          this->mst_target_pos_.set(x, y);
        }
      }
    }
    return 0;
  }
  ilist<coord_t> *mst_refresh_pos(const char dff, const char bo_shu)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return NULL;

    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return NULL;
    return &obj->mst_refresh_pos_;
  }
  coord_t &target_pos()
  { return this->mst_target_pos_; }
  int over_all_time(const char dff, const char bo_shu)
  {
    // need change?
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return MAX_INVALID_INT;

    int all_time = 0;
    for (int i = 1; i <= bo_shu; ++i)
    {
      cfg_obj *obj = array->find(i);
      if (obj == NULL) return MAX_INVALID_INT;
      all_time += obj->all_time_;
    }
    return all_time == 0 ? MAX_INVALID_INT : all_time;
  }
  int over_all_time(const char dff, const char bo_shu, const char batch)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return MAX_INVALID_INT;

    int all_time = 0;
    for (int i = 1; i < bo_shu; ++i)
    {
      cfg_obj *obj = array->find(i);
      if (obj == NULL) return MAX_INVALID_INT;
      all_time += obj->all_time_;
    }
    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return MAX_INVALID_INT;
    all_time += obj->one_time_ * batch;
    return all_time == 0 ? MAX_INVALID_INT : all_time;
  }
  int award_resource(const char dff, const char bo_shu)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return 0;

    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return 0;
    return obj->award_resource_;
  }
  int award_contrib(const char dff, const char bo_shu)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return 0;

    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return 0;
    return obj->award_contrib_;
  }
  int refresh_mst_cid(const char dff, const char bo_shu, const char batch)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return 0;
    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return 0;
    pair_t<int> *pt = obj->mst_info_.find(batch);
    if (pt == NULL) return 0;
    return pt->first_;
  }
  int refresh_mst_amt(const char dff, const char bo_shu, const char batch)
  {
    array_t<cfg_obj *> *array = this->yi_bo_info_.find(dff);
    if (array == NULL) return 0;
    cfg_obj *obj = array->find(bo_shu);
    if (obj == NULL) return 0;
    pair_t<int> *pt = obj->mst_info_.find(batch);
    if (pt == NULL) return 0;
    return pt->second_;
  }
private:
  coord_t mst_target_pos_;
  array_t<array_t<cfg_obj *> *> yi_bo_info_;
};
guild_scp_config::guild_scp_config() : impl_(new guild_scp_config_impl()) { }
int guild_scp_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int guild_scp_config::reload_config(const char *cfg_root)
{
  guild_scp_config_impl *tmp_impl = new guild_scp_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", GUILD_SCP_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<coord_t> *guild_scp_config::mst_refresh_pos(const char dff, const char bo_shu)
{ return this->impl_->mst_refresh_pos(dff, bo_shu); }
coord_t &guild_scp_config::target_pos()
{ return this->impl_->target_pos(); }
int guild_scp_config::over_all_time(const char dff, const char bo_shu)
{ return this->impl_->over_all_time(dff, bo_shu); }
int guild_scp_config::over_all_time(const char dff, const char bo_shu, const char batch)
{ return this->impl_->over_all_time(dff, bo_shu, batch); }
int guild_scp_config::award_resource(const char dff, const char bo_shu)
{ return this->impl_->award_resource(dff, bo_shu); }
int guild_scp_config::award_contrib(const char dff, const char bo_shu)
{ return this->impl_->award_contrib(dff, bo_shu); }
int guild_scp_config::refresh_mst_cid(const char dff, const char bo_shu, const char batch)
{ return this->impl_->refresh_mst_cid(dff, bo_shu, batch); }
int guild_scp_config::refresh_mst_amt(const char dff, const char bo_shu, const char batch)
{ return this->impl_->refresh_mst_amt(dff, bo_shu, batch); }
