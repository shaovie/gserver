// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-25 14:09
 */
//========================================================================

#ifndef ZHAN_LI_CFG_H_
#define ZHAN_LI_CFG_H_

// Lib header
#include <cstring>
#include "singleton.h"

#include "def.h"

// Forward declarations
class zhan_li_calc_cfg_impl;

#define ZHAN_LI_CALC_CFG_PATH                   "fight_score.json"

class zhan_li_calc_coe
{
public:
  zhan_li_calc_coe() { ::memset(this->attr_coe_, 0, sizeof(this->attr_coe_)); }
  double attr_coe_[ATTR_T_ITEM_CNT];
};
/**
 * @class zhan_li_calc_cfg
 * 
 * @brief
 */
class zhan_li_calc_cfg : public singleton<zhan_li_calc_cfg>
{
  friend class singleton<zhan_li_calc_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  zhan_li_calc_coe *get_zhan_li_calc_coe(const char career);
private:
  zhan_li_calc_cfg();
  zhan_li_calc_cfg(const zhan_li_calc_cfg &);
  zhan_li_calc_cfg& operator= (const zhan_li_calc_cfg &);

  zhan_li_calc_cfg_impl *impl_;
};

#endif // ZHAN_LI_CFG_H_

