// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-12-29 17:45
 */
//========================================================================

#ifndef EFFECT_OBJ_H_
#define EFFECT_OBJ_H_

#include "obj_pool.h"
#include "scene_unit.h"

// Lib header
#include "singleton.h"
#include "time_value.h"

// Forward declarations

/**
 * @class effect_obj
 * 
 * @brief
 */
class effect_obj : public scene_unit
{
public:
  effect_obj();
  virtual ~effect_obj() { }

  void set(const int sk_cid,
           const int master_id,
           const int scene_cid,
           const int scene_id,
           const short x,
           const short y);

  virtual int unit_type()   const { return scene_unit::EFFECT; }
  virtual bool can_block()  const { return false; }

  int do_enter_scene();
  int do_exit_scene();

  virtual int do_ai(const time_value &) = 0;
  virtual void release() = 0;

  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_list,
                                  ilist<pair_t<int> > *new_snap_list);
protected:
  int master_id_;
};
/**
 * @class sk_last_hurt_effect_obj 
 * 
 * @brief
 */
class sk_last_hurt_effect_obj : public effect_obj
{
public:
  void set(const time_value &now,
           const int master_id,
           const int sk_cid,
           const int skill_lvl,
           const int scene_cid,
           const int scene_id,
           const short x,
           const short y,
           const int last_time);
  virtual int do_ai(const time_value &);
  virtual void release();
private:
  int skill_lvl_;
  time_value last_action_time_;
  time_value end_time_;
};
class sk_last_hurt_effect_obj_alloctor : public singleton<sk_last_hurt_effect_obj_alloctor>
{
  friend class singleton<sk_last_hurt_effect_obj_alloctor>;
public:
  sk_last_hurt_effect_obj *alloc();
  void release(sk_last_hurt_effect_obj *);
};
#endif // EFFECT_OBJ_H_

