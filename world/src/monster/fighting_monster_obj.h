// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-24 18:53
 */
//========================================================================

#ifndef FIGHTING_MONSTER_OBJ_H_
#define FIGHTING_MONSTER_OBJ_H_

#include "skill_info.h"
#include "buff_module.h"
#include "movable_monster_obj.h"
#include "global_param_cfg.h"

// Forward declarations
class ai_info;
class hate_val_map;
class monster_cfg_obj;

/* ---V--------------------------V-----------------------V-----
 *   受伤---------------------受伤完毕----------------攻击完毕---
 *         受伤动作播放时间              攻击动作播放时间
 */
/**
 * @class fighting_monster_obj
 *
 * @brief
 */
class fighting_monster_obj : public movable_monster_obj
{
  typedef movable_monster_obj super;
public:
  enum
  {
    PASSIVE_ATT        = 2,         // 被动怪
    ACTIVE_ATT         = 1          // 主动怪
  };
  fighting_monster_obj();
  virtual ~fighting_monster_obj();

  //=
  virtual char career()  const { return this->career_; }
  virtual short lvl()  const { return this->lvl_; }
  virtual int  attack_target() const { return scene_unit::PLAYER; }
  virtual void do_ji_tui(char_obj *, const time_value &, const int, const int);
protected:
  virtual int do_load_config(monster_cfg_obj *);
  virtual int do_rebirth(const time_value &);
  virtual int do_rebirth_init(const time_value &);

  virtual void do_patrol(const time_value &);
  virtual void do_attack(const time_value &now);
  virtual void do_chase(const time_value &now);
  virtual void do_back(const time_value &now);
  virtual void do_dead(const int);
  virtual void do_fight_back(const int hurter_id);
  int  do_search_in_view_area(const time_value &now);
  int  do_search_proper_target_in_view_area();
  void do_off_war(const time_value &);
  virtual skill_info *do_select_skill(const time_value &now,
                                      char_obj *target,
                                      int &result,
                                      int &skill_hurt_delay,
                                      const skill_detail *&sd);
  void do_adjust_dir(const short target_x, const short target_y);
  int  do_select_another_target(const int old_target);
  int  do_find_max_hate_and_proper_target();
  void do_clear_hate();
protected:
  void to_attack();
  virtual void to_back(const time_value &);
  void to_chase();
  void to_be_repulsed(const time_value &now);
protected:
  // 天生不可移动的怪和处于不可移动状态的怪，追击失败后的反应是不一样的
  virtual void on_chase_failed();
  virtual void on_back();
  virtual void on_be_hurt(char_obj *, const time_value &, const int, const int);
  virtual void on_use_skill_ok(skill_info *si,
                               const skill_detail *,
                               const int ,
                               const time_value &);
protected:
  int  can_use_skill_i(const time_value &now);
protected:
  virtual void dispatch_aev(const int aev_id, mblock *);
  void aev_other_dead(mblock *mb);
protected:
  //= ai
  void destroy_ai_list();
  void do_load_ai(monster_cfg_obj *);
  void do_ai(ilist<ai_info *> *, const time_value &, const bool);
  int  dispatch_ai_if(ai_info *, const time_value &now);
  int  dispatch_ai_do(ai_info *, const time_value &now);
  int  ai_do_use_skill(ai_info *, const time_value &now);
  int  ai_do_call_mst(ai_info *, const time_value &now);
  int  ai_do_learn_skill(ai_info *, const time_value &now);
  int  ai_do_recover_hp(ai_info *, const time_value &now);
protected:
  bool change_target_;
  char sort_;
  char att_type_;
  char career_;
  char eye_angle_;
  short eye_radius_;
  short chase_radius_;
  short chase_speed_;
  short lvl_;
  int att_obj_id_;
  int base_skill_cid_;
  int refresh_time_;
  int stiff_mian_yi_time_; // msec 僵直免疫时间
  int first_be_attacked_time_; // 第一次受伤时间
  long common_cd_;
  long fight_back_delay_;
  hate_val_map *hate_val_map_;
  ilist<ai_info *> *birth_ai_list_;
  ilist<ai_info *> *attack_ai_list_;
  ilist<ai_info *> *back_ai_list_;
  ilist<ai_info *> *die_ai_list_;
  ilist<skill_info *> skill_list_;
  time_value next_search_time_;
  time_value next_check_ai_time_;
  time_value stiff_mian_yi_end_time_; // 僵直免疫结束时间
};
#include "fighting_monster_obj.inl"
#endif // FIGHTING_MONSTER_OBJ_H_

