#include "monster_obj.h"
#include "sys_log.h"
#include "monster_mgr.h"
#include "monster_cfg.h"
#include "time_util.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "team_mgr.h"
#include "mblock_pool.h"
#include "scene_mgr.h"
#include "ltime_act_module.h"

// Lib header
#include <stdlib.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

monster_obj::monster_obj() :
  birth_dir_(DIR_XX),
  birth_coord_x_(0),
  birth_coord_y_(0),
  status_(ST_NULL),
  do_status_(DO_PATROLING),
  live_time_(0),
  birth_time_(0),
  killer_id_(0)
{ }
monster_obj::~monster_obj()
{
  monster_mgr::instance()->destroy_in_scene(this->scene_id_, this->id_);
  monster_mgr::instance()->remove(this->id_);
  this->do_release_id();
}
void monster_obj::do_close()
{ delete this; }
void monster_obj::to_destroy()
{
  if (this->status_ == ST_TO_DESTROY) return ; // avoid warning log
  if (this->can_switch_to_status(ST_TO_DESTROY) != 0) return ;
  this->status_ = ST_TO_DESTROY;
}
int monster_obj::do_timeout(const time_value &now)
{
  // handle pending events at first.
  this->handle_aev_queue();

  int ret = 0;
  if (this->status_ == ST_ENTER_SCENE)
  {
    ret = this->do_action_in_scene(now);
  }else if (this->status_ == ST_DELAY_DIE)
  {
    if (now > this->be_hurt_done_time_)
      this->do_dead(this->killer_id_);
  }else if (this->status_ == ST_DIE_IN_SCENE)
  {
    if ((now - this->dead_time_).msec() >= MST_ZOMBIE_TIME)
      this->do_exit_scene();
  }else if (this->status_ == ST_EXIT_SCENE)
  {
    ret = this->do_rebirth(now);
  }else if (this->status_ == ST_TO_DESTROY)
  {
    this->do_exit_scene();
    ret = -1;
  }else if (this->status_ == ST_NULL)
  {
    this->to_destroy();
  }else if (this->status_ == ST_DELAY_ENTER_SCENE)
  {
    if (now >= this->enter_scene_time_)
    {
      if (this->do_enter_scene() != 0)
        ret = -1;
      this->birth_time_ = now.sec();
    }
  }
  return ret;
}
int monster_obj::init(const int mst_cid,
                      const int scene_id,
                      const int scene_cid,
                      const char dir,
                      const short x,
                      const short y)
{
  this->id_             = this->do_assign_id();
  if (this->id_ == -1) return -1;
  monster_mgr::instance()->insert(this->id_, this);
  this->cid_            = mst_cid;
  this->scene_id_       = scene_id;
  this->scene_cid_      = scene_cid;
  this->birth_dir_      = dir;
  this->birth_coord_x_  = x;
  this->birth_coord_y_  = y;

  return this->do_rebirth_init(time_value::gettimeofday());
}
int monster_obj::do_assign_id()
{ return monster_mgr::instance()->assign_monster_id(); }
void monster_obj::do_release_id()
{ monster_mgr::instance()->release_monster_id(this->id_); }
int monster_obj::do_rebirth_init(const time_value &now)
{
  monster_cfg_obj *cfg 
    = monster_cfg::instance()->get_monster_cfg_obj(this->cid_);
  if (cfg == NULL)
  {
    e_log->wning("not found monster cfg %d where %s", this->cid_, __func__);
    return -1;
  }
  if (this->do_load_config(cfg) != 0)
    return -1;

  this->dir_              = this->birth_dir_;
  this->coord_x_          = this->birth_coord_x_;
  this->coord_y_          = this->birth_coord_y_;
  if (this->birth_dir_ == DIR_XX)
    this->dir_ = rand() % 8 + 1;
  this->status_           = ST_NULL;
  this->do_status_        = DO_PATROLING;
  this->unit_status_      = 0;
  this->birth_time_       = now.sec();

  return 0;
}
int monster_obj::do_load_config(monster_cfg_obj *cfg)
{
  this->block_radius_ = cfg->block_radius_;
  this->live_time_    = cfg->live_time_;
  return 0;
}
int monster_obj::do_activate(const int delay)
{
  if (delay == 0)
  {
    if (this->do_enter_scene() != 0)
      return -1;
  }else
  {
    this->enter_scene_time_ = time_value::gettimeofday() + time_value(0, delay * 1000);
    this->status_ = ST_DELAY_ENTER_SCENE;
  }
  monster_mgr::instance()->attach_timer(this);
  monster_mgr::instance()->birth_to_scene(this->scene_id_, this->id_);
  this->on_activated();
  return 0;
}
int monster_obj::do_enter_scene()
{
  if (this->can_switch_to_status(ST_ENTER_SCENE) != 0) return -1;
  if (char_obj::do_enter_scene() != 0) return -1;
  this->status_ = ST_ENTER_SCENE;

  this->broadcast_enter_scene();
  return 0;
}
int monster_obj::do_exit_scene()
{
  if (this->can_switch_to_status(ST_EXIT_SCENE) != 0) return -1;
  char_obj::do_exit_scene();
  this->status_ = ST_EXIT_SCENE;
  return 0;
}
int monster_obj::do_action_in_scene(const time_value &now)
{
  if (this->live_time_ > 0
      && (now.sec() - this->birth_time_) > this->live_time_)
  {
    this->to_destroy();
    return 0;
  }

  if (this->do_think(now) != 0) return 0;

  this->do_update_all_buff(now);

  if (this->do_status_ == DO_PATROLING)
    this->do_patrol(now);
  else if (this->do_status_ == DO_ATTACKING)
    this->do_attack(now);
  else if (this->do_status_ == DO_CHASING)
    this->do_chase(now);
  else if (this->do_status_ == DO_BACKING)
    this->do_back(now);

  return 0;
}
int monster_obj::can_switch_to_status(const int to_st)
{
  if (this->status_ == ST_ENTER_SCENE)
  {
    if (to_st == ST_EXIT_SCENE
        || to_st == ST_DELAY_DIE
        || to_st == ST_TO_DESTROY)
      return 0;
  }else if (this->status_ == ST_EXIT_SCENE)
  {
    if (to_st == ST_ENTER_SCENE
        || to_st == ST_TO_DESTROY)
      return 0;
  }else if (this->status_ == ST_DELAY_DIE)
  {
    if (to_st == ST_DIE_IN_SCENE
        || to_st == ST_TO_DESTROY)
      return 0;
  }else if (this->status_ == ST_DIE_IN_SCENE)
  {
    if (to_st == ST_ENTER_SCENE
        || to_st == ST_EXIT_SCENE
        || to_st == ST_TO_DESTROY)
      return 0;
  }else if (this->status_ == ST_TO_DESTROY)
  {
    if (to_st == ST_EXIT_SCENE)
      return 0;
  }else if (this->status_ == ST_DELAY_ENTER_SCENE)
  {
    if (to_st == ST_ENTER_SCENE
        || to_st == ST_TO_DESTROY)
      return 0;
  }else if (this->status_ == ST_NULL)
    return 0;

  e_log->rinfo("msonter %d:%d in %d:%d switch to_status from %d to %d failed!",
               this->id_,
               this->cid_,
               this->scene_cid(),
               this->scene_id(),
               this->status_,
               to_st);
  return -1;
}
bool monster_obj::do_build_snap_info(const int /*char_id*/,
                                     out_stream &os,
                                     ilist<pair_t<int> > *old_snap_unit_list,
                                     ilist<pair_t<int> > *new_snap_unit_list)
{
  new_snap_unit_list->push_back(pair_t<int>(this->id_, this->cid_));

  if (this->is_new_one(old_snap_unit_list))
  {
    os << T_CID_ID << this->cid_ << this->id_;
    this->do_build_snap_pos_info(os); 
    this->do_build_snap_hp_mp_info(os); 
    os << T_MST_EXTRA_INFO << this->master_id();

    os << T_END;
    return true;
  }

  return false;
}
void monster_obj::do_build_snap_pos_info(out_stream &os)
{
  os << T_POS_INFO
    << this->dir_
    << this->coord_x_
    << this->coord_y_
    << this->move_speed();
}
//= aev
void monster_obj::dispatch_aev(const int aev_id, mblock *mb)
{
#define AEV_SHORT_CODE(ID, FUNC) case ID:  \
  this->FUNC(mb);                          \
  break
#define AEV_SHORT_DEFAULT default:         \
  e_log->error("unknow aev id %d", aev_id);\
  break

  //
  switch (aev_id)
  {
    AEV_SHORT_CODE(AEV_BE_KILLED, aev_be_killed);
  }
}
void monster_obj::aev_be_killed(mblock *mb)
{
  this->killer_id_ = *((int *)mb->rd_ptr());

  if (this->can_switch_to_status(ST_DELAY_DIE) != 0) return ;
  this->status_ = ST_DELAY_DIE;
}
void monster_obj::do_assign_award(const int owner_id)
{
  player_obj *owner = player_mgr::instance()->find(owner_id);
  if (owner == NULL
      || owner->scene_id() != this->scene_id_)
    return ;

  team_info *ti = team_mgr::instance()->find(owner->team_id());
  if (ti == NULL)
  {
    this->do_assign_award_to_char(owner->id());
    return ;
  }

  int valid_team_member[MAX_TEAM_MEMBER];
  int valid_team_member_cnt = 0;
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (ti->mem_id_[i] != 0)
    {
      if (ti->mem_id_[i] == owner->id()
          || owner->is_in_my_view(ti->mem_id_[i]))
        valid_team_member[valid_team_member_cnt++] = ti->mem_id_[i];
    }
  }

  for (int i = 0; i < valid_team_member_cnt; ++i)
    this->do_assign_award_to_char(valid_team_member[i]);
}
void monster_obj::do_assign_award_to_char(const int char_id)
{
  monster_cfg_obj *mco = monster_cfg::instance()->get_monster_cfg_obj(this->cid_);
  if (mco == NULL) return ;
  player_obj *owner = player_mgr::instance()->find(char_id);
  if (owner == NULL) return ;

  { // kill monster
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
    *mb << this->cid_ << (int)mco->sort_;
    owner->post_aev(AEV_KILL_MONSTER, mb);
  }

  { // exp
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
    int award_exp = ltime_act_module::on_kill_mst_got_exp(mco->exp_);
    *mb << award_exp;
    owner->post_aev(AEV_GOT_EXP, mb);
  }
}
void monster_obj::broadcast_enter_scene()
{
  if (scene_mgr::instance()->unit_cnt(this->scene_id_, scene_unit::PLAYER) <= 0)
    return ;
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
  out_stream os(mb->wr_ptr(), mb->space());
  os << NTF_BROADCAST_MST_ENTER_SCENE << this->id_;
  mb->wr_ptr(os.length());
  player_mgr::instance()->post_aev_to_view_area(this->scene_id_,
                                                this->coord_x_,
                                                this->coord_y_,
                                                AEV_DIRECT_SEND_MSG,
                                                mb);
}
