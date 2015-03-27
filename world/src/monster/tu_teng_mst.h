// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-08-15 11:15
 */
//========================================================================

#ifndef TU_TENG_MST_H_
#define TU_TENG_MST_H_

#include "monster_template.h"

// Forward declarations
class monster_cfg_obj;

/**
 * @class tu_teng_mst
 * 
 * @brief
 */
class tu_teng_mst : public unmovable_monster_obj
{
  typedef unmovable_monster_obj super;
public:
  tu_teng_mst() :
    eye_radius_(0),
    common_cd_(0)
  { }
  virtual ~tu_teng_mst() { }

  virtual char career()  const { return CAREER_ZHONG_LI; }
  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void do_patrol(const time_value &);
  virtual int can_be_attacked(char_obj * /*attacker*/) { return ERR_CAN_NOT_ATTACK_TARGET_2; };
private:
  int eye_radius_;
  int common_cd_;
  time_value last_cure_time_;
};
/**
 * @class player_tu_teng_mst
 * 
 * @brief
 */
class player_tu_teng_mst : public tu_teng_mst 
{
public:
  player_tu_teng_mst() { }
  virtual ~player_tu_teng_mst() { }

  virtual int attack_target() const { return scene_unit::PLAYER; }
};
#endif // TU_TENG_MST_H_

