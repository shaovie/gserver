// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-26 10:40
 */
//========================================================================

#ifndef SKILL_CONFIG_H_
#define SKILL_CONFIG_H_

#include "def.h"
#include "ilist.h"

// Lib header
#include "singleton.h"

// Forward declarations
class skill_config_impl;

#define MAX_SKILL_LEVEL                   32
#define SKILL_CFG_PATH                    "skill.json"

enum sk_buff_target_t
{
  SK_BUFF_TARGET_SELF       = 1,
  SK_BUFF_TARGET_SINGLE     = 2,
  SK_BUFF_TARGET_AREA       = 3,
};
enum
{
  LI_LIANG_COMMON_SKILL_1   = 31110001,
  LI_LIANG_COMMON_SKILL_2   = 31110002,
  LI_LIANG_COMMON_SKILL_3   = 31110003,

  MIN_JIE_COMMON_SKILL_1    = 31210001,
  MIN_JIE_COMMON_SKILL_2    = 31210002,
  MIN_JIE_COMMON_SKILL_3    = 31210003,
  MIN_JIE_BIAN_SHEN_COMMON_SKILL = 31210004,

  ZHI_LI_COMMON_SKILL_1     = 31310001,
  ZHI_LI_COMMON_SKILL_2     = 31310002,
  ZHI_LI_COMMON_SKILL_3     = 31310003,
};
class sk_rel_buff_info
{
public:
  sk_rel_buff_info() :
    id_(0),
    rate_(0),
    target_type_(0)
  { }
  sk_rel_buff_info(const sk_rel_buff_info &v)
  {
    this->id_    = v.id_;
    this->rate_  = v.rate_;
    this->target_type_ = v.target_type_;
  }
  sk_rel_buff_info &operator = (const sk_rel_buff_info &v)
  {
    if (this != &v)
    {
      this->id_    = v.id_;
      this->rate_  = v.rate_;
      this->target_type_ = v.target_type_;
    }
    return *this;
  }

  int id_;
  int rate_;
  int target_type_;
};
/**
 * @class skill_detail
 * 
 * @brief
 */
class skill_detail
{
public:
  skill_detail() :
    cur_lvl_(1),
    lvl_(1),
    distance_(0),
    cd_(0),
    coin_(0),
    mp_(0),
    hate_coe_(0),
    hurt_percent_(100),
    add_fixed_hurt_(0),
    back_dis_(0),
    param_1_(0),
    param_2_(0),
    param_3_(0)
  { }

  short cur_lvl_;
  short lvl_;
  short distance_;
  int cd_;
  int coin_;
  int mp_;
  int hate_coe_;
  int hurt_percent_;
  int add_fixed_hurt_;
  int back_dis_;
  int param_1_;
  int param_2_;
  int param_3_;
  ilist<sk_rel_buff_info> buff_info_;
};
/**
 * @class skill_cfg_obj
 * 
 * @brief 
 */
class skill_cfg_obj
{
public:
  skill_cfg_obj();
  ~skill_cfg_obj();

  const skill_detail *get_detail(const int lvl) const
  {
    if (lvl > 0 && lvl < MAX_SKILL_LEVEL)
      return this->details_[lvl];
    return NULL;
  }
public:
  int skill_cid_;
  int career_;
  int hurt_delay_;
  skill_detail **details_;
};
/**
 * @class skill_config
 * 
 * @brief
 */
class skill_config : public singleton<skill_config>
{
  friend class singleton<skill_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const skill_cfg_obj *get_skill(const int skill_cid);
  const skill_detail *get_detail(const int skill_cid, const short lvl);
  ilist<int> *get_skill_list(const int career);
private:
  skill_config();
  skill_config(const skill_config &);
  skill_config& operator= (const skill_config &);

  skill_config_impl *impl_;
};

#endif // SKILL_CONFIG_H_

