// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-19 09:52
 */
//========================================================================

#ifndef EXIT_TRANSFER_CFG_H_
#define EXIT_TRANSFER_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"

// Forward declarations
class exit_transfer_cfg_impl;

#define SCENE_TRANSFER_CFG_PATH               "scene_transfer.json"

/**
 * @class exit_transfer_cfg
 * 
 * @brief
 */
class exit_transfer_cfg : public singleton<exit_transfer_cfg>
{
  friend class singleton<exit_transfer_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  scene_coord_t get_target_coord(const int id, const int scene_cid);
private:
  exit_transfer_cfg();
  exit_transfer_cfg(const exit_transfer_cfg &);
  exit_transfer_cfg& operator= (const exit_transfer_cfg &);

  exit_transfer_cfg_impl *impl_;
};

#endif // EXIT_TRANSFER_CFG_H_

