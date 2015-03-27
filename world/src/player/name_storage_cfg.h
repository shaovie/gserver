// -*- C++ -*-
//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-09 15:20
 */
//========================================================================

#ifndef NAME_STORAGE_CFG_H_
#define NAME_STORAGE_CFG_H_

// Lib header
#include "singleton.h"
#include <string>

#define NAME_STORAGE_CFG_PATH        "name.json"

// Forward declarations
class name_storage_cfg_impl;

/**
 * @class name_storage_cfg
 *
 * @brief
 */
class name_storage_cfg : public singleton<name_storage_cfg>
{
  friend class singleton<name_storage_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  void adjust_data();
  std::string get_name();
  void on_create_char(const char *name);
private:
  std::string get_rand_name();
private:
  name_storage_cfg();
  name_storage_cfg_impl *impl_;
};
#endif // NAME_STORAGE_CFG_H_
