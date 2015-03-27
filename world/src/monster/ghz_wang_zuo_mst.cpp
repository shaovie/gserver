#include "ghz_wang_zuo_mst.h"
#include "global_param_cfg.h"
#include "monster_cfg.h"
#include "ghz_module.h"
#include "guild_module.h"
#include "fighter_mgr.h"
#include "sys_log.h"
#include "error.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

ghz_wang_zuo_mst::ghz_wang_zuo_mst() :
  guild_id_(0)
{ }
void ghz_wang_zuo_mst::on_activated()
{
  if (this->scene_cid_ == global_param_cfg::ghz_scene_cid)
    ghz_module::on_wang_zuo_activated(this->id_);
}
int ghz_wang_zuo_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  int hp = 10000;
  int gj = 10000;
  int fy = 10000;
  ghz_module::do_calc_world_arerage_zhan_li(hp, gj, fy);
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = (int)::ceil(double(hp) * double(cfg->hp_) / 1000.0);
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = (int)::ceil(double(gj) * double(cfg->gong_ji_) / 1000.0);
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = (int)::ceil(double(fy) * double(cfg->fang_yu_) / 1000.0);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->guild_id_ = guild_module::get_ghz_winner();
  this->obj_attr_.hp_ = this->total_hp();
  return 0;
}
void ghz_wang_zuo_mst::do_dead(const int killer_id)
{
  super::do_dead(killer_id);

  char_obj *killer = fighter_mgr::instance()->find(killer_id);
  if (killer != NULL
      && killer->scene_id() == this->scene_id_)
    ghz_module::on_wang_zuo_dead(killer->master_id());
}
int ghz_wang_zuo_mst::can_attack(char_obj *target)
{
  if (this->guild_id_ != 0
      && target->guild_id() == this->guild_id_)
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  return super::can_attack(target);
}
int ghz_wang_zuo_mst::can_be_attacked(char_obj *attacker)
{
  if (this->guild_id_ != 0
      && attacker->guild_id() == this->guild_id_)
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  return super::can_be_attacked(attacker);
}
