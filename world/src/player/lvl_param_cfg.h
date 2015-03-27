// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-23 17:47
 */
//========================================================================

#ifndef LVL_PARAM_CFG_H_
#define LVL_PARAM_CFG_H_

#include "singleton.h"

// Lib header
#include <stdint.h>

// Forward declarations
class lvl_param_cfg_impl;

#define LVL_PARAM_CFG_PATH                   "lvl_param.json"
#define LVL_PARAM_COE_CHU_SHU                 1000.0

/**
 * @class lvl_param_cfg
 *
 * @brief
 */
class lvl_param_cfg : public singleton<lvl_param_cfg>
{
  friend class singleton<lvl_param_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int64_t lvl_up_exp(const short lvl);
  double attr_param(const short lvl);
  int64_t worship_exp(const short lvl);
  int64_t mission_exp(const short lvl);
  int worship_coin(const short lvl);
  int guangai_exp(const short lvl);
  int guangai_coin(const short lvl);
  int rob_exp(const short lvl);
  int rob_coin(const short lvl);
private:
  lvl_param_cfg();
  lvl_param_cfg(const lvl_param_cfg &);
  lvl_param_cfg& operator= (const lvl_param_cfg &);

  lvl_param_cfg_impl *impl_;
};

#endif // LVL_PARAM_CFG_H_

