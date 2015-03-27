// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-14 22:58
 */
//========================================================================

#ifndef MONSTER_MGR_H_
#define MONSTER_MGR_H_

#include "singleton.h"

// Forward declarations
class mblock;
class reactor;
class monster_obj;
class monster_mgr_impl;

/**
 * @class monster_mgr
 *
 * @brief
 */
class monster_mgr : public singleton<monster_mgr>
{
  friend class singleton<monster_mgr>;
public:
  int init(reactor *r);
  int assign_monster_id();
  void release_monster_id(const int id);
  void attach_timer(monster_obj *p);

  //=
  void insert(const int monster_id, monster_obj *);
  monster_obj *find(const int monster_id);
  void remove(const int monster_id);
  int size();

  void birth_to_scene(const int scene_id, const int obj_id);
  void destroy_in_scene(const int scene_id, const int obj_id);
  void destroy_scene_monster(const int scene_id);
private:
  monster_mgr();
  monster_mgr(const monster_mgr &);
  monster_mgr& operator= (const monster_mgr &);

  monster_mgr_impl *impl_;
};

#endif // MONSTER_MGR_H_

