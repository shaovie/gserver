// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-07 15:25
 */
//========================================================================

#ifndef TIANFU_SKILL_UPGRADE_CFG_H_
#define TIANFU_SKILL_UPGRADE_CFG_H_

// Lib header
#include "singleton.h"

// Forward declarations
class tianfu_skill_upgrade_cfg_impl;

#define TIANFU_SKILL_UPGRADE_CFG_PATH           "tianfu_lvlup.json"

class tianfu_skill_upgrade_cfg_obj
{
public:
  int cost_;
  int exp_;
  int item_cid_;
  int item_cnt_;
  int item_exp_;
};
/**
 * @class tianfu_skill_upgrade_cfg
 * 
 * @brief
 */
class tianfu_skill_upgrade_cfg : public singleton<tianfu_skill_upgrade_cfg>
{
  friend class singleton<tianfu_skill_upgrade_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const tianfu_skill_upgrade_cfg_obj *get_upgrade_info(const short lvl);
private:
  tianfu_skill_upgrade_cfg();
  tianfu_skill_upgrade_cfg(const tianfu_skill_upgrade_cfg &);
  tianfu_skill_upgrade_cfg& operator= (const tianfu_skill_upgrade_cfg &);

  tianfu_skill_upgrade_cfg_impl *impl_;
};

#endif // TIANFU_SKILL_UPGRADE_CFG_H_

