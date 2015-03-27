// -*- C++ -*-

//========================================================================
/**
 * Author   : alvinhu
 * Date     : 2013-02-04 15:00
 */
//========================================================================

#ifndef SUIT_CONFIG_H_
#define SUIT_CONFIG_H_

#include "singleton.h"
#include "def.h"

#define SUIT_CFG_PATH                            "suit.json"
// Forward declarations
class suit_config_impl;

/**
 * @class suit_attr_info
 *
 * @brief
 */
class suit_attr_info
{
public:
  suit_attr_info();

public:
  int attr_val_[ATTR_T_ITEM_CNT];
};
/**
 * @class suit_config
 * 
 * @brief
 */
class suit_config : public singleton<suit_config>
{
  friend class singleton<suit_config>;
public:
  int load_config(const char *cfg_root);

  int reload_config(const char *cfg_root);

  suit_attr_info *get_suit_attr_info(const int suit_id, const char count);
private:
  suit_config();

  suit_config_impl *impl_;
};

#endif // SUIT_CONFIG_H_

