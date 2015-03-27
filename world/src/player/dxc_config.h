// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-26 18:57
 */
//========================================================================

#ifndef DXC_CONFIG_H_
#define DXC_CONFIG_H_

// Lib header
#include "singleton.h"
#include "def.h"

#define DXC_CONFIG_PATH                 "di_xia_cheng.json"

enum
{
  DXC_NORMAL = 1,
  DXC_HARD   = 2,
  DXC_MASTER = 3,
  DXC_END
};

// Forward declarations
class dxc_config_impl;

class dxc_cfg_obj
{
public:
  dxc_cfg_obj();
  int zhanli_limit_[DXC_END];
};
/**
 * @class dxc_config
 * 
 * @brief
 */
class dxc_config : public singleton<dxc_config>
{
  friend class singleton<dxc_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const dxc_cfg_obj *get_dxc_cfg_obj(const int scene_cid);
private:
  dxc_config();
  dxc_config(const dxc_config &);
  dxc_config& operator= (const dxc_config &);

  dxc_config_impl *impl_;
};

#endif // DXC_CONFIG_H_
