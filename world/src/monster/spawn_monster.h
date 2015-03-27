// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-07-08 14:18
 */
//========================================================================

#ifndef SPAWN_MONSTER_H_
#define SPAWN_MONSTER_H_

// Lib header
#include "ilist.h"

#include "def.h"

// Forward declarations

/**
 * @class spawn_monster
 * 
 * @brief
 */
class spawn_monster
{
public:
  static int spawn_all_scene_monster();

  static int spawn_scene_monster(const int scene_id,
                                 const int scene_cid,
                                 const int idx,
                                 ilist<pair_t<int> > *mst_list,
                                 ilist<int/*cid*/> *exclude_mst_list);

  static int spawn_one(const int monster_cid,
                       const int delay,
                       const int scene_id,
                       const int scene_cid,
                       const char dir,
                       const short x,
                       const short y);

  static ilist<pair_t<int> > spawn_mst_list; // first id, second cid
};

#endif // SPAWN_MONSTER_H_

