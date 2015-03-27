// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-02 15:19
 */
//========================================================================

#ifndef MONSTER_POS_CFG_H_
#define MONSTER_POS_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define MONSTAR_POS_CFG_PATH          "monster_pos_cfg.json"

// Forward declarations
class monster_pos_cfg_impl;

/**
 * @class mst_spawn_list
 *
 * @brief
 */
class mst_spawn_list
{
public:
  mst_spawn_list() {}
  ~mst_spawn_list()
  {
    while (!this->spawn_list_.empty())
      delete this->spawn_list_.pop_front();
  }
  class _info
  {
  public:
    _info() :
      monster_cid_(0),
      monster_cnt_(0),
      param_(0)
    { }

    int monster_cid_;
    int monster_cnt_;
    int param_;
    ilist<coord_t> tar_coord_;
  };

  ilist<_info *> spawn_list_;
};

/**
 * @class monster_pos_cfg
 *
 * @brief
 */
class monster_pos_cfg : public singleton<monster_pos_cfg>
{
  friend class singleton<monster_pos_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  mst_spawn_list *get_mst_spawn_list(const int scene_cid);
  mst_spawn_list::_info *get_mst_spawn_info(const int scene_cid, const int monster_cid);
private:
  monster_pos_cfg();

  monster_pos_cfg_impl *impl_;
};

#endif  // MONSTER_POS_CFG_H_
