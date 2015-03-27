// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-25 14:42
 */
//========================================================================

#ifndef LUE_DUO_CONFIG_H_
#define LUE_DUO_CONFIG_H_

// Lib header
#include "singleton.h"

#define LUE_DUO_CONFIG_PATH                "rob_win_award.json"

// Forward declarations
class lue_duo_cfg_impl;

/**
 * @class ld_obj
 *
 * @brief
 */
class ld_obj
{
public:
  ld_obj() :
    item_cid_(0),
    item_cnt_(0),
    rate_(0)
  { }

  int item_cid_;
  int item_cnt_;
  int rate_;
};

/**
 * @class lue_duo_cfg
 *
 * @brief
 */
class lue_duo_config : public singleton<lue_duo_config>
{
  friend class singleton<lue_duo_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ld_obj *get_random_ld_obj();
private:
  lue_duo_config();

  lue_duo_cfg_impl *impl_;
};
#endif  //LUE_DUO_CONFIG_H_
