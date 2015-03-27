// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-07 15:46
 */
//========================================================================

#ifndef TIANFU_SKILL_CFG_H_
#define TIANFU_SKILL_CFG_H_

// Lib header
#include "singleton.h"
#include "skill_config.h"

// Forward declarations
class tianfu_skill_cfg_impl;

#define TIANFU_SKILL_CFG_PATH            "tianfu.json"

/**
 * @class tianfu_skill_cfg_obj
 * 
 * @brief 
 */
class tianfu_skill_cfg_obj
{
public:
  tianfu_skill_cfg_obj();
public:
  int anti_cid_;
  int init_val_;
  int inc_val_;
  ilist<sk_rel_buff_info> buff_info_;
};
/**
 * @class tianfu_skill_cfg
 * 
 * @brief
 */
class tianfu_skill_cfg : public singleton<tianfu_skill_cfg>
{
  friend class singleton<tianfu_skill_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const tianfu_skill_cfg_obj *get_skill(const int skill_cid);
  int get_tian_fu_amount();
private:
  tianfu_skill_cfg();
  tianfu_skill_cfg(const tianfu_skill_cfg &);
  tianfu_skill_cfg& operator= (const tianfu_skill_cfg &);

  tianfu_skill_cfg_impl *impl_;
};

#endif // TIANFU_SKILL_CFG_H_

