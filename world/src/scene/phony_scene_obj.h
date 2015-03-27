// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-14 17:52
 */
//========================================================================

#ifndef PHONY_SCENE_OBJ_H_
#define PHONY_SCENE_OBJ_H_

#include "scene_obj.h"

// Forward declarations

/**
 * @class phony_scene_obj
 * 
 * @brief
 */
class phony_scene_obj : public scene_obj
{
public:
  phony_scene_obj(const int scene_cid) :
    scene_obj(scene_cid)
  { }
  virtual ~phony_scene_obj() { }

  virtual int init(const short , const short ) { return 0; }
  virtual int enter_scene(scene_unit *su, const short , const short )
  {
    this->unit_list_.push_back(su);
    return 0;
  }
  virtual int exit_scene(scene_unit *su, const short , const short )
  { return this->unit_list_.remove(su); }

  virtual int scan_scene_unit_info(const short /*cur_x*/,
                                   const short /*cur_y*/,
                                   const int char_id,
                                   ilist<mblock *> &slice_mb_list,
                                   ilist<pair_t<int> > *old_scene_unit_list,
                                   ilist<pair_t<int> > *new_scene_unit_list)
  {
    return this->scan_grid_unit_info(this->unit_list_,
                                     char_id,
                                     slice_mb_list,
                                     old_scene_unit_list,
                                     new_scene_unit_list);
  }
private:
  ilist<scene_unit *> unit_list_;
};

#endif // PHONY_SCENE_OBJ_H_

