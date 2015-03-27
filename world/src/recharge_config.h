// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-16 17:35
 */
//========================================================================

#ifndef RECHARGE_CONFIG_H_
#define RECHARGE_CONFIG_H_

// Lib header
#include "singleton.h"

#include "ilist.h"
#include "global_macros.h"

#define RECHARGE_CFG_PATH                  "charge.json"

// 数据库存储长度127 支持12个类型各充值99999次。
#define MAX_RC_TYPE_CNT       12
#define RECHARGE_MONTHLY_CARD 1

// Forward declarations
class recharge_config_impl;

/**
 * @class recharge_cfg_obj
 *
 * @brief
 */
class recharge_cfg_obj
{
public:
  recharge_cfg_obj()
    : diamond_(0),
    b_diamond_1_(0),
    limit_cnt_(0),
    b_diamond_2_(0),
    charge_rmb_(0)
  { }

  int diamond_;
  int b_diamond_1_;
  int limit_cnt_;
  int b_diamond_2_;
  int charge_rmb_;
};
/**
 * @class recharge_config
 *
 * @brief
 */
class recharge_config : public singleton<recharge_config>
{
  friend class singleton<recharge_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  recharge_cfg_obj  *get_recharge_obj(const int recharge_id);
private:
  recharge_config();
  recharge_config(const recharge_config &);
  recharge_config& operator= (const recharge_config &);

  recharge_config_impl *impl_;
};

#endif  // RECHARGE_CONFIG_H_
