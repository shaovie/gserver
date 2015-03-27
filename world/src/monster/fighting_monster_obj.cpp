#include "fighting_monster_obj.h"
#include "ltime_act_module.h"
#include "scene_config.h"
#include "dropped_item.h"
#include "monster_cfg.h"
#include "fighter_mgr.h"
#include "player_mgr.h"
#include "scene_mgr.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

typedef std::map<int/*id*/, int/*hate val*/> hate_val_map_t;
typedef std::map<int/*id*/, int/*hate val*/>::iterator hate_val_map_itor;

// impl
class hate_val_map { public : hate_val_map_t hater_map_; };

DIR_STEP(s_dir_step);

fighting_monster_obj::fighting_monster_obj() :
  change_target_(false),
  sort_(MST_SORT_COMMON),
  att_type_(ACTIVE_ATT),
  career_(0),
  eye_angle_(MST_ANGLE_GLOBAL),
  eye_radius_(0),
  chase_radius_(0),
  chase_speed_(0),
  lvl_(1),
  att_obj_id_(0),
  base_skill_cid_(0),
  refresh_time_(0),
  stiff_mian_yi_time_(800),
  first_be_attacked_time_(0),
  common_cd_(500),
  fight_back_delay_(500),
  hate_val_map_(NULL),
  birth_ai_list_(NULL),
  attack_ai_list_(NULL),
  back_ai_list_(NULL),
  die_ai_list_(NULL)
{
  hate_val_map_ = new hate_val_map();
}
fighting_monster_obj::~fighting_monster_obj()
{
  while (!this->skill_list_.empty())
  {
    skill_info *si = this->skill_list_.pop_front();
    skill_info_pool::instance()->release(si);
  }
  delete this->hate_val_map_;
  this->destroy_ai_list();
}
int fighting_monster_obj::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;

  this->career_         = cfg->career_;
  this->lvl_            = cfg->lvl_;
  this->att_type_       = cfg->att_type_;
  this->eye_angle_      = cfg->eye_angle_;
  this->eye_radius_     = cfg->eye_radius_;
  this->chase_radius_   = cfg->chase_radius_;
  this->chase_speed_    = cfg->chase_speed_;
  this->refresh_time_   = cfg->refresh_time_;
  this->common_cd_      = cfg->common_cd_;
  this->fight_back_delay_ = cfg->fight_back_delay_;
  this->change_target_  = cfg->change_target_;
  this->sort_           = cfg->sort_;
  this->stiff_mian_yi_time_ = cfg->stiff_my_time_;

  // skill
  this->base_skill_cid_ = cfg->base_skill_;
  while (!this->skill_list_.empty())
  {
    skill_info *si = this->skill_list_.pop_front();
    skill_info_pool::instance()->release(si);
  }
  skill_info *si   = skill_info_pool::instance()->alloc();
  si->cid_         = this->base_skill_cid_;
  si->rate_        = 100;
  si->lvl_         = 1;
  this->skill_list_.push_back(si);

  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]        = cfg->hp_;
  obj_attr::attr_v_add[ATTR_T_GONG_JI]   = cfg->gong_ji_;
  obj_attr::attr_v_add[ATTR_T_FANG_YU]   = cfg->fang_yu_;
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->do_load_ai(cfg);

  return 0;
}
int fighting_monster_obj::do_rebirth(const time_value &now)
{
  if (this->refresh_time_ == 0) return -1;
  if ((now - this->dead_time_).msec()
      > (long)(this->refresh_time_ + MST_ZOMBIE_TIME))
  {
    this->do_rebirth_init(now);
    if (this->do_enter_scene() != 0)
      return -1;
  }

  return 0;
}
int fighting_monster_obj::do_rebirth_init(const time_value &now)
{
  if (super::do_rebirth_init(now) != 0) return -1;

  this->do_ai(this->birth_ai_list_, now, true);

  this->next_search_time_ = now + time_value(0, 800*1000);
  this->obj_attr_.hp_ = this->total_hp();
  this->att_obj_id_ = 0;
  this->first_be_attacked_time_ = 0;
  this->hate_val_map_->hater_map_.clear();
  return 0;
}
//= aev
void fighting_monster_obj::dispatch_aev(const int aev_id, mblock *mb)
{
  if (aev_id == AEV_DEAD)
    this->aev_other_dead(mb);
  else
    super::dispatch_aev(aev_id, mb);
}
void fighting_monster_obj::aev_other_dead(mblock *mb)
{
  int id = *((int *)mb->rd_ptr());
  this->hate_val_map_->hater_map_.erase(id);
  if (id == this->att_obj_id_)
    this->att_obj_id_ = 0;
}
int fighting_monster_obj::do_search_in_view_area(const time_value &now)
{
  if (this->att_obj_id_ != 0
      || this->do_status_ == DO_BACKING
      || now < this->next_search_time_)
    return 0;
  this->next_search_time_ = now + time_value(1, 200*1000);

  this->att_obj_id_ = this->do_search_proper_target_in_view_area();
  if (this->att_obj_id_ != 0)
    this->to_attack();
  return 0;
}
int fighting_monster_obj::do_search_proper_target_in_view_area()
{
  int target_set[64] = {0};
  int ret = 0;
  if (this->eye_angle_ == MST_ANGLE_GLOBAL)
    ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                     sizeof(target_set)/sizeof(target_set[0]),
                                                     this->attack_target(),
                                                     OBJ_DEAD,
                                                     this->coord_x_,
                                                     this->coord_y_,
                                                     this->eye_radius_,
                                                     target_set);
  else if (this->eye_angle_ == MST_ANGLE_1_4)
    ret = scene_mgr::instance()->get_1_4_view_obj_set(this->scene_id_,
                                                      sizeof(target_set)/sizeof(target_set[0]),
                                                      this->attack_target(),
                                                      OBJ_DEAD,
                                                      this->coord_x_,
                                                      this->coord_y_,
                                                      this->dir_,
                                                      this->eye_radius_,
                                                      target_set);
  if (ret <= 0) return 0;
  int begin = rand() % ret;
  for (int i = begin; i < ret; ++i)
  {
    int target_id = target_set[i];
    if (this->id_ == target_id) continue;
    char_obj *target = fighter_mgr::instance()->find(target_id);
    if (target != NULL
        && this->is_proper_attack_target(target) == 0)
      return target_id;
  }
  return 0;
}
void fighting_monster_obj::on_be_hurt(char_obj *hurter,
                                      const time_value &now,
                                      const int hate_val,
                                      const int skill_hurt_delay)
{
  super::on_be_hurt(hurter, now, hate_val, skill_hurt_delay);

  int &val = this->hate_val_map_->hater_map_[hurter->id()];
  val += hate_val;

  if (BIT_DISABLED(this->unit_status_, OBJ_DEAD))
    this->do_fight_back(hurter->id());

  if (BIT_DISABLED(this->unit_status_, OBJ_DEAD))
  {
    if (now > this->stiff_mian_yi_end_time_)
    {
      this->stiff_mian_yi_end_time_ = now + time_value(now.sec(),
                                                       now.usec() + this->stiff_mian_yi_time_ * 1000);
      if (now > this->be_hurt_done_time_) // 如果僵直免疫时间太短，那么得等伤害过程走完
      {
        int delay = skill_hurt_delay + this->fight_back_delay_;
        this->be_hurt_done_time_ = time_value(now.sec(), now.usec() + delay * 1000);
      }
    }
  }else
  {
    int delay = skill_hurt_delay;
    this->be_hurt_done_time_ = time_value(now.sec(), now.usec() + delay * 1000);
  }
}
void fighting_monster_obj::do_fight_back(const int hurter_id)
{
  if (this->att_obj_id_ == 0) // 第一刀
  {
    this->att_obj_id_ = hurter_id;
    this->to_attack();
  }
  if (this->first_be_attacked_time_ == 0)
    this->first_be_attacked_time_ = time_util::now;
}
void fighting_monster_obj::do_patrol(const time_value &now)
{
  super::do_patrol(now);

  if (this->att_type_ == ACTIVE_ATT
      // optimize: 场景内没有人，就不用执行AI了
      && scene_mgr::instance()->unit_cnt(this->scene_id_, scene_unit::PLAYER) > 0)
    this->do_search_in_view_area(now);
}
void fighting_monster_obj::do_attack(const time_value &now)
{
  if (this->can_use_skill_i(now) != 0) return ;

  this->do_ai(this->attack_ai_list_, now, false);

  if (this->change_target_)
  {
    int target_id = this->do_select_another_target(0);
    if (target_id != 0)
      this->att_obj_id_ = target_id;
  }

  char_obj *target = fighter_mgr::instance()->find(this->att_obj_id_);
  if (target == NULL
      || target->scene_id() != this->scene_id_
      || this->is_proper_attack_target(target) != 0)
  {
    int target_id = this->do_select_another_target(this->att_obj_id_);
    if (target_id == 0
        || (target = fighter_mgr::instance()->find(target_id)) == NULL)
    {
      this->do_off_war(now);
      return ;
    }
    this->att_obj_id_ = target_id;
  }

  int result = 0;
  int skill_hurt_delay = 300;
  const skill_detail *sd = NULL;
  skill_info *si = this->do_select_skill(now,
                                         target,
                                         result,
                                         skill_hurt_delay,
                                         sd);
  if (si == NULL)
  {
    if (result == ERR_SKILL_OUT_OF_DISTANCE)
      this->to_chase();
    return ;
  }

  this->do_adjust_dir(target->coord_x(), target->coord_y());

  if (this->do_use_skill(this->att_obj_id_,
                         now,
                         target->coord_x(),
                         target->coord_y(),
                         si->cid_,
                         skill_hurt_delay,
                         sd) == 0)
    this->on_use_skill_ok(si, sd, skill_hurt_delay, now);
  return ;
}
int fighting_monster_obj::do_select_another_target(const int old_target)
{
  if (old_target != 0)
    this->hate_val_map_->hater_map_.erase(old_target);

  int new_target_id = this->do_find_max_hate_and_proper_target();
  if (new_target_id == 0
      && this->att_type_ == ACTIVE_ATT
      && this->eye_radius_ > 0)
    return this->do_search_proper_target_in_view_area();
  return new_target_id;
}
int fighting_monster_obj::do_find_max_hate_and_proper_target()
{
  int max_val = 0;
  int max_hater = 0;
  for (hate_val_map_itor itor = this->hate_val_map_->hater_map_.begin();
       itor != this->hate_val_map_->hater_map_.end();)
  {
    char_obj *target = fighter_mgr::instance()->find(itor->first);
    if (target == NULL
        || target->scene_id() != this->scene_id_)
    {
      this->hate_val_map_->hater_map_.erase(itor++);
      continue;
    }
    if (itor->second > max_val
        && this->is_proper_attack_target(target) == 0)
    {
      max_val  = itor->second;
      max_hater = itor->first;
    }
    ++itor;
  }
  return max_hater;
}
void fighting_monster_obj::do_adjust_dir(const short target_x, const short target_y)
{
  if (this->can_move(target_x, target_y) != 0) return ; // 有些不可移动的怪，就不用调整朝向了
  char dir = util::calc_next_dir(this->coord_x_,
                                 this->coord_y_,
                                 target_x,
                                 target_y);
  if (dir != DIR_XX && dir != this->dir_)
  {
    this->dir_  = dir;
    this->broadcast_position();
  }
}
skill_info *fighting_monster_obj::do_select_skill(const time_value &now,
                                                  char_obj *target,
                                                  int &result,
                                                  int &skill_hurt_delay,
                                                  const skill_detail *&sd)
{
  skill_info *base_skill_si = NULL;
  skill_info *select_skill_si = NULL;
  if (this->skill_list_.size() == 1) // only base skill
  {
    base_skill_si = this->skill_list_.head()->value_;
  }else
  {
    int sk_rand_rate = rand() % 100 + 1;
    int sk_rate_1 = 0;
    int sk_rate_2 = 0;
    bool first_sk = true;
    for (ilist_node<skill_info *> *itor = this->skill_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      skill_info *si = itor->value_;

      if (si->cid_ == this->base_skill_cid_)
        base_skill_si = si;
      else
      {
        util::cake_rate(si->rate_, first_sk, sk_rate_1, sk_rate_2);
        if (sk_rand_rate >= sk_rate_1 && sk_rand_rate <= sk_rate_2)
        {
          select_skill_si = si;
          break;
        }
      }
    }
    if (select_skill_si != NULL) // 如果选中的技能 施放条件不满足，那么就尝试使用普通攻击
    {
      const skill_cfg_obj *sco = skill_config::instance()->get_skill(select_skill_si->cid_);
      if (sco != NULL)
      {
        skill_hurt_delay = sco->hurt_delay_;
        sd = sco->get_detail(select_skill_si->lvl_);
        if (sd != NULL)
        {
          result = this->can_use_skill(sd,
                                       select_skill_si,
                                       target->block_radius(),
                                       now,
                                       coord_t(target->coord_x(), target->coord_y()));
          if (result != 0)
            select_skill_si = NULL;
          if (result == ERR_SKILL_OUT_OF_DISTANCE) // 如果技能施放距离不够，那么开始追击
            return select_skill_si;
        }
      }
    }
  }

  if (select_skill_si == NULL) // 如果没有选中技能
  {
    select_skill_si = base_skill_si;
    const skill_cfg_obj *sco = skill_config::instance()->get_skill(select_skill_si->cid_);
    if (sco != NULL)
    {
      sd = sco->get_detail(select_skill_si->lvl_);
      skill_hurt_delay = sco->hurt_delay_;
      if (sd != NULL)
      {
        result = this->can_use_skill(sd,
                                     select_skill_si,
                                     target->block_radius(),
                                     now,
                                     coord_t(target->coord_x(), target->coord_y()));
        if (result != 0)
          select_skill_si = NULL;
        if (result == ERR_SKILL_OUT_OF_DISTANCE)
          return select_skill_si;
      }
    }
  }

  return select_skill_si;
}
void fighting_monster_obj::do_chase(const time_value &now)
{
  if (this->can_move(0, 0) != 0)
  {
    this->on_chase_failed();
    return ;
  }
  if (now < this->reach_pos_time_
      || now < this->be_hurt_done_time_
      || now < this->attack_done_time_)
    return ;

  char_obj *target = fighter_mgr::instance()->find(this->att_obj_id_);
  if (target == NULL
      || target->scene_id() != this->scene_id_)
  {
    this->to_attack(); // 如果追击目标无效，那么让战斗流程去更换目标
    return ;
  }

  // 超出怪物活动范围，回出生点
  if (util::is_out_of_distance(this->birth_coord_x_,
                               this->birth_coord_y_,
                               this->coord_x_,
                               this->coord_y_,
                               this->chase_radius_))
  {
    this->to_back(now);
    return ;
  }

  // 先根据方向找下个最近坐标，如果遇到障碍再A*寻路
  int next_dir = util::calc_next_dir(this->coord_x_,
                                     this->coord_y_,
                                     target->coord_x(),
                                     target->coord_y());
  if (next_dir == DIR_XX) // 坐标相同
  {
    this->to_attack();
    return ;
  }
  short next_x = this->coord_x_ + s_dir_step[next_dir][0];
  short next_y = this->coord_y_ + s_dir_step[next_dir][1];
  if (!scene_config::instance()->can_move(this->scene_cid_,
                                          next_x,
                                          next_y))
  {
    coord_t path[2];
    int got_steps = 0;
    if (scene_mgr::instance()->find_path(this->scene_id_,
                                         coord_t(this->coord_x_, this->coord_y_),
                                         coord_t(target->coord_x(), target->coord_y()),
                                         true,
                                         sizeof(path) / sizeof(path[0]),
                                         path,
                                         got_steps) != 0)
    {
      e_log->wning("monster %d chase failed! in %d from %d.%d to %d.%d.",
                   this->cid(),
                   this->scene_cid_,
                   this->coord_x_, this->coord_y_,
                   target->coord_x(), target->coord_y());
      this->to_destroy();
      return ;
    }
    if (got_steps > 1)
      this->do_move_i(now, path[1].x_, path[1].y_);
  }else // scene can move
  {
    if (this->block_radius() > 0
        || scene_mgr::instance()->can_move(this->scene_id_, next_x, next_y))
      this->do_move_i(now, next_x, next_y);
    else // 如果前方有目标，那么尝试绕开
    {
      for (int i = 1; i < 8; ++i)
      {
        if (rand() % 3) ++next_dir;
        else if (next_dir > DIR_UP) --next_dir;
        else next_dir = DIR_LEFT_UP;
        if (next_dir > DIR_LEFT_UP) next_dir = DIR_LEFT_UP;

        next_x = this->coord_x_ + s_dir_step[next_dir][0];
        next_y = this->coord_y_ + s_dir_step[next_dir][1];
        if (scene_mgr::instance()->can_move(this->scene_id_, next_x, next_y)
            && scene_config::instance()->can_move(this->scene_cid_,
                                                  next_x,
                                                  next_y))
        {
          this->do_move_i(now, next_x, next_y);
          break;
        }
      }
    }
  }

  this->to_attack();
}
void fighting_monster_obj::to_back(const time_value &now)
{
  this->do_status_  = DO_BACKING;
  this->att_obj_id_ = 0;
  this->first_be_attacked_time_ = 0;
  this->hate_val_map_->hater_map_.clear();
  buff_module::do_remove_all_buff(this);
  this->do_ai(this->back_ai_list_, now, true);

  if (this->coord_x_ != this->birth_coord_x_
      || this->coord_y_ != this->birth_coord_y_)
  {
    this->path_.clear();
    this->do_simple_find_path(this->coord_x_,
                              this->coord_y_,
                              this->birth_coord_x_,
                              this->birth_coord_y_);
    if (this->path_.empty())
    {
      this->do_flash_move(this->birth_coord_x_,
                          this->birth_coord_y_,
                          this->birth_dir_);
    }
  }
}
void fighting_monster_obj::do_back(const time_value &now)
{
  if (now < this->reach_pos_time_) return ;

  // 超出怪物活动范围，回出生点
  if (this->birth_coord_x_ == this->coord_x_
      && this->birth_coord_y_ == this->coord_y_)
  {
    if (this->birth_dir_ != DIR_XX
        && this->dir_ != this->birth_dir_) // do_adjust_dir
    {
      this->dir_ = this->birth_dir_;
      this->broadcast_position();
    }
    this->on_back();
    return ;
  }

  if (this->path_.empty()) // 路径太长，闪移回去
  {
    char dir = this->birth_dir_;
    if (dir == DIR_XX)
      dir = rand() % 8 + 1;
    this->do_flash_move(this->birth_coord_x_,
                        this->birth_coord_y_,
                        dir);
    this->on_back();
    return ;
  }

  this->do_move_i(now);
}
void fighting_monster_obj::on_back()
{
  monster_cfg_obj *cfg
    = monster_cfg::instance()->get_monster_cfg_obj(this->cid_);
  if (cfg != NULL)
    this->do_load_ai(cfg);
  this->to_patrol();
}
void fighting_monster_obj::do_dead(const int killer_id)
{
  if (this->can_switch_to_status(ST_DIE_IN_SCENE) != 0) return ;
  this->status_ = ST_DIE_IN_SCENE;

  this->broadcast_unit_life_status(LIFE_V_DEAD);
  time_value now = time_value::gettimeofday();
  this->dead_time_ = now;
  buff_module::do_remove_all_buff(this);

  this->do_ai(this->die_ai_list_, now, true);

  int dropped_item_owner_id = 0;
  char_obj *killer = fighter_mgr::instance()->find(killer_id);
  if (killer != NULL
      && killer->scene_id() == this->scene_id_)
  {
    dropped_item_owner_id = killer->master_id();
    this->do_assign_award(killer->master_id());
  }

  // drop item
  dropped_item::do_clear_dropped_item_list();
  int drop_times = ltime_act_module::on_drop_items();
  for (int i = 0; i < drop_times; ++i)
  {
    dropped_item::do_build_drop_item_list(dropped_item_owner_id,
                                          this->id_,
                                          this->cid_,
                                          dropped_item::dropped_item_list);
  }
  this->do_drop_items_on_scene(dropped_item::dropped_item_list, DROP_AREA_RADIUS);
}
void fighting_monster_obj::do_ji_tui(char_obj *attacker,
                                     const time_value &/*now*/,
                                     const int back_dis,
                                     const int skill_cid)
{
  if (back_dis <= 0
      || this->sort_ == MST_SORT_BOSS
      || this->can_be_moved() != 0)
    return ;

  int n_dir = util::calc_next_dir(attacker->coord_x(),
                                  attacker->coord_y(),
                                  this->coord_x_,
                                  this->coord_y_);
  if (n_dir == DIR_XX) n_dir = attacker->dir();

  short target_x = this->coord_x_;
  short target_y = this->coord_y_;
  for (int i = 0; i < back_dis; ++i)
  {
    target_x += s_dir_step[n_dir][0];
    target_y += s_dir_step[n_dir][1];
  }
  if (!scene_config::instance()->can_move(this->scene_cid_,
                                          target_x,
                                          target_y))
    return ;
  if (this->do_move(target_x, target_y, this->dir()) != 0) return ;
  this->broadcast_ji_tui(skill_cid, target_x, target_y);
}
void fighting_monster_obj::do_clear_hate()
{ this->hate_val_map_->hater_map_.clear(); }
