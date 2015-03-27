#include "ghz_activity_obj.h"
#include "sys_log.h"
#include "scene_config.h"
#include "activity_cfg.h"
#include "global_param_cfg.h"
#include "activity_module.h"
#include "guild_module.h"
#include "ghz_module.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "activity_mgr.h"
#include "monster_mgr.h"
#include "monster_obj.h"
#include "spawn_monster.h"
#include "time_util.h"
#include "notice_module.h"
#include "mblock_pool.h"

// Lib header
#include <set>
#include "mblock.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("ghz");
static ilog_obj *e_log = err_log::instance()->get_ilog("ghz");

static ilist<int/*mst cid*/> s_spawn_scene_mst_list;

class shou_wei_info
{
public:
  shou_wei_info() : state_(0), dir_(DIR_DOWN), x_(0), y_(0), idx_(0), cid_(0) { }

  char state_;  // 0 dead,  1 live
  char dir_;
  short x_;
  short y_;
  int idx_;
  int cid_;
};
static ilist<shou_wei_info *> s_ghz_shou_wei_list; // first:cid second:idx

int ghz_activity_obj::parse_shou_wei_idx_cfg(const char *s)
{
  while (!s_ghz_shou_wei_list.empty())
    delete s_ghz_shou_wei_list.pop_front();

  char bf[256] = {0};
  ::strncpy(bf, s, sizeof(bf) - 1);

  char *tok_p = NULL;
  char *token = NULL;
  for (token = ::strtok_r(bf, ",", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, ",", &tok_p))
  {
    shou_wei_info *p = new shou_wei_info();
    if (::sscanf(token, "%d:%d", &p->cid_, &p->idx_) != 2
        || p->idx_ <= 0
        || p->idx_ > GHZ_SHOU_WEI_CNT)
    {
      delete p;
      return -1;
    }
    s_ghz_shou_wei_list.push_back(p);
  }
  if (s_ghz_shou_wei_list.size() > GHZ_SHOU_WEI_CNT)
    return -1;
  return 0;
}
int ghz_activity_obj::get_shou_wei_idx(const int cid)
{
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (cid == itor->value_->cid_)
      return itor->value_->idx_;
  }
  return 0;
}
void ghz_activity_obj::set_shou_wei_pos(const int cid,
                                        const char dir,
                                        const short x,
                                        const short y)
{
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (cid == itor->value_->cid_)
    {
      itor->value_->dir_ = dir;
      itor->value_->x_ = x;
      itor->value_->y_ = y;
      return ;
    }
  }
}
int ghz_activity_obj::get_shou_wei_pos(const int cid, char &dir, coord_t &pos)
{
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (cid == itor->value_->cid_)
    {
      dir = itor->value_->dir_;
      pos.x_ = itor->value_->x_;
      pos.y_ = itor->value_->y_;
      return 0;
    }
  }
  return -1;
}
char ghz_activity_obj::get_shou_wei_state(const int cid)
{
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (cid == itor->value_->cid_)
      return itor->value_->state_;
  }
  return 0; // dead
}
void ghz_activity_obj::set_shou_wei_state(const int cid, const char v)
{
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (cid == itor->value_->cid_)
    {
      itor->value_->state_ = v;
      return ;
    }
  }
}
ghz_activity_obj::ghz_activity_obj() :
  activity_obj(ACTIVITY_GHZ),
  fight_over_(true),
  ghz_state_(IN_PREPARING),
  wang_zuo_id_(0),
  begin_time_(0)
{ }
int ghz_activity_obj::init()
{
  this->spawn_monster_not_in_activity();
  return 0;
}
int ghz_activity_obj::open()
{
  this->wang_zuo_id_ = 0;
  this->shou_wei_list_.clear();
  this->fight_over_ = false;
  this->begin_time_ = time_util::now;
  this->ghz_state_ = IN_PREPARING;

  // 活动开始的时候把此场景中不相关的怪全部清掉（包括主宰分身）
  monster_mgr::instance()->destroy_scene_monster(global_param_cfg::ghz_scene_cid);

  spawn_monster::spawn_mst_list.clear();
  spawn_monster::spawn_scene_monster(global_param_cfg::ghz_scene_cid,
                                     global_param_cfg::ghz_scene_cid,
                                     0,
                                     &(spawn_monster::spawn_mst_list),
                                     &s_spawn_scene_mst_list);
  while (!spawn_monster::spawn_mst_list.empty())
  {
    pair_t<int> p = spawn_monster::spawn_mst_list.pop_front();
    if (ghz_activity_obj::get_shou_wei_idx(p.second_) > 0)
    {
      this->shou_wei_list_.push_back(p.first_);
      monster_obj *mo = monster_mgr::instance()->find(p.first_);
      if (mo != NULL)
      {
        ghz_activity_obj::set_shou_wei_pos(p.second_,
                                           mo->dir(),
                                           mo->coord_x(),
                                           mo->coord_y());
        ghz_activity_obj::set_shou_wei_state(p.second_, 1);
      }
    }
  }

  scene_coord_t to_relive_coord;
  to_relive_coord = scene_config::instance()->relive_coord(global_param_cfg::ghz_scene_cid);
  if (to_relive_coord.cid_ != 0)
  {
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*3);
    *mb << to_relive_coord.cid_ << to_relive_coord.coord_.x_ << to_relive_coord.coord_.y_;
    player_mgr::instance()->post_aev_to_scene(global_param_cfg::ghz_scene_cid,
                                              AEV_TRANSFER_TO,
                                              mb);
  }
  notice_module::ghz_before_opened(global_param_cfg::ghz_prepare_time/60);
  ghz_module::on_ghz_open();
  return 0;
}
int ghz_activity_obj::left_time()
{
  int lt = super::left_time() - global_param_cfg::ghz_prepare_time;
  return lt < 0 ? 0 : lt;
}
int ghz_activity_obj::do_ai(const int)
{
  if (this->fight_over_) return 0;
  
  if (this->ghz_state_ == IN_PREPARING
      && time_util::now - this->begin_time_ > global_param_cfg::ghz_prepare_time)
  {
    this->ghz_state_ = IN_FIGHTING;
    notice_module::ghz_opened();
  }
  return 0;
}
void ghz_activity_obj::do_something(mblock *mb, int *, mblock *ret_mb)
{
  if (mb->data_type() == ACT_EV_GHZ_WANG_ZUO_ACTIVATED)
    this->wang_zuo_id_ = *((int *)mb->rd_ptr());
  else if (mb->data_type() == ACT_EV_GHZ_WANG_ZUO_DEAD)
  {
    int killer_id = 0;
    *mb >> killer_id;
    int win_guild_id = guild_module::get_guild_id(killer_id);
    ghz_module::on_ghz_over(win_guild_id, killer_id);
    this->fight_over_ = true;
    this->on_ghz_over();
    this->on_activity_closed();
  }else if (mb->data_type() == ACT_EV_GHZ_SHOU_WEI_DEAD)
  {
    int id = 0;
    int cid = 0;
    *mb >> id >> cid;
    int s = this->shou_wei_list_.size();
    for (int i = 0; i < s; ++i)
    {
      int v = this->shou_wei_list_.pop_front();
      if (v != id)
        this->shou_wei_list_.push_back(v);
    }
    ghz_activity_obj::set_shou_wei_state(cid, 0);
  }else if (mb->data_type() == ACT_EV_GHZ_SHOU_WEI_LIVE)
  {
    int id = 0;
    int cid = 0;
    *mb >> id >> cid;
    this->shou_wei_list_.push_back(id);
    ghz_activity_obj::set_shou_wei_state(cid, 1);
  }else if (mb->data_type() == ACT_EV_GHZ_OBTAIN_FIGHTING_INFO)
  {
    monster_obj *wang_zuo = monster_mgr::instance()->find(this->wang_zuo_id_);
    if (wang_zuo != NULL) *ret_mb << wang_zuo->hp() << wang_zuo->total_hp();
    else *ret_mb << (int)0 << (int)0;

    *ret_mb << (char)s_ghz_shou_wei_list.size();
    for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
         itor != NULL;
         itor = itor->next_)
    {
      ilist_node<int> *mitor = this->shou_wei_list_.head();
      int hp = 0;
      int total_hp = 0;
      for (; mitor != NULL; mitor = mitor->next_)
      {
        monster_obj *mo = monster_mgr::instance()->find(mitor->value_);
        if (mo != NULL
            && mo->cid() == itor->value_->cid_)
        {
          hp = mo->hp();
          total_hp = mo->total_hp();
          break;
        }
      }
      *ret_mb << itor->value_->cid_ << hp << total_hp;
    }
  }
}
int ghz_activity_obj::close()
{
  // timeout
  if (!this->fight_over_)
  {
    ghz_module::on_ghz_over(0, 0);

    this->on_ghz_over();
  }

  this->spawn_monster_not_in_activity();
  return 0;
}
void ghz_activity_obj::spawn_monster_not_in_activity()
{
  ilist<int> exclude_mst_list;
  exclude_mst_list.push_back(global_param_cfg::wang_zuo_cid);
  for (ilist_node<shou_wei_info *> *itor = s_ghz_shou_wei_list.head();
       itor != NULL;
       itor = itor->next_)
    exclude_mst_list.push_back(itor->value_->cid_);

  spawn_monster::spawn_mst_list.clear();
  spawn_monster::spawn_scene_monster(global_param_cfg::ghz_scene_cid,
                                     global_param_cfg::ghz_scene_cid,
                                     0,
                                     &spawn_monster::spawn_mst_list,
                                     &exclude_mst_list);
  for (ilist_node<pair_t<int> > *itor = spawn_monster::spawn_mst_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (!s_spawn_scene_mst_list.find(itor->value_.second_))
      s_spawn_scene_mst_list.push_back(itor->value_.second_);
  }
}
void ghz_activity_obj::on_ghz_over()
{
  monster_obj *mo = monster_mgr::instance()->find(this->wang_zuo_id_);
  if (mo != NULL)
    mo->to_destroy();
  while (!this->shou_wei_list_.empty())
  {
    int v = this->shou_wei_list_.pop_front();
    mo = monster_mgr::instance()->find(v);
    if (mo != NULL)
      mo->to_destroy();
  }
}
int ghz_activity_obj::can_enter(player_obj *player)
{
  if (!this->is_opened()) return 0;
  if (this->ghz_state_ == IN_PREPARING)
  {
    if (guild_module::get_ghz_winner() == 0) // first
       return ERR_GHZ_PREPARING;

    if (player->guild_id() != guild_module::get_ghz_winner())
      return ERR_GHZ_PREPARING;
  }
  return 0;
}
