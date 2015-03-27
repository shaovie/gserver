// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-08-06 11:00
 */
//========================================================================

#ifndef WATER_TREE_CFG_H_
#define WATER_TREE_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define WATER_TREE_CFG_PATH                "guan_gai.json"

class water_tree_cfg_impl;
class item_amount_bind_t;

class water_tree_random_award
{
public:
  int rate_;
  item_amount_bind_t award_;
};
/**
 * @class water_tree_obj
 *
 * @brief
 */
class water_tree_obj
{
public:
  water_tree_obj();
  ~water_tree_obj();

  int exp_;
  int attr_param_;
  int cai_liao_cid_;
  int cai_liao_cnt_;
  int bind_diamond_;
  item_amount_bind_t role_reward_;
  ilist<water_tree_random_award *> random_award_;
};

/**
 * @class water_tree_cfg
 *
 * @brief
 */
class water_tree_cfg : public singleton<water_tree_cfg>
{
  friend class singleton<water_tree_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  water_tree_obj *get_water_tree_obj(const int lvl);
public:
  water_tree_cfg();

  water_tree_cfg_impl *impl_;
};

#endif  // WATER_TREE_CFG_H_
