#include "movable_monster_obj.h"
#include "scene_config.h"
#include "monster_cfg.h"
#include "scene_mgr.h"
#include "time_util.h"
#include "sys_log.h"
#include "util.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

DIR_STEP(s_dir_step);

movable_monster_obj::movable_monster_obj() :
  move_speed_(0),
  prev_move_speed_(0),
  patrol_speed_(0),
  patrol_radius_(0),
  patrol_interval_(0)
{ }
movable_monster_obj::~movable_monster_obj()
{ }
int movable_monster_obj::do_load_config(monster_cfg_obj *cfg)
{
  if (super::do_load_config(cfg) != 0) return -1;
  this->patrol_speed_    = cfg->patrol_speed_;
  this->patrol_radius_   = cfg->patrol_radius_;
  this->patrol_interval_ = cfg->patrol_interval_;
  return 0;
}
int movable_monster_obj::do_rebirth_init(const time_value &now)
{
  if (super::do_rebirth_init(now) != 0) return -1;

  this->move_speed_ = this->patrol_speed_;
  this->prev_move_speed_ = this->move_speed_;
  this->to_patrol();
  return 0;
}
int movable_monster_obj::can_patrol(const time_value &now)
{
  if (this->patrol_radius_ == 0) return -1;
  // optimize: 场景内没有人，就没必要巡逻了
  if (scene_mgr::instance()->unit_cnt(this->scene_id_, scene_unit::PLAYER) <= 0)
    return -1;
  if (now < this->reach_pos_time_) return -1;
  return 0;
}
void movable_monster_obj::do_patrol(const time_value &now)
{
  if (this->can_patrol(now) != 0) return ;

  if (!this->path_.empty())
  {
    this->do_move_i(now);
    if (this->path_.empty())
      this->do_calc_next_patrol_time();
    return ;
  }

  if (now >= this->next_patrol_time_)
  {
    if (this->do_calc_patrol_path() != 0)
    {
      this->to_destroy();
      return ;
    }
  }
  return ;
}
int movable_monster_obj::do_calc_patrol_path()
{
  coord_t target_pos;
  if (this->coord_x_ == this->birth_coord_x_
      && this->coord_y_ == this->birth_coord_y_)
  {
    int radius = rand() % this->patrol_radius_ + 1;
    if (radius < 3 && radius < this->patrol_radius_)
      radius = 3;
    target_pos = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                          this->coord_x_,
                                                          this->coord_y_,
                                                          this->patrol_radius_);
    if (!scene_config::instance()->can_move(this->scene_cid_,
                                            target_pos.x_,
                                            target_pos.y_)
        || !scene_mgr::instance()->can_move(this->scene_id_,
                                            target_pos.x_,
                                            target_pos.y_))
      return 0;
  }else
  {
    target_pos.x_ = this->birth_coord_x_;
    target_pos.y_ = this->birth_coord_y_;
  }

  this->do_simple_find_path(this->coord_x_,
                            this->coord_y_,
                            target_pos.x_,
                            target_pos.y_);

  return 0;
}
int movable_monster_obj::do_find_path(const coord_t &from,
                                      const coord_t &to,
                                      const bool can_chuan_tou)
{
  static coord_t path[MAX_MONSTER_FIND_PATH_DISTANCE];

  int got_steps = 0;
  if (scene_mgr::instance()->find_path(this->scene_id_,
                                       from,
                                       to,
                                       can_chuan_tou,
                                       sizeof(path) / sizeof(path[0]),
                                       path,
                                       got_steps) != 0)
  {
    e_log->wning("monster %d find path failed! in %d from %d.%d to %d.%d.",
                 this->cid(),
                 this->scene_cid_,
                 from.x_, from.y_,
                 to.x_, to.y_);
    return -1;
  }
  for (int i = 1; i < got_steps; ++i)
    this->path_.push_back(coord_t(path[i].x_, path[i].y_));
  return 0;
}
void movable_monster_obj::do_simple_find_path(const short from_x,
                                              const short from_y,
                                              const short to_x,
                                              const short to_y)
{
  short cur_x = from_x;
  short cur_y = from_y;
  while (true)
  {
    int next_dir = util::calc_next_dir(cur_x, cur_y, to_x, to_y);
    if (next_dir == DIR_XX) break; // equal
    cur_x += s_dir_step[next_dir][0];
    cur_y += s_dir_step[next_dir][1];
    if (!scene_config::instance()->can_move(this->scene_cid_,
                                            cur_x,
                                            cur_y))
      break;
    this->path_.push_back(coord_t(cur_x, cur_y));
  }
}
