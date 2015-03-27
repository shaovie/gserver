// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-20 23:20
 */
//========================================================================

#ifndef LTIME_ACT_CFG_H_
#define LTIME_ACT_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class ltime_recharge_cfg_impl;

#define LTIME_RECHARGE_CFG_PATH                   "ltime_recharge_act.json"

enum
{
  LTIME_ACT_ACC_RECHARGE    = 1100, // 限时累积充值
  LTIME_ACT_DAILY_RECHARGE  = 1200, // 限时累积充值
  LTIME_ACT_ACC_CONSUME     = 1300, // 限时累积消费
  LTIME_ACT_DAILY_CONSUME   = 1400, // 限时每日消费

  LTIME_ACT_LUCKY_TURN      = 2001, // 限时幸运转盘
  LTIME_ACT_EXP             = 2002, // 限时双倍经验
  LTIME_ACT_DROP            = 2003, // 限时双倍掉落
  LTIME_ACT_CNT
};

class ltime_recharge_cfg_obj
{
public:
  int sub_id_;
  int param_1_;
  ilist<item_amount_bind_t> award_list_;
};
/**
 * @class ltime_recharge_cfg
 * 
 * @brief
 */
class ltime_recharge_cfg : public singleton<ltime_recharge_cfg>
{
  friend class singleton<ltime_recharge_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<ltime_recharge_cfg_obj *> *get_recharge_cfg(const int id);
private:
  ltime_recharge_cfg_impl *impl_;

  ltime_recharge_cfg();
};

#endif // LTIME_ACT_CFG_H_

