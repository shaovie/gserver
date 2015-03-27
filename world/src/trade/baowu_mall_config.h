// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-25 10:50
 */
//========================================================================

#ifndef BAOWU_MALL_CONFIG_H_
#define BAOWU_MALL_CONFIG_H_

#include "singleton.h"

#define BAOWU_MALL_CONFIG_PATH                     "baowu_mall.json"

// Forward declarations
class baowu_mall_config_impl;

/**
 * @class baowu_mall_config
 *
 * @brief
 */
class baowu_mall_config: public singleton<baowu_mall_config>
{
  friend class singleton<baowu_mall_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int get_item_index(const int lvl);
private:
  baowu_mall_config();
  baowu_mall_config_impl *impl_;
};
#endif // BAOWU_MALL_CONFIG_H_

