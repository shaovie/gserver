// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-06 17:30
 */
//========================================================================

#ifndef SCENE_CONFIG_H_
#define SCENE_CONFIG_H_

#include "def.h"
#include "ilist.h"

// Lib header
#include "singleton.h"

// Forward declarations
class scene_config_impl;
class coord_t;

#define SCENE_CFG_PATH                           "scene.json"
#define SCENE_CFG_PATH_FMT                       "s%d.json"
#define SCENE_MPT_CFG_PATH_FMT                   "s%d.mpt"

#define MAX_SEARCH_TABLE_RANGE                   30

/**
 * @class scene_config
 * 
 * @brief
 */
class scene_config : public singleton<scene_config>
{
  friend class singleton<scene_config>;
public:
  // return 0 on success otherwise -1.
  bool is_reload();
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<int> &scene_list();
  short x_len(const int scene_cid);
  short y_len(const int scene_cid);
  bool can_move(const int scene_cid, const short x, const short y);
  bool can_pk(const int scene_cid);
  bool can_drop(const int scene_cid);
  bool can_use_item(const int scene_cid, const int item_cid);
  char coord_value(const int scene_cid, const short x, const short y);
  coord_t get_random_pos(const int scene_cid,
                         const short center_x,
                         const short center_y,
                         const short area_radius);
  coord_t get_scene_random_pos(const int scene_cid,
                               const short center_x,
                               const short center_y);
  short lvl_limit(const int scene_cid);
  scene_coord_t relive_coord(const int scene_cid);
private:
  scene_config();
  scene_config(const scene_config &);
  scene_config& operator= (const scene_config &);

  scene_config_impl *impl_;
};
extern pair_t<short> *g_search_table;
#endif // SCENE_CONFIG_H_

