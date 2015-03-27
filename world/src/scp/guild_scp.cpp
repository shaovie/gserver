#include "guild_scp.h"
#include "sys_log.h"
#include "scp_obj.h"
#include "scp_module.h"
#include "monster_cfg.h"
#include "monster_mgr.h"
#include "time_util.h"
#include "client.h"
#include "mblock.h"
#include "istream.h"
#include "player_mgr.h"
#include "message.h"
#include "player_obj.h"
#include "guild_config.h"
#include "guild_module.h"
#include "spawn_monster.h"
#include "scene_config.h"
#include "scp_mgr.h"
#include "guild_module.h"

// Lib header

#define GUILD_SCP_INIT_HP       100
#define GUILD_WAIT_KICK_TIME    10
#define GUILD_WAIT_CLOSE_TIME   5
#define GUILD_WAIT_REFRESH_TIME 5
#define GUILD_NO_BODY_TIME      600

enum guild_scp_status
{
  GSS_WAIT_PLAYER_ENTER = 1,
  GSS_WAIT_REFRESH_MST  = 2,
  GSS_ON_REFRESH_MST    = 3,
  GSS_WAIT_KILL_ALL_MST = 4,
  GSS_WAIT_KICK_CHAR    = 5,
  GSS_WAIT_CLOSE        = 6,
};
enum guild_scp_result
{
  GSR_FAIL = 0,
  GSR_WIN  = 1,
};
enum guild_mst_cost_hp
{
  GMCH_MST  = 1,
  GMCH_BOSS = 5,
};

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

guild_scp::guild_scp(const int scene_cid, const int guild_id) :
  scp_obj(scene_cid, 0, 0, guild_id)
{ }
sub_scp_obj *guild_scp::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new guild_sub_scp(new_scp_id, scene_cid);
}
// ===================================== sub_scp ====================================================
guild_sub_scp::guild_sub_scp(const int scene_id, const int scene_cid) :
  sub_scp_obj(scene_id, scene_cid),
  difficulty_(0),
  bo_shu_(0),
  batch_(0),
  hp_(0),
  scp_lvl_(0),
  scp_status_(0),
  add_resource_(0),
  switch_status_time_(0),
  mst_amt_(0)
{ }
int guild_sub_scp::run(const int now)
{
  switch (this->scp_status_)
  {
  case GSS_WAIT_PLAYER_ENTER:
    {
      if (now - this->switch_status_time_ > GUILD_NO_BODY_TIME)
        return -1;
      break;
    }
  case GSS_WAIT_REFRESH_MST:
    {
      if (now - this->switch_status_time_ > GUILD_WAIT_REFRESH_TIME)
        this->switch_to_status(GSS_ON_REFRESH_MST);
      break;
    }
  case GSS_ON_REFRESH_MST:
    {
      const int bo_shu_time =
        guild_scp_config::instance()->over_all_time(this->difficulty_, this->bo_shu_);
      if (bo_shu_time == MAX_INVALID_INT)
      {
        this->switch_to_status(GSS_WAIT_KILL_ALL_MST);
        return 0;
      }
      if (now > this->switch_status_time_ + bo_shu_time)
      {
        this->do_award_yi_bo_over(this->bo_shu_);
        ++this->bo_shu_;
        this->batch_ = 1;
        this->do_spawn_monster(this->bo_shu_, this->batch_);
        this->do_bro_scp_info();
      }
      const int batch_time =
        guild_scp_config::instance()->over_all_time(this->difficulty_, this->bo_shu_, this->batch_);
      if (now > this->switch_status_time_ + batch_time)
      {
        ++this->batch_;
        this->do_spawn_monster(this->bo_shu_, this->batch_);
      }
      break;
    }
  case GSS_WAIT_KILL_ALL_MST:
    {
      if (this->mst_amt_ <= 0)
        this->do_over_scp(GSR_WIN);
      break;
    }
  case GSS_WAIT_KICK_CHAR:
    {
      if (now < this->switch_status_time_ + GUILD_WAIT_KICK_TIME)
        return 0;
      this->switch_to_status(GSS_WAIT_CLOSE);
      player_mgr::instance()->post_aev_to_scene(this->scene_id_, AEV_KICK_FROM_SCP, NULL);
      break;
    }
  case GSS_WAIT_CLOSE:
    {
      if (now < this->switch_status_time_ + GUILD_WAIT_CLOSE_TIME)
        return -1;
      break;
    }
  }
  return 0;
}
void guild_sub_scp::do_something(mblock *mb, int *res, mblock *)
{
  switch (mb->data_type())
  {
  case SCP_EV_GUILD_INIT_SCP:
    this->do_init_scp(mb);
    break;
  case SCP_EV_GUILD_SCP_MST_REACH:
    this->on_scp_reach_finish(mb);
    break;
  case SCP_EV_GUILD_SCP_CAN_ENTER:
    this->if_scp_can_enter(res);
    break;
  case SCP_EV_GUILD_SCP_MST_EXIT:
    this->on_scp_mst_exit();
    break;
  }
}
void guild_sub_scp::do_init_scp(mblock *mb)
{
  *mb >> this->difficulty_ >> this->scp_lvl_;
  this->hp_ = GUILD_SCP_INIT_HP;
  this->switch_to_status(GSS_WAIT_PLAYER_ENTER);
}
void guild_sub_scp::on_scp_reach_finish(mblock *mb)
{
  char sort = 0;
  *mb >> sort;
  const int cost_hp = sort == MST_SORT_BOSS ? GMCH_BOSS : GMCH_MST;
  this->hp_ -= cost_hp;
  if (this->hp_ <= 0)
    this->do_over_scp(GSR_FAIL);
  this->do_bro_scp_info();
}
void guild_sub_scp::if_scp_can_enter(int *res)
{
  if (res == NULL) return ;
  if (this->scp_status_ == GSS_WAIT_KICK_CHAR
      || this->scp_status_ == GSS_WAIT_CLOSE)
    *res = ERR_CAN_NOT_ENTER_SCP;
}
void guild_sub_scp::on_scp_mst_exit()
{ --this->mst_amt_; }
void guild_sub_scp::switch_to_status(const char status)
{
  this->scp_status_ = status;
  this->switch_status_time_ = time_util::now;
}
void guild_sub_scp::do_over_scp(const char result)
{
  this->switch_to_status(GSS_WAIT_KICK_CHAR);

  monster_mgr::instance()->destroy_scene_monster(this->scene_id_);

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << result;
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(this->scene_id_, NTF_GUILD_SCP_OVER, &mb);

  guild_module::on_close_scp(this->master_scp_->owner_id());
}
void guild_sub_scp::do_bro_scp_info()
{
  for (ilist_node<int> *itor = this->master_scp_->enter_char_list()->head();
       itor != NULL;
       itor = itor->next_)
    this->do_notify_scp_info(itor->value_);
}
void guild_sub_scp::do_notify_scp_info(const int char_id)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->hp_
    << this->bo_shu_
    << this->add_resource_
    << this->get_player_contirb(char_id);
  player->do_delivery(NTF_GUILD_SCP_INFO, &mb);
}
int guild_sub_scp::get_player_contirb(const int char_id)
{
  contrib_map_itor itor = this->contrib_map_.find(char_id);
  return itor == this->contrib_map_.end() ? 0 : itor->second;
}
void guild_sub_scp::enter_sub_scp(const int char_id)
{
  sub_scp_obj::enter_sub_scp(char_id);
  if (this->scp_status_ == GSS_WAIT_PLAYER_ENTER)
  {
    this->switch_to_status(GSS_WAIT_REFRESH_MST);
    this->bo_shu_ = 1;
  }
  this->do_notify_scp_info(char_id);
}
void guild_sub_scp::do_award_yi_bo_over(const char bo_shu)
{
  const int add_resource = guild_scp_config::instance()->award_resource(this->difficulty_, bo_shu);
  const int real_resource = add_resource * guild_lvl_config::instance()->scp_award_per(this->scp_lvl_) / 1000;
  guild_module::do_got_resource(this->master_scp_->owner_id(), real_resource);
  this->add_resource_ += real_resource;

  const int add_contrib = guild_scp_config::instance()->award_contrib(this->difficulty_, bo_shu);
  const int real_contrib = add_contrib * guild_lvl_config::instance()->scp_award_per(this->scp_lvl_) / 1000;
  for (ilist_node<int> *itor = this->master_scp_->enter_char_list()->head();
       itor != NULL;
       itor = itor->next_)
  {
    guild_module::do_got_contrib(this->master_scp_->owner_id(), itor->value_, real_contrib);
    this->do_notify_scp_info(itor->value_);
    contrib_map_itor cm_itor = this->contrib_map_.find(itor->value_);
    if (cm_itor == this->contrib_map_.end())
      this->contrib_map_.insert(std::make_pair(itor->value_, real_contrib));
    else
      cm_itor->second += real_contrib;
  }
}
void guild_sub_scp::do_spawn_monster(const char bo_shu, const char batch)
{
  const int refresh_mst_cid = guild_scp_config::instance()->refresh_mst_cid(this->difficulty_, bo_shu, batch);
  if (refresh_mst_cid == 0) return ;
  const int refresh_mst_amt = guild_scp_config::instance()->refresh_mst_amt(this->difficulty_, bo_shu, batch);

  ilist<coord_t> *refresh_pos = guild_scp_config::instance()->mst_refresh_pos(this->difficulty_, bo_shu);
  if (refresh_pos == NULL) return ;

  coord_t &target_pos = guild_scp_config::instance()->target_pos();

  for (ilist_node<coord_t> *itor = refresh_pos->head();
       itor != NULL;
       itor = itor->next_)
  {
    int delay = 0;
    for (int i = 0; i < refresh_mst_amt; ++i)
    {
      int birth_dir = util::calc_next_dir(itor->value_.x_,
                                          itor->value_.y_,
                                          target_pos.x_,
                                          target_pos.y_);
      coord_t birth_pos = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                                   itor->value_.x_,
                                                                   itor->value_.y_,
                                                                   2);
      const int mst_id = spawn_monster::spawn_one(refresh_mst_cid,
                                                  delay,
                                                  this->scene_id_,
                                                  this->scene_cid_,
                                                  birth_dir,
                                                  birth_pos.x_,
                                                  birth_pos.y_);
      if (mst_id != -1)
        ++this->mst_amt_;

      delay += 200 + (rand() % 3 + 1) * 200;
    }
  }
}
