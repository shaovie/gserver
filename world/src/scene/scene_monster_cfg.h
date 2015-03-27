// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-27 14:45
 */
//========================================================================

#ifndef SCENE_MONSTER_CFG_H_
#define SCENE_MONSTER_CFG_H_

// Lib header
#include "singleton.h"

#define SCENE_MONSTER_CFG_PATH                 "scene_monster.json"

// Forward declarations
class scene_monster_cfg_impl;

/**
 * @class scene_monster_cfg
 * 
 * @brief
 */
class scene_monster_cfg : public singleton<scene_monster_cfg>
{
  friend class singleton<scene_monster_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  bool have_monster(const int scene_cid);
  int get_monster_info(const int scene_cid,
                       const int idx,
                       int &mst_cid,
                       int &dir);
private:
  scene_monster_cfg();

  scene_monster_cfg_impl *impl_;
};

#endif // SCENE_MONSTER_CFG_H_

