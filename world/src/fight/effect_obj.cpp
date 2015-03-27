#include "effect_obj.h"
#include "effect_obj_mgr.h"
#include "scene_mgr.h"
#include "global_macros.h"
#include "fighter_mgr.h"
#include "char_obj.h"
#include "sys_log.h"

// Lib header
#include <stdlib.h>

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("effect_obj");
static ilog_obj *e_log = err_log::instance()->get_ilog("effect_obj");

effect_obj::effect_obj() :
  master_id_(0)
{ }
void effect_obj::set(const int cid,
                     const int master_id,
                     const int scene_cid,
                     const int scene_id,
                     const short x,
                     const short y)
{
  this->id_          = effect_obj_mgr::instance()->assign_effect_obj_id();
  this->cid_         = cid;
  this->scene_cid_   = scene_cid;
  this->scene_id_    = scene_id;
  this->coord_x_     = x;
  this->coord_y_     = y;

  this->master_id_   = master_id;
}
int effect_obj::do_enter_scene()
{
  if (scene_unit::do_enter_scene() != 0) return -1;
  return 0;
}
int effect_obj::do_exit_scene()
{
  scene_unit::do_exit_scene();
  return 0;
}
bool effect_obj::do_build_snap_info(const int ,
                                    out_stream &os,
                                    ilist<pair_t<int> > *old_snap_unit_list,
                                    ilist<pair_t<int> > *new_snap_unit_list)
{
  new_snap_unit_list->push_back(pair_t<int>(this->id_, this->cid_));

  if (this->is_new_one(old_snap_unit_list))
  {
    os << T_CID_ID << this->cid_ << this->id_;
    os << T_X_Y_INFO << this->coord_x_ << this->coord_y_ << this->master_id_ << (int)0;
    os << T_END;
    return true;
  }
  return false;
}
// ---------------------------------------------------------------------------------
void sk_last_hurt_effect_obj::set(const time_value &now,
                                  const int master_id,
                                  const int sk_cid,
                                  const int skill_lvl,
                                  const int scene_cid,
                                  const int scene_id,
                                  const short x,
                                  const short y,
                                  const int last_time)
{
  this->skill_lvl_    = skill_lvl;
  this->last_action_time_ = now;
  this->end_time_ = now + time_value(0, last_time * 1000);
  effect_obj::set(sk_cid,
                  master_id,
                  scene_cid,
                  scene_id,
                  x,
                  y);
}
int sk_last_hurt_effect_obj::do_ai(const time_value &now)
{
  if (now > this->end_time_) return -1;

  const skill_detail *sd = skill_config::instance()->get_detail(this->cid_,
                                                                this->skill_lvl_);
  if (sd == NULL) return -1;

  if ((now - this->last_action_time_).msec() > (long)sd->param_2_)
  {
    this->last_action_time_ = now;

    char_obj *master = fighter_mgr::instance()->find(this->master_id_);
    if (master == NULL
        || master->scene_id() != this->scene_id_)
      return -1;

    int target_set[32] = {0};
    int ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                         sizeof(target_set)/sizeof(target_set[0]),
                                                         master->attack_target(),
                                                         OBJ_DEAD,
                                                         this->coord_x_,
                                                         this->coord_y_,
                                                         sd->param_1_,
                                                         target_set);
    if (ret <= 0) return 0;
    int sum_hurt = 0;
    for (int i = 0; i < ret; ++i)
    {
      if (this->master_id_ == target_set[i]) continue;
      char_obj *target = fighter_mgr::instance()->find(target_set[i]);
      if (target == NULL
          || master->is_proper_attack_target(target) != 0)
        continue;

      if (target->can_be_hurt(master) == 0)
      {
        int tip = 0;
        int real_hurt = master->do_calc_hurt(tip, target);
        if (tip != SK_TIP_SHAN_BI && tip != SK_TIP_HU_DUN)
        {
          if (sd->hurt_percent_ > 0)
            real_hurt = real_hurt * sd->hurt_percent_ / 100;
          real_hurt += sd->add_fixed_hurt_;
          sum_hurt += real_hurt;
          target->do_be_hurt(master, now, real_hurt, 0, this->cid_, 300);

          master->sk_do_insert_buff(target,
                                    this->cid_,
                                    real_hurt,
                                    now,
                                    sd->buff_info_,
                                    SK_BUFF_TARGET_AREA,
                                    0);
        }
        target->broadcast_be_hurt_effect(this->cid_, real_hurt, tip, master->id());
      }
      target->on_be_attacked(master);
    }
    master->on_attack_somebody(0, 0, now, this->cid_, sum_hurt);
  }
  return 0;
}
void sk_last_hurt_effect_obj::release()
{ sk_last_hurt_effect_obj_alloctor::instance()->release(this); }
static obj_pool<sk_last_hurt_effect_obj, obj_pool_std_allocator<sk_last_hurt_effect_obj> > s_sk_last_hurt_effect_obj_pool;
sk_last_hurt_effect_obj *sk_last_hurt_effect_obj_alloctor::alloc()
{ return s_sk_last_hurt_effect_obj_pool.alloc(); }
void sk_last_hurt_effect_obj_alloctor::release(sk_last_hurt_effect_obj *p)
{ s_sk_last_hurt_effect_obj_pool.release(p); }
