// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-12 23:07
 */
//========================================================================

#ifndef MONSTER_OBJ_H_
#define MONSTER_OBJ_H_

#include "util.h"
#include "error.h"
#include "char_obj.h"

// Lib header
#include "time_value.h"

#define MST_ZOMBIE_TIME             1200   // !!! MUST > snap interval

// Forward declarations
class time_value;
class monster_cfg_obj;

/**
 * @class monster_obj
 * 
 * @brief
 */
class monster_obj : public char_obj
{
  friend class mst_loop_timer;
  typedef char_obj super;
public:
  enum
  {
    ST_NULL               = 10,        // 
    ST_ENTER_SCENE        = 11,        // 在场景内活跃状态
    ST_EXIT_SCENE         = 12,        // 退出场景
    ST_DIE_IN_SCENE       = 13,        // 死亡状态
    ST_TO_DESTROY         = 14,        // 通知销毁
    ST_DELAY_DIE          = 15,        // 延迟死亡
    ST_DELAY_ENTER_SCENE  = 16,        // 延迟出现
  };
  enum
  {
    DO_PATROLING          = 1L << 0,   // 巡逻状态
    DO_ATTACKING          = 1L << 1,   // 战斗状态
    DO_CHASING            = 1L << 2,   // 追击状态
    DO_BACKING            = 1L << 3,   // 返回出生点
  };
  monster_obj();
  virtual ~monster_obj();

  int init(const int mst_cid,
           const int scene_id,
           const int scene_cid,
           const char dir,
           const short x,
           const short y);
public:
  //= inline property method
  virtual int unit_type()   const { return scene_unit::MONSTER; }

  virtual int can_be_attacked(char_obj * /*attacker*/);
public:
  void to_destroy();
public:
  int  do_timeout(const time_value &now);
  virtual int do_activate(const int delay);
  virtual void on_activated() { return ; }
  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_unit_list,
                                  ilist<pair_t<int> > *new_snap_unit_list);
protected:
  virtual void dispatch_sev(const int /*sev_id*/, mblock *) { return ; }
  virtual void dispatch_aev(const int aev_id, mblock *);
  void aev_be_killed(mblock *mb);
protected:
  int can_switch_to_status(const int st);

protected:
  virtual int  do_assign_id();
  virtual void do_release_id();
  virtual int  do_enter_scene();
  virtual int  do_exit_scene();

  virtual int  do_load_config(monster_cfg_obj *);
  virtual int  do_rebirth(const time_value &) { return 0; }
  virtual int  do_rebirth_init(const time_value &);
  virtual int  do_action_in_scene(const time_value &);
  virtual int  do_think(const time_value &) { return 0; };
  virtual void do_patrol(const time_value &) { return ; }
  virtual void do_attack(const time_value &) { return ; }
  virtual void do_chase(const time_value &) { return ; }
  virtual void do_back(const time_value &) { return ; }
  virtual void do_dead(const int) { return ; }

  void do_assign_award(const int killer_id);
  void do_assign_award_to_char(const int );
protected:
  void do_build_snap_base_info(out_stream &os);
  void do_build_snap_pos_info(out_stream &os);
private:
  void do_close();

  void broadcast_enter_scene();
protected:
  char  birth_dir_;
  short birth_coord_x_;
  short birth_coord_y_;

  int status_;
  int do_status_;
  int live_time_;
  int birth_time_;
  int killer_id_;

  time_value dead_time_;
  time_value be_hurt_done_time_;
  time_value attack_done_time_;
  time_value enter_scene_time_;
};
#include "monster_obj.inl"
#endif // MONSTER_OBJ_H_

