// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-05 17:25
 */
//========================================================================

#ifndef PASSIVE_SKILL_UPGRADE_CFG_H_
#define PASSIVE_SKILL_UPGRADE_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"

// Forward declarations
class passive_skill_upgrade_cfg_impl;

#define PASSIVE_SKILL_UPGRADE_CFG_PATH           "passive_lvlup.json"

class passive_skill_upgrade_cfg_obj
{
public:
  int cost_;
  int item_cid_;
  int item_cnt_;
  int probability_;
  int min_upgrade_cnt_;
  int min_bless_val_;
  int max_bless_val_;
  int total_bless_val_;
  int lucky_rate_;
};
/**
 * @class passive_skill_upgrade_cfg
 * 
 * @brief
 */
class passive_skill_upgrade_cfg : public singleton<passive_skill_upgrade_cfg>
{
  friend class singleton<passive_skill_upgrade_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const passive_skill_upgrade_cfg_obj *get_upgrade_info(const short lvl);
private:
  passive_skill_upgrade_cfg();
  passive_skill_upgrade_cfg(const passive_skill_upgrade_cfg &);
  passive_skill_upgrade_cfg& operator= (const passive_skill_upgrade_cfg &);

  passive_skill_upgrade_cfg_impl *impl_;
};

#endif // PASSIVE_SKILL_UPGRADE_CFG_H_

