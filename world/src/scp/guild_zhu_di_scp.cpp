#include "guild_zhu_di_scp.h"
#include "global_param_cfg.h"
#include "scp_module.h"
#include "scp_config.h"
#include "player_mgr.h"
#include "player_obj.h"
#include "sys_log.h"
#include "scp_mgr.h"
#include "spawn_monster.h"
#include "scene_config.h"
#include "time_util.h"
#include "monster_obj.h"
#include "monster_mgr.h"
#include "mblock_pool.h"
#include "aev.h"

// Lib header
#include "mblock.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

guild_zhu_di_scp::guild_zhu_di_scp(const int scene_cid, const int guild_id) :
  scp_obj(scene_cid, 0, 0, guild_id)
{ }
guild_zhu_di_scp::~guild_zhu_di_scp()
{ }
sub_scp_obj *guild_zhu_di_scp::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new guild_zhu_di_sub_scp(new_scp_id, scene_cid);
}
void guild_zhu_di_scp::do_something(mblock *mb, int *, mblock *)
{
  if (mb->data_type() == SCP_EV_REMOVE_GUILD)
    this->scp_status_ = S_SCP_TO_CLOSE;
}
//= static method
void guild_zhu_di_scp::on_new_guild(const int guild_id)
{
  scp_obj *so = scp_module::construct_scp(global_param_cfg::guild_zhu_di_cid,
                                          0,
                                          0,
                                          guild_id);
  if (so == NULL || so->open() != 0)
  {
    e_log->rinfo("create guild %d zhudi scp failed!", guild_id);
    delete so;
    return ;
  }
}
void guild_zhu_di_scp::on_remove_guild(const int zhudi_scp_id)
{
  char bf[32] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_REMOVE_GUILD);
  scp_mgr::instance()->do_something(zhudi_scp_id, &mb, NULL, NULL);
}
void guild_zhu_di_scp::on_exit_guild(player_obj *player, const int )
{
  if (player->scene_cid() == global_param_cfg::guild_zhu_di_cid)
    player->post_aev(AEV_KICK_FROM_SCP, NULL);
}
int guild_zhu_di_scp::do_enter_zhu_di(player_obj *player)
{
  scp_obj *so = scp_mgr::instance()->get_scp_by_guild_owner(player->guild_id());
  if (so == NULL
      || so->first_scene_id() <= 0)
    return ERR_GUILD_ZHU_DI_NOT_OPEN;

  int ret = scp_module::can_enter_scp(player,
                                      global_param_cfg::guild_zhu_di_cid);
  if (ret != 0) return ret;

  ret = so->can_enter_scp(player);
  if (ret != 0) return ret;

  coord_t enter_pos = scp_config::instance()->get_enter_pos(global_param_cfg::guild_zhu_di_cid);
  if (enter_pos.x_ == -1) return ERR_CONFIG_NOT_EXIST;
  ret = player->do_transfer_to(global_param_cfg::guild_zhu_di_cid,
                               so->first_scene_id(),
                               enter_pos.x_,
                               enter_pos.y_);
  if (ret == 0)
    so->enter_scp(player->id(), so->first_scene_id());
  return ret;
}
int guild_zhu_di_scp::do_summon_tld_boss(player_obj *player, const int boss_cid)
{
  scp_obj *so = scp_mgr::instance()->get_scp_by_guild_owner(player->guild_id());
  if (so == NULL
      || so->first_scene_id() <= 0)
    return ERR_GUILD_ZHU_DI_NOT_OPEN;

  char bf[32] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(SCP_EV_GUILD_SUMMON_BOSS);
  mb << boss_cid << player->coord_x() << player->coord_y();
  scp_mgr::instance()->do_something(so->first_scene_id(), &mb, NULL, NULL);
  return 0;
}
// ------------------------------------ sub scp -----------------------------------
guild_zhu_di_sub_scp::guild_zhu_di_sub_scp(const int scene_id, const int scene_cid)
: sub_scp_obj(scene_id, scene_cid),
  last_create_tld_boss_time_(0),
  tld_boss_id_(0)
{ }
guild_zhu_di_sub_scp::~guild_zhu_di_sub_scp()
{ }
int guild_zhu_di_sub_scp::run(const int now)
{
  if (this->last_create_tld_boss_time_ != 0
      && time_util::diff_days(this->last_create_tld_boss_time_) == 1)
  {
    monster_obj *boss = monster_mgr::instance()->find(this->tld_boss_id_);
    if (boss != NULL)
      boss->to_destroy();
    this->last_create_tld_boss_time_ = 0;
    this->tld_boss_id_ = 0;
  }
  return super::run(now);
}
void guild_zhu_di_sub_scp::do_something(mblock *mb, int *, mblock *)
{
  if (mb->data_type() == SCP_EV_REMOVE_GUILD)
    this->on_remove_guild(mb);
  else if (mb->data_type() == SCP_EV_GUILD_SUMMON_BOSS)
    this->do_summon_tld_boss(mb);
  else
    e_log->rinfo("unknow scp ev %d when %s", mb->data_type(), __func__);
}
void guild_zhu_di_sub_scp::on_remove_guild(mblock *mb)
{
  this->master_scp_->do_something(mb, NULL, NULL);
}
void guild_zhu_di_sub_scp::do_summon_tld_boss(mblock *mb)
{
  int boss_cid = 0;
  short cx = 0;
  short cy = 0;
  *mb >> boss_cid >> cx >> cy;

  coord_t pos = scene_config::instance()->get_scene_random_pos(this->scene_cid_,
                                                               cx,
                                                               cy);
  this->tld_boss_id_ = spawn_monster::spawn_one(boss_cid,
                                                0,
                                                this->scene_id_,
                                                this->scene_cid_,
                                                DIR_XX,
                                                pos.x_,
                                                pos.y_);
}
