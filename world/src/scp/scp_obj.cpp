#include "scp_obj.h"
#include "sub_scp_obj.h"
#include "sys_log.h"
#include "scp_config.h"
#include "scp_module.h"
#include "time_util.h"
#include "scp_mgr.h"
#include "player_mgr.h"
#include "player_obj.h"
#include "buff_module.h"
#include "buff_obj.h"
#include "mblock_pool.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

scp_obj::scp_obj(const int scene_cid,
                 const int char_id,
                 const int team_id,
                 const int guild_id)
: scp_status_(S_SCP_INIT),
  char_id_(char_id),
  team_id_(team_id),
  guild_id_(guild_id),
  scene_cid_(scene_cid),
  open_time_(time_util::now)
{
  s_log->rinfo("new scp %d [%d,%d,%d]!",
               this->scene_cid_,
               this->char_id_,
               this->team_id_,
               this->guild_id_);
}
scp_obj::~scp_obj()
{
  if (this->char_id_ != 0)
    scp_mgr::instance()->unbind_scp_char_owner(this->char_id_);
  else if (this->team_id_ != 0)
    scp_mgr::instance()->unbind_scp_team_owner(this->team_id_);
  else if (this->guild_id_ != 0)
    scp_mgr::instance()->unbind_scp_guild_owner(this->guild_id_);

  while (!this->sub_scp_list_.empty())
  {
    sub_scp_obj *sub_scp = this->sub_scp_list_.pop_front();
    scp_mgr::instance()->remove_sub_scp(sub_scp->scene_id());
    delete sub_scp;
  }
  scp_mgr::instance()->remove_scp(this);
  s_log->rinfo("delete scp %d [%d,%d,%d]!",
               this->scene_cid_,
               this->char_id_,
               this->team_id_,
               this->guild_id_);
}
int scp_obj::open()
{
  if (this->init() != 0) return -1;

  // 先分配第一个副本（就是默认的主副本的CID）
  sub_scp_obj *sub_scp = this->construct_first_sub_scp(this->scene_cid_);
  if (sub_scp == NULL) return -1;
  if (sub_scp->open(this) != 0)
  {
    e_log->wning("scp %d open failed!", this->scene_cid_);
    delete sub_scp;
    return -1;
  }

  if (this->char_id_ != 0)
    scp_mgr::instance()->bind_scp_char_owner(this->char_id_, this);
  else if (this->team_id_ != 0)
    scp_mgr::instance()->bind_scp_team_owner(this->team_id_, this);
  else if (this->guild_id_ != 0)
    scp_mgr::instance()->bind_scp_guild_owner(this->guild_id_, this);

  scp_mgr::instance()->insert_scp(this);
  this->insert_new_sub_scp(sub_scp);
  s_log->rinfo("scp %d open ok!", this->scene_cid_);
  return 0;
}
int scp_obj::init()
{
  if (this->load_config() != 0)
  {
    e_log->error("scp %d load config failed!", this->scene_cid_);
    return -1;
  }
  this->open_time_ = time_util::now;
  return 0;
}
void scp_obj::insert_new_sub_scp(sub_scp_obj *sub_scp)
{
  scp_mgr::instance()->insert_sub_scp(sub_scp->scene_id(), sub_scp);
  this->sub_scp_list_.push_back(sub_scp);
}
void scp_obj::put_aev_to_all_char(const int aev, mblock *ev_mb)
{
  for (ilist_node<int> *itor = this->enter_char_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    player_obj *player = player_mgr::instance()->find(itor->value_);
    if (player != NULL)
    {
      mblock *mb = NULL;
      if (ev_mb != NULL)
      {
        mb = mblock_pool::instance()->alloc(ev_mb->length());
        mb->copy(ev_mb->rd_ptr(), ev_mb->length());
      }
      player->post_aev(aev, mb);
    }
  }
}
sub_scp_obj *scp_obj::construct_first_sub_scp(const int scene_cid)
{ return scp_module::construct_sub_scp(scene_cid); }
int scp_obj::first_scene_id()
{
  if (this->sub_scp_list_.empty()) return -1;
  return this->sub_scp_list_.head()->value_->scene_id();
}
int scp_obj::load_config()
{
  return 0;
}
int scp_obj::close()
{
  for (ilist_node<sub_scp_obj *> *itor = this->sub_scp_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    sub_scp_obj *sub_scp = itor->value_;
    sub_scp->close();
  }
  s_log->rinfo("scp %d close ok! %d", this->scene_cid_, this->sub_scp_list_.size());
  return 0;
}
int scp_obj::can_enter_scp(player_obj *)
{
  if (this->scp_status_ != S_SCP_RUNNING)
    return ERR_CAN_NOT_ENTER_SCP;
  return 0;
}
int scp_obj::enter_scp(const int char_id, const int scene_id)
{
  s_log->rinfo("enter scp %d to %d", char_id, scene_id);
  if (this->enter_char_list_.find(char_id))
  {
    e_log->wning("%d enter scp %d:%d failed!!",
                 char_id, this->scp_cid(), scene_id);
    return 0;
  }
  this->enter_char_list_.push_back(char_id);

  // 如果支持重复进入副本，要清除先前的离开记录
  int l_size = this->exit_char_list_.size();
  for (int i = 0; i < l_size; ++i)
  {
    pair_t<int> v = this->exit_char_list_.pop_front();
    if (v.first_ != char_id)
      this->exit_char_list_.push_back(v);
  }

  ilist_node<sub_scp_obj *> *itor = this->sub_scp_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    sub_scp_obj *sub_scp = itor->value_;
    if (sub_scp->scene_id() == scene_id)
    {
      sub_scp->enter_sub_scp(char_id);
      break;
    }
  }
  return 0;
}
int scp_obj::exit_scp(const int char_id, const int scene_id, const int reason)
{
  s_log->rinfo("exit scp %d in %d %d", char_id, scene_id, this->enter_char_list_.size());
  if (this->enter_char_list_.remove(char_id) != 0)
  {
    e_log->wning("char %d exit scp %d:%d failed!",
                 char_id, this->scp_cid(), scene_id);
    return 0;
  }

  bool b_find = false;
  for (ilist_node<pair_t<int> > *itor = this->exit_char_list_.head();
       itor != NULL && !b_find;
       itor = itor->next_)
  {
    if (itor->value_.first_ == char_id)
    {
      e_log->error("char %d exit scp %d repeat!", char_id, this->scp_cid());
      b_find = true;
    }
  }
  if (!b_find)
    this->exit_char_list_.push_back(pair_t<int>(char_id, reason));

  ilist_node<sub_scp_obj *> *itor = this->sub_scp_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    sub_scp_obj *sub_scp = itor->value_;
    if (sub_scp->scene_id() == scene_id)
    {
      sub_scp->exit_sub_scp(char_id);
      break;
    }
  }

  this->on_exit_scp(char_id, reason);

  this->do_check_nodoby();
  return 0;
}
bool scp_obj::do_check_nodoby()
{
  if (this->enter_char_list_.empty())
  {
    // 异步销毁副本，当玩家又进入新的副本时
    // 要检查玩家关联的旧副本是否存在scp_mgr::get_scp_by_char_owner
    this->scp_status_ = S_SCP_TO_CLOSE;
    return true;
  }
  return false;
}
void scp_obj::on_exit_scp(const int char_id, const int reason)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;

  buff_module::do_remove_buff_by_bits(player, buff_obj::BF_BIT_SCP_ACTIVITY);

  s_log->rinfo("char %d exit scp %d, reason = %s",
               char_id,
               this->scp_cid(),
               reason == CHAR_INITIATIVE_EXIT ? "act" : "passive");

}
int scp_obj::run(const int now)
{
  if (this->scp_status_ == S_SCP_INIT)
  {
    if (this->do_check_nodoby())
    {
      e_log->rinfo("scp %d nobody enter, so close! [%d,%d,%d]!",
                   this->scene_cid_,
                   this->char_id_,
                   this->team_id_,
                   this->guild_id_);
    }else
      this->scp_status_ = S_SCP_RUNNING;
  }else if (this->scp_status_ == S_SCP_RUNNING)
  {
    for (ilist_node<sub_scp_obj *> *itor = this->sub_scp_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      if (itor->value_->run(now) != 0)
      {
        this->scp_status_ = S_SCP_TO_CLOSE;
        break;
      }
    }
  }else if (this->scp_status_ == S_SCP_TO_CLOSE)
  {
    this->close();
    return -1;
  }
  return 0;
}
