#include "tu_teng_mst.h"
#include "scene_mgr.h"
#include "monster_cfg.h"
#include "fighter_mgr.h"

// Lib header

int tu_teng_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  this->common_cd_      = cfg->common_cd_;
  this->eye_radius_     = cfg->eye_radius_;

  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = cfg->hp_;
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);
  return 0;
}
void tu_teng_mst::do_patrol(const time_value &now)
{
  // optimize: 场景内没有人，就没必要巡逻了
  if (scene_mgr::instance()->unit_cnt(this->scene_id_, scene_unit::PLAYER) <= 0)
    return ;
  if ((now - this->last_cure_time_).msec() < this->common_cd_)
    return ;
  this->last_cure_time_ = now;

  static int target_set[256] = {0};
  int ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                       sizeof(target_set)/sizeof(target_set[0]),
                                                       this->attack_target(),
                                                       OBJ_DEAD,
                                                       this->coord_x_,
                                                       this->coord_y_,
                                                       this->eye_radius_,
                                                       target_set);
  for (int i = 0; i < ret; ++i)
  {
    if (this->id_ == target_set[i]) continue;
    char_obj *target = fighter_mgr::instance()->find(target_set[i]);
    if (target == NULL) continue;
    target->do_add_mp(target->total_mp() * this->total_hp() / 100);
    target->do_add_hp(target->total_hp() * this->total_hp() / 100);
    target->broadcast_hp_mp();
  }
}
