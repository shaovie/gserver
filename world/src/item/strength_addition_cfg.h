// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-16 10:06
 */
//========================================================================

#ifndef STRENGTH_ADDITION_CFG_H_
#define STRENGTH_ADDITION_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"

#define STRENGTH_ADDITION_CFG_PATH          "strength_addition.json"

// Forward declarations
class strength_addition_cfg_impl;

/**
 * @class sa_attr
 *
 * @brief
 */
class sa_attr
{
public:
  sa_attr();
  int attr_val_[ATTR_T_ITEM_CNT];
};

/**
 * @class strength_addition_cfg
 *
 * @brief
 */
class strength_addition_cfg : public singleton<strength_addition_cfg>
{
  friend class singleton<strength_addition_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  sa_attr *get_sa_attr(const int count);
private:
  strength_addition_cfg();

  strength_addition_cfg_impl *impl_;
};

#endif  // STRENGTH_ADDITION_CFG_H_
