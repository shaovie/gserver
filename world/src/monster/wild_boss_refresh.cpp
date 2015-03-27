#include "wild_boss_refresh.h"
#include "wild_boss_config.h"
#include "message.h"
#include "monster_mgr.h"
#include "spawn_monster.h"
#include "notice_module.h"
#include "scene_config.h"
#include "client.h"
#include "sys_log.h"
#include "time_util.h"
#include "istream.h"
#include "player_mgr.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("wild_boss");
static ilog_obj *e_log = err_log::instance()->get_ilog("wild_boss");

class wild_boss_refresh_impl
{
public:
  typedef std::tr1::unordered_map<int, wild_boss_refresh::wild_boss *> wild_boss_map_t;
  typedef std::tr1::unordered_map<int, wild_boss_refresh::wild_boss *>::iterator wild_boss_map_itor;

  wild_boss_refresh_impl()
    : last_check_wild_boss_time_(0),
    wild_boss_map_(10)
  { }
  ~wild_boss_refresh_impl()
  {
    for (wild_boss_map_itor itor = this->wild_boss_map_.begin();
         itor != this->wild_boss_map_.end();
         ++itor)
      delete itor->second;
  }
  int do_timeout(const int now)
  {
    if ((now - this->last_check_wild_boss_time_) < 5)
      return 0;
    this->last_check_wild_boss_time_ = now;

    bool boss_appear = false;
    for (ilist_node<wild_boss_config::wild_boss*> *current
         = wild_boss_config::instance()->get_head();
         current != NULL;
         current = current->next_)
    {
      wild_boss_config::wild_boss* boss_cfg = current->value_;
      if (boss_cfg == NULL)
        continue;

      wild_boss_refresh::wild_boss *boss = NULL;
      wild_boss_map_itor itor = this->wild_boss_map_.find(boss_cfg->index_);
      if (itor == this->wild_boss_map_.end())
      {
        boss = new wild_boss_refresh::wild_boss();
        this->wild_boss_map_.insert(std::make_pair(boss_cfg->index_, boss));
      }else
        boss = itor->second;

      // boss dead
      if (boss->monster_id_ != 0
          && monster_mgr::instance()->find(boss->monster_id_) == NULL)
      {
        boss->monster_id_ = 0;
        boss->next_appear_time_ = now + boss_cfg->interval_;
      }else if (boss->monster_id_ == 0 && boss->next_appear_time_ <= now)
      {
        //refresh boss
        if (this->refresh_boss(boss_cfg, boss) == 0)
        {
          boss_appear = true;
          notice_module::boss_appear(boss_cfg->monster_cid_, boss_cfg->scene_cid_);
        }
      }
    }
    if (boss_appear)
    {
      mblock mb(client::send_buf, client::send_buf_len);
      mb.wr_ptr(sizeof(proto_head));
      player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_BOSS_APPEAR, &mb);
    }
    return 0;
  }
  int refresh_boss(wild_boss_config::wild_boss *boss_cfg,
                   wild_boss_refresh::wild_boss *boss)
  {
    boss->monster_cid_ = boss_cfg->monster_cid_;
    if (boss_cfg->coord_.empty())
    {
      e_log->rinfo("refresh boss[%d] in scene[%d] failed![no coord]",
                   boss_cfg->monster_cid_, boss_cfg->scene_cid_);
      return -1;
    }
    ilist_node<coord_t> *head = boss_cfg->coord_.head();
    int chosen = rand() % boss_cfg->coord_.size() + 1;
    for (int i = 1; i != chosen; ++i)
      head = head->next_;

    coord_t coord = head->value_;
    if (!scene_config::instance()->can_move(boss_cfg->scene_cid_,
                                            coord.x_,
                                            coord.y_))
    {
      e_log->rinfo("refresh boss[%d] in scene[%d] failed![%d,%d can not move]",
                   boss_cfg->monster_cid_, boss_cfg->scene_cid_,
                   coord.x_, coord.y_);
      return -1;
    }

    int monster_id = spawn_monster::spawn_one(boss_cfg->monster_cid_,
                                              0,
                                              boss_cfg->scene_cid_,
                                              boss_cfg->scene_cid_,
                                              0,
                                              coord.x_,
                                              coord.y_);
    if (monster_id < 0)
    {
      e_log->rinfo("refresh boss[%d] in scene[%d] failed!",
                   boss_cfg->monster_cid_, boss_cfg->scene_cid_);
      return -1;
    }
    boss->monster_id_ = monster_id;
    s_log->rinfo("refresh boss[%d][%d] in scene[%d][%d,%d] ok!",
                 boss_cfg->monster_cid_, monster_id,
                 boss_cfg->scene_cid_, coord.x_, coord.y_);
    return 0;
  }

  int fetch_wild_boss_info(out_stream &os)
  {
    os << (char)this->wild_boss_map_.size();
    for (wild_boss_map_itor itor = this->wild_boss_map_.begin();
         itor != this->wild_boss_map_.end();
         ++itor)
    {
      if (itor->second->monster_id_ != 0)
        os << itor->second->monster_cid_ << 0;
      else
      {
        os << itor->second->monster_cid_
          << (itor->second->next_appear_time_ - time_util::now);
      }
    }
    return 0;
  }

private:
  int last_check_wild_boss_time_;
  wild_boss_map_t wild_boss_map_;
};
// --------------------------------------------------------
wild_boss_refresh::wild_boss_refresh() :
  impl_(new wild_boss_refresh_impl())
{ }
int wild_boss_refresh::do_timeout(const int now)
{ return this->impl_->do_timeout(now); }
int wild_boss_refresh::fetch_wild_boss_info(out_stream &os)
{ return this->impl_->fetch_wild_boss_info(os); }
