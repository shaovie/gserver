// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-21 15:27
 */
//========================================================================

#ifndef CHENG_JIU_CFG_H_
#define CHENG_JIU_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define CHENG_JIU_CFG_PATH                "achievement.json"

class cheng_jiu_cfg_impl;
class item_amount_bind_t;

/**
 * @class cheng_jiu_obj
 *
 * @brief
 */
class cheng_jiu_obj
{
public:
  cheng_jiu_obj();

  int show_lvl_;
  int value_;
  int next_cj_id_;
  int title_;
  int lucky_rate_;
  ilist<item_amount_bind_t> award_list_;
  ilist<item_amount_bind_t> lucky_award_;
};

/**
 * @class cheng_jiu_cfg
 *
 * @brief
 */
class cheng_jiu_cfg : public singleton<cheng_jiu_cfg>
{
  friend class singleton<cheng_jiu_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  cheng_jiu_obj *get_cheng_jiu_obj(const int cj_id);
public:
  cheng_jiu_cfg();

  cheng_jiu_cfg_impl *impl_;
};

#endif  // CHENG_JIU_CFG_H_
