// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-23 17:47
 */
//========================================================================

#ifndef BASIC_ATTR_CFG_H_
#define BASIC_ATTR_CFG_H_

// Lib header
#include <cstring>
#include "singleton.h"

#include "def.h"

// Forward declarations
class basic_attr_cfg_impl;

#define BASIC_ATTR_CFG_PATH                   "char_base_attr.json"

class basic_attr_cfg_info
{
public:
  basic_attr_cfg_info() { }
  int attr_val_[ATTR_T_ITEM_CNT];
};
/**
 * @class basic_attr_cfg
 * 
 * @brief
 */
class basic_attr_cfg : public singleton<basic_attr_cfg>
{
  friend class singleton<basic_attr_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  basic_attr_cfg_info *basic_attr(const char career);
private:
  basic_attr_cfg();
  basic_attr_cfg(const basic_attr_cfg &);
  basic_attr_cfg& operator= (const basic_attr_cfg &);

  basic_attr_cfg_impl *impl_;
};

#endif // BASIC_ATTR_CFG_H_

