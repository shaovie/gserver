// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-22 21:18
 */
//========================================================================

#ifndef MOVABLE_MONSTER_OBJ_H_
#define MOVABLE_MONSTER_OBJ_H_

#include "time_util.h"
#include "monster_obj.h"

// Forward declarations

#define MAX_MONSTER_FIND_PATH_DISTANCE    128

/**
 * @class movable_monster_obj
 * 
 * @brief
 */
class movable_monster_obj : public monster_obj
{
  typedef monster_obj super;
public:
  movable_monster_obj();
  virtual ~movable_monster_obj();

public:
  virtual short move_speed() const { return this->move_speed_; }
  virtual void  move_speed(const short );
  virtual short prev_move_speed() const { return this->prev_move_speed_; }

  virtual int can_move(const short x, const short y);
  virtual int can_be_moved();
  virtual int can_patrol(const time_value &);
protected:
  void to_patrol();
protected:
  virtual int  do_load_config(monster_cfg_obj *);
  virtual int  do_rebirth_init(const time_value &);
  virtual void do_patrol(const time_value &);
  int  do_find_path(const coord_t &from, const coord_t &to, const bool);
  void do_simple_find_path(const short from_x, const short from_y, const short to_x, const short to_y);
  int  do_flash_move(const short x, const short y, const char dir);
  void do_move_i(const time_value &);
  void do_move_i(const time_value &, const short to_x, const short to_y);
private:
  int  do_calc_patrol_path();
  void do_calc_next_patrol_time();
protected:
  short move_speed_;
  short prev_move_speed_;
  short patrol_speed_;
  short patrol_radius_;
  int patrol_interval_;
  time_value reach_pos_time_;
  time_value next_patrol_time_;
  ilist<coord_t> path_;
};

#include "movable_monster_obj.inl"
#endif // MOVABLE_MONSTER_OBJ_H_

