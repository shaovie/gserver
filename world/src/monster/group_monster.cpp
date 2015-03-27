#include "group_monster.h"
#include "monster_pos_cfg.h"
#include "monster_cfg.h"
#include "scp_module.h"
#include "scp_mgr.h"
#include "ghz_module.h"
#include "sys_log.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

int xszc_arm_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  int hp = 10000;
  int gj = 10000;
  int fy = 10000;
  ghz_module::do_calc_world_arerage_zhan_li(hp, gj, fy);
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = (int)::ceil(hp * cfg->hp_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = (int)::ceil(gj * cfg->gong_ji_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = (int)::ceil(fy * cfg->fang_yu_ / 1000.0);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->obj_attr_.hp_ = this->total_hp();

  mst_spawn_list::_info *info = monster_pos_cfg::instance()->get_mst_spawn_info(this->scene_cid_,
                                                                                this->cid_);
  if (info != NULL)
  {
    ilist_node<coord_t> *iter = info->tar_coord_.head();
    if (iter != NULL)
    {
      for (iter = iter->next_; iter != NULL; iter = iter->next_)
        this->coord_list_.push_back(iter->value_);
    }
  }

  return 0;
}
void xszc_arm_mst::do_patrol(const time_value &now)
{
  if (this->path_.empty())
  {
    if (!this->coord_list_.empty())
    {
      coord_t to = this->coord_list_.pop_front();
      if (this->coord_list_.empty())
        this->coord_list_.push_back(to);
      this->do_find_path(coord_t(this->coord_x_, this->coord_y_), to, true);
    }

  }
  super::do_patrol(now);
}
void xszc_arm_mst::to_back(const time_value &now)
{
  this->do_status_  = DO_PATROLING;
  this->att_obj_id_ = 0;
  this->do_ai(this->back_ai_list_, now, true);

  this->path_.clear();
}
void xszc_arm_mst::do_dead(const int killer_id)
{
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_XSZC_ARM_DEAD);
  mb << killer_id;
  scp_mgr::instance()->do_something(this->scene_id_, &mb, NULL, NULL);

  super::do_dead(killer_id);
}
int xszc_defender_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  int hp = 10000;
  int gj = 10000;
  int fy = 10000;
  ghz_module::do_calc_world_arerage_zhan_li(hp, gj, fy);
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = (int)::ceil(hp * cfg->hp_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = (int)::ceil(gj * cfg->gong_ji_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = (int)::ceil(fy * cfg->fang_yu_ / 1000.0);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->obj_attr_.hp_ = this->total_hp();
  return 0;
}
void xszc_defender_mst::do_dead(const int killer_id)
{
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_XSZC_DEFENDER_DEAD);
  mb << killer_id;
  scp_mgr::instance()->do_something(this->scene_id_, &mb, NULL, NULL);

  super::do_dead(killer_id);
}
int xszc_barrack_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  int hp = 10000;
  int gj = 10000;
  int fy = 10000;
  ghz_module::do_calc_world_arerage_zhan_li(hp, gj, fy);
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = (int)::ceil(hp * cfg->hp_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = (int)::ceil(gj * cfg->gong_ji_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = (int)::ceil(fy * cfg->fang_yu_ / 1000.0);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->obj_attr_.hp_ = this->total_hp();
  return 0;
}
void xszc_barrack_mst::do_dead(const int killer_id)
{
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_XSZC_BARRACK_DEAD);
  mb << this->cid_ << killer_id;
  scp_mgr::instance()->do_something(this->scene_id_, &mb, NULL, NULL);

  super::do_dead(killer_id);
}
int xszc_main_mst::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  int hp = 10000;
  int gj = 10000;
  int fy = 10000;
  ghz_module::do_calc_world_arerage_zhan_li(hp, gj, fy);
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = (int)::ceil(hp * cfg->hp_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = (int)::ceil(gj * cfg->gong_ji_ / 1000.0);
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = (int)::ceil(fy * cfg->fang_yu_ / 1000.0);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->obj_attr_.hp_ = this->total_hp();
  return 0;
}
void xszc_main_mst::do_dead(const int killer_id)
{
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_XSZC_MAIN_DEAD);
  mb << this->group_ << killer_id;
  scp_mgr::instance()->do_something(this->scene_id_, &mb, NULL, NULL);

  super::do_dead(killer_id);
}

