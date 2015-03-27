// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-05 15:26
 */
//========================================================================

#ifndef PASSIVE_SKILL_CFG_H_
#define PASSIVE_SKILL_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class passive_skill_cfg_impl;

#define PASSIVE_SKILL_CFG_PATH                    "passive_skill.json"

/**
 * @class passive_skill_cfg_obj
 * 
 * @brief 
 */
class passive_skill_cfg_obj
{
public:
  passive_skill_cfg_obj();
public:
  char career_;
  int effect_id_;
  int learn_lvl_;
  int attr_val_;
};
/**
 * @class passive_skill_cfg
 * 
 * @brief
 */
class passive_skill_cfg : public singleton<passive_skill_cfg>
{
  friend class singleton<passive_skill_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const passive_skill_cfg_obj *get_skill(const int skill_cid);
  ilist<int> *get_skill_list(const int career);
private:
  passive_skill_cfg();
  passive_skill_cfg(const passive_skill_cfg &);
  passive_skill_cfg& operator= (const passive_skill_cfg &);

  passive_skill_cfg_impl *impl_;
};

#endif // PASSIVE_SKILL_CFG_H_

