// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-11 17:57
 */
//========================================================================

#ifndef CHAR_OBJ_H_
#define CHAR_OBJ_H_

#include "util.h"
#include "error.h"
#include "client.h"
#include "message.h"
#include "obj_attr.h"
#include "scene_unit.h"
#include "skill_config.h"
#include "aev_dispatch.h"
#include "sev_dispatch.h"
#include "buff_effect_id.h"

// Lib header
#include "macros.h"
#include "time_value.h"

// Forward declarations
class buff_obj;
class skill_info;
class dropped_item;
class buff_time_map;
class tianfu_skill_info;
class fa_bao_attr;

enum SK_TIPS
{
  SK_TIP_SHAN_BI         = 1,
  SK_TIP_BAO_JI          = 2,
  SK_MIAN_YI             = 3,
  SK_TIP_FAN_TAN         = 4,
  SK_TIP_HU_DUN          = 5,
  TIP_END
};
enum
{
  MODIFY_HP              = 1,
  MODIFY_MP              = 2,
};

/**
 * @class char_obj
 * 
 * @brief
 */
class char_obj : public scene_unit
                   , public aev_dispatch
                   , public sev_dispatch
{
  friend class buff_module;
  friend class sk_last_hurt_effect_obj;
  friend class tianfu_skill_module;
  friend class fa_bao_module;
public:
  char_obj();

  virtual ~char_obj();

  //= inline property method
  virtual char career()  const = 0; 
  virtual short lvl()    const { return 0; }
  virtual int team_id()  const { return 0; }
  virtual int guild_id() const { return 0; }
  virtual int master_id()const { return 0; } 
  virtual void master_id(const int ){ } 
  virtual short move_speed() const { return 0; }
  virtual void  move_speed(const short ) { return ; }
  virtual short prev_move_speed() const { return 0; }
  virtual int attack_target() const { return 0; }
  virtual char  pk_mode() const { return PK_MODE_FIGHT; }

  virtual char group() const { return 0; }

  int  hp()          const { return this->obj_attr_.hp_;        }
  int  total_hp()    const { return this->obj_attr_.total_hp(); }
  int  mp()          const { return this->obj_attr_.mp_;        }
  int  total_mp()    const { return this->obj_attr_.total_mp(); }
  obj_attr &get_obj_attr() { return this->obj_attr_;            }
  ilist<tianfu_skill_info *> &get_tianfu_list() { return this->tianfu_skill_list_; }

  virtual int do_enter_scene();
  virtual int do_exit_scene();

  int  do_calc_hurt(int &attacked_tip, char_obj *defender);
  int  do_add_hp(const int value);
  int  do_add_mp(const int value);
  int  do_reduce_hp(const int value);
  int  do_reduce_mp(const int value);
  void do_check_hp(char_obj *);
  int  do_insert_buff(const int buff_id,
                      const buff_effect_id_t effect_id,
                      const int src_id,
                      const int buff_rel_cid,
                      const int left_value,
                      const int left_time,
                      const int interval,
                      const int value,
                      const time_value &now,
                      const int bits,
                      const int any_param);
  // 执行真正受到伤害的逻辑
  virtual int do_be_hurt(char_obj *attacker,
                         const time_value &,
                         const int real_hurt,
                         const int hate_coe,
                         const int skill_cid,
                         const int skill_hurt_delay);
  virtual void do_ji_tui(char_obj *,
                         const time_value &,
                         const int /*back_dis*/,
                         const int /*skill_cid*/) { };
protected:
  // 计算基本属性得出的伤害数值
  int  do_calc_hurt_by_attr(int &attacked_tip, char_obj *defender);

  // 计算BUF等效果影响的伤害数值
  virtual int do_calc_hurt_by_other_effect_for_attacker(const int hurt);
  virtual int do_calc_hurt_by_other_effect_for_defender(int &attacked_tip, const int hurt);
  void do_tianfu_effect(char_obj *, const time_value &);
  int tianfu_work_rate(const int );
  int  do_move(const short x, const short y, const char dir);
  int  do_use_skill(const int target_id,
                    const time_value &now,
                    const short target_x,
                    const short target_y,
                    const int skill_cid,
                    const int skill_hurt_delay,
                    const skill_detail *sd);
  int  do_modify_energy(const int value, const int type);

  // snap info
  void do_build_snap_hp_mp_info(out_stream &os);

  // broadcast info
  void do_build_broadcast_position_msg(mblock &);
  void do_build_broadcast_speed_msg(mblock &);
  void do_build_broadcast_hp_mp_msg(mblock &);
  void do_build_broadcast_use_skill(mblock &mb,
                                    const int skill_cid,
                                    const short skill_lvl,
                                    const int target_id,
                                    const short x,
                                    const short y);
  void do_build_broadcast_be_hurt_effect_msg(mblock &mb,
                                             const int skill_cid,
                                             const int hurt,
                                             const int tip,
                                             const int attacker_id);
  void do_build_broadcast_unit_life_status(mblock &mb, const char st);
  void do_build_broadcast_add_buff_msg(mblock &mb, const int );
  void do_build_broadcast_del_buff_msg(mblock &mb, const int );
  void do_build_broadcast_flash_move(mblock &mb,
                                     const short target_x,
                                     const short target_y);
  void do_build_broadcast_ji_tui(mblock &mb,
                                 const int skill_cid,
                                 const short target_x,
                                 const short target_y);
  void do_build_broadcast_tianfu_skill(mblock &mb,
                                       const int skill_cid);

  void do_drop_items_on_scene(ilist<dropped_item *> &/*drop_list*/,
                              const int );
  void do_update_all_buff(const time_value &now);
public:
  // broadcast method
  virtual void broadcast_position();
  virtual void broadcast_speed();
  virtual void broadcast_hp_mp();
  virtual void broadcast_use_skill(const int skill_cid,
                                   const short skill_lvl,
                                   const int target_id,
                                   const short x,
                                   const short y);
  virtual void broadcast_be_hurt_effect(const int skill_cid,
                                        const int hurt,
                                        const int tip,
                                        const int attacker_id);
  virtual void broadcast_unit_life_status(const char st);
  virtual void broadcast_add_buff(buff_obj *bo);
  virtual void broadcast_del_buff(buff_obj *bo);
  virtual void broadcast_flash_move(const short target_x,
                                    const short target_y);
  virtual void broadcast_ji_tui(const int skill_cid,
                                const short target_x,
                                const short target_y);
  virtual void broadcast_tianfu_skill(const int skill_cid);
public:
  int is_proper_attack_target(char_obj *target);
  int is_proper_cure_target(char_obj *target);
  bool has_buff(const buff_effect_id_t buff_effect_id);
  buff_obj *get_buff_by_effect(const buff_effect_id_t effect_id);
  virtual bool can_block() const { return true; }
  virtual bool can_ke_zhi(char_obj * /*defender*/)      { return false; }
  virtual bool can_be_ke_zhi(char_obj * /*defender*/)   { return false; }
  virtual int  can_attack(char_obj * /*target*/);
  virtual int  can_be_attacked(char_obj * /*attacker*/);
  virtual int  can_be_hurt(char_obj * /*target*/);
  virtual int  can_cure(char_obj * /*target*/)          { return 0; }
  virtual int  can_be_cured(char_obj * /*attacker*/)    { return 0; }
  virtual int  can_use_skill(const skill_detail *sd,
                             const skill_info *si,
                             const int block_radius,
                             const time_value &now,
                             const coord_t &target_pos);
  virtual int  can_be_moved();
  virtual int  can_move(const short , const short);
  virtual int  can_insert_buff(const int bf_id,
                               const buff_effect_id_t effect_id);
public:
  virtual void on_attack_somebody(const int /*target_id*/,
                                  const int /*obj_type*/,
                                  const time_value &/*now*/,
                                  const int /*skill_cid*/,
                                  const int /*real_hurt*/) { }
  // 被攻击后做何反应
  virtual void on_be_attacked(char_obj *attacker);

  // 被攻击并且产生伤害值后做何反应
  virtual void on_be_hurt(char_obj *attacker, const time_value &, const int, const int );
  virtual void on_add_buff(buff_obj *bo);
  virtual void on_remove_buff(buff_obj *bo);
  virtual void on_modify_buff(buff_obj *) { }

  virtual void on_attr_update(const int , const int) { }
private:
  int dispatch_skill(const int target_id,
                     const time_value &now,
                     const short target_x,
                     const short target_y,
                     const int skill_cid,
                     const int skill_hurt_delay,
                     const skill_detail *sd);

  //= skill effect
  void sk_do_insert_buff(char_obj *target,
                         const int skill_cid,
                         const int real_hurt,
                         const time_value &now,
                         const ilist<sk_rel_buff_info> &buff_info,
                         const sk_buff_target_t,
                         const int bits);
  int sk_percent_hurt_and_add_fixed_hurt(const int target_id,
                                         const time_value &now,
                                         const int skill_cid,
                                         const int skill_hurt_delay,
                                         const skill_detail *sd,
                                         const short target_x,
                                         const short target_y);
  int sk_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                              const int skill_cid,
                                              const int skill_hurt_delay,
                                              const skill_detail *sd,
                                              const int target_id,
                                              const short target_x,
                                              const short target_y,
                                              const bool with_tian_fu);
  int sk_line_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                   const int skill_cid,
                                                   const int skill_hurt_delay,
                                                   const skill_detail *sd);
  int sk_1_4_view_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                       const int skill_cid,
                                                       const int skill_hurt_delay,
                                                       const skill_detail *sd);
  int sk_insert_buff_to_self(const time_value &, const int , const skill_detail *);
  int sk_insert_buff_to_target(const time_value &, const int , const skill_detail *sd, const int );
  int sk_jing_xiang_fen_shen(const int , const int , const skill_detail *sd);
  int sk_area_percent_hurt_and_add_fixed_hurt_and_insert_buff_to_self(const time_value &now,
                                                                      const int skill_cid,
                                                                      const int skill_hurt_delay,
                                                                      const skill_detail *sd,
                                                                      const short target_x,
                                                                      const short target_y);
  void sk_do_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                  const int target_cnt,
                                                  const int *target_set,
                                                  const int skill_cid,
                                                  const int skill_hurt_delay,
                                                  const skill_detail *sd,
                                                  const bool with_tian_fu,
                                                  int &attacked_target_id,
                                                  int &sum_hurt);
  int sk_place_a_last_hurt_effect_and_insert_buf_to_self(const time_value &, const int , const skill_detail *);
  int sk_area_total_hp_percent_hurt(const time_value &now,
                                    const int skill_cid,
                                    const int skill_hurt_delay,
                                    const skill_detail *sd,
                                    const short target_x,
                                    const short target_y);

protected:
  fa_bao_attr *fa_bao_attr_;
  buff_time_map *buff_time_map_;

  time_value last_use_skill_time_;
  time_value next_update_buff_time_;
  time_value last_tianfu_worked_time_;

  obj_attr obj_attr_;

  ilist<tianfu_skill_info *> tianfu_skill_list_;
  ilist<buff_obj *> buff_obj_list_;
  char buff_effect_cnt_[BF_CNT];
};
#include "char_obj.inl"
#endif // CHAR_OBJ_H_

