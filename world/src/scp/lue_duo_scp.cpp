#include "lue_duo_scp.h"
#include "player_obj.h"
#include "lue_duo_module.h"
#include "scp_module.h"
#include "scp_mgr.h"
#include "scp_config.h"
#include "monster_obj.h"
#include "monster_mgr.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

lue_duo_scp::lue_duo_scp(const int scene_cid, const int char_id) :
  scp_obj(scene_cid, char_id, 0, 0)
{ }
lue_duo_scp::~lue_duo_scp()
{ }
sub_scp_obj *lue_duo_scp::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new lue_duo_sub_scp(new_scp_id, scene_cid);
}
void lue_duo_scp::on_char_dead(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_CHAR_DEAD);
  mb << player->id();
  scp_mgr::instance()->do_something(player->scene_id(), &mb, NULL, NULL);
}
// ------------------------------------ sub scp -----------------------------------
lue_duo_sub_scp::lue_duo_sub_scp(const int scene_id, const int scene_cid)
: sub_scp_obj(scene_id, scene_cid),
  end_(false),
  has_fanpai_(false),
  monster_id_(0)
{ }
lue_duo_sub_scp::~lue_duo_sub_scp()
{ }
int lue_duo_sub_scp::open(scp_obj *master_scp)
{
  this->last_time_ = scp_config::instance()->last_time(master_scp->scp_cid(),
                                                       this->scene_cid_);
  return super::open(master_scp);
}
int lue_duo_sub_scp::run(const int now)
{
  if (this->end_) return 0;

  if (now - this->open_time_ > this->last_time_)
    this->lue_duo_end(this->master_scp_->owner_id());

  return super::run(now);
}
void lue_duo_sub_scp::do_something(mblock *mb, int *res, mblock *)
{
  if (mb->data_type() == SCP_EV_JING_JI_MST)
    *mb >> this->monster_id_;
  else if (mb->data_type() == SCP_EV_CHAR_DEAD)
  {
    // 保证竞技场中只有对手怪物和玩家可发SCP_EV_CHAR_DEAD事件
    int id = *((int *)mb->rd_ptr());
    this->lue_duo_end(id);
  }else if (mb->data_type() == SCP_EV_LUEDUO_FANPAI)
  {
    if (this->has_fanpai_)
      *res = ERR_CLIENT_OPERATE_ILLEGAL;
    else
    {
      this->has_fanpai_ = true;
      *res = 0;
    }
  }
  else
    e_log->rinfo("unknow scp ev %d when %s", mb->data_type(), __func__);
}
void lue_duo_sub_scp::lue_duo_end(const int loser_id)
{
  this->end_ = true;

  bool win = false;
  if (loser_id != this->master_scp_->owner_id())
    win = true;
  else // lose
  {
    monster_obj *mo = monster_mgr::instance()->find(this->monster_id_);
    if (mo != NULL)
      mo->post_aev(AEV_JING_JI_END, NULL);
  }

  lue_duo_module::on_lue_duo_end(this->master_scp_->owner_id(), win);
}
void lue_duo_sub_scp::exit_sub_scp(const int )
{
  if (!this->end_)
    lue_duo_module::on_lue_duo_end(this->master_scp_->owner_id(), false);
}
