// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-05 18:05
 */
//========================================================================

#ifndef ACTIVITY_CFG_H_
#define ACTIVITY_CFG_H_

#include "def.h"
#include "ilist.h"

//Lib header
#include "singleton.h"

// Forward declarations
class activity_cfg_impl;

#define ACTIVITY_CFG_PATH                           "activity.json"

enum
{
  ACTIVITY_GHZ               = 1,  // 公会战
  ACTIVITY_AWARD_TI_LI       = 2,  // 送体力
  ACTIVITY_MONTH_CARD_AWARD  = 3,  // 月卡奖励
  ACTIVITY_XSZC              = 4,  // 雄狮战场
  ACTIVITY_GOODS_AWARD       = 5,  // 实物奖励
  ACTIVITY_KAI_FU            = 6,  // 开服活动
  ACTIVITY_CNT
};

/**
 * @class activity_cfg_obj
 *
 * @brief
 */
class activity_cfg_obj
{
public:
  activity_cfg_obj()
    : is_valid_(true),
    act_id_(0),
    lvl_(1)
  { }
  ~activity_cfg_obj() {}

  bool is_valid_;
  int  act_id_;            // activity id
  int  lvl_;
  ilist<int> week_list_;
  ilist<pair_t<itime> > time_list_;
};
/**
 * @class activity_cfg
 *
 * @brief
 */
class activity_cfg : public singleton<activity_cfg>
{
  friend class singleton<activity_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int open_lvl(const int act_id);

  activity_cfg_obj *get_act_cfg_obj(const int act_id);

  ilist<activity_cfg_obj *> *get_act_cfg_obj_list();

private:
  activity_cfg_impl *impl_;

  activity_cfg();
};
#endif // ACTIVITY_CFG_H_
