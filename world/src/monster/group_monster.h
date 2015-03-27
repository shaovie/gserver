// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-04 10:57
 */
//========================================================================

#ifndef GROUP_MONSTER_H_
#define GROUP_MONSTER_H_

#include "fighting_monster_obj.h"
#include "monster_template.h"

// Forward declarations
class time_value;

/**
 * @class xszc_arm_mst
 * 
 * @brief
 */
class xszc_arm_mst : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  xszc_arm_mst(const char group) :
    group_(group)
  { }

  virtual char group() const { return this->group_; }
  virtual int  attack_target() const { return scene_unit::PLAYER|scene_unit::MONSTER; }
protected:
  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void do_patrol(const time_value &);
  virtual void do_dead(const int);

  virtual void to_back(const time_value &);
private:
  int  do_calc_patrol_path() { return 0; }
private:
  char group_;
  ilist<coord_t> coord_list_;
};
/**
 * @class xszc_defender_mst
 * 
 * @brief
 */
class xszc_defender_mst : public fighting_unmovable_monster
{
  typedef fighting_unmovable_monster super;
public:
  xszc_defender_mst(const char group) :
    group_(group)
  { }

  virtual char group() const { return this->group_; }
protected:
  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void do_dead(const int);
private:
  char group_;
};
/**
 * @class xszc_barrack_mst
 * 
 * @brief
 */
class xszc_barrack_mst : public fighting_unmovable_monster
{
  typedef fighting_unmovable_monster super;
public:
  xszc_barrack_mst(const char group) :
    group_(group)
  { }

  virtual char group() const { return this->group_; }
protected:
  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void do_dead(const int);
private:
  char group_;
};
/**
 * @class xszc_main_mst
 * 
 * @brief
 */
class xszc_main_mst : public fighting_unmovable_monster
{
  typedef fighting_unmovable_monster super;
public:
  xszc_main_mst(const char group) :
    group_(group)
  { }

  virtual char group() const { return this->group_; }
protected:
  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void do_dead(const int);
private:
  char group_;
};

#endif // GROUP_MONSTER_H_
