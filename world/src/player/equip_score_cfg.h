// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-07 13:39
 */
//========================================================================

#ifndef EQUIP_SCORE_CFG_H_
#define EQUIP_SCORE_CFG_H_

// Lib header
#include <cstring>
#include "singleton.h"

#include "def.h"

// Forward declarations
class equip_score_cfg_impl;

#define EQUIP_SCORE_CFG_PATH                   "equip_score.json"

class equip_score_coe
{
public:
  equip_score_coe() { ::memset(this->attr_coe_, 0, sizeof(this->attr_coe_)); }
  double attr_coe_[ATTR_T_ITEM_CNT];
};
/**
 * @class equip_score_cfg
 * 
 * @brief
 */
class equip_score_cfg : public singleton<equip_score_cfg>
{
  friend class singleton<equip_score_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  equip_score_coe *get_equip_score_coe(const char career);
private:
  equip_score_cfg();
  equip_score_cfg(const equip_score_cfg &);
  equip_score_cfg& operator= (const equip_score_cfg &);

  equip_score_cfg_impl *impl_;
};

#endif // EQUIP_SCORE_CFG_H_

