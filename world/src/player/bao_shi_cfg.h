// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-24 17:50
 */
//========================================================================

#ifndef BAO_SHI_CONFIG_H_
#define BAO_SHI_CONFIG_H_

// Lib header
#include "singleton.h"

#define BAO_SHI_LVL_CFG_PATH                        "gem_upgrade.json"
#define BAO_SHI_EFF_CFG_PATH                        "gem_val.json"

// Forward declarations
class bao_shi_lvl_cfg_impl;
class bao_shi_eff_cfg_impl;

/**
 * @class bao_shi_lvl_cfg
 *
 * @brief
 */
class bao_shi_lvl_cfg : public singleton<bao_shi_lvl_cfg>
{
  friend class singleton<bao_shi_lvl_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  bool is_vaild_lvl(const short lvl);
  int  lvl_cost_cid(const short lvl);
  int  lvl_cost_amt(const short lvl);
  int  lvl_cost_money(const short lvl);
  int  lvl_rand_add_exp(const short lvl);
  int  lvl_up_exp(const short lvl);
  int  luck_rate(const short lvl);
  short max_lvl();
private:
  bao_shi_lvl_cfg();
  bao_shi_lvl_cfg_impl *impl_;
};
/**
 * @class bao_shi_eff_cfg
 *
 * @brief
 */
class bao_shi_eff_cfg : public singleton<bao_shi_eff_cfg>
{
  friend class singleton<bao_shi_eff_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  char pos_eff_id(const char pos);
  int eff_init_val(const char pos);
private:
  bao_shi_eff_cfg();
  bao_shi_eff_cfg_impl *impl_;
};
#endif // BAO_SHI_CONFIG_H_
