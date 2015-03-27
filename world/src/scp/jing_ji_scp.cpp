#include "jing_ji_scp.h"
#include "player_obj.h"
#include "jing_ji_module.h"
#include "scp_module.h"
#include "scp_mgr.h"
#include "scp_config.h"
#include "monster_obj.h"
#include "monster_mgr.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

jing_ji_scp::jing_ji_scp(const int scene_cid, const int char_id) :
  scp_obj(scene_cid, char_id, 0, 0)
{ }
jing_ji_scp::~jing_ji_scp()
{ }
sub_scp_obj *jing_ji_scp::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new jing_ji_sub_scp(new_scp_id, scene_cid);
}
// ------------------------------------ sub scp -----------------------------------
jing_ji_sub_scp::jing_ji_sub_scp(const int scene_id, const int scene_cid)
: sub_scp_obj(scene_id, scene_cid),
  end_(false),
  monster_id_(0)
{ }
jing_ji_sub_scp::~jing_ji_sub_scp()
{ }
int jing_ji_sub_scp::open(scp_obj *master_scp)
{
  this->last_time_ = scp_config::instance()->last_time(master_scp->scp_cid(),
                                                       this->scene_cid_);
  return super::open(master_scp);
}
int jing_ji_sub_scp::run(const int now)
{
  if (this->end_) return 0;

  if (now - this->open_time_ > this->last_time_)
    this->jing_ji_end(this->master_scp_->owner_id());

  return super::run(now);
}
void jing_ji_sub_scp::do_something(mblock *mb, int *, mblock *)
{
  if (mb->data_type() == SCP_EV_JING_JI_MST)
    *mb >> this->monster_id_;
  else if (mb->data_type() == SCP_EV_CHAR_DEAD)
  {
    // 保证竞技场中只有对手怪物和玩家可发SCP_EV_CHAR_DEAD事件
    int id = *((int *)mb->rd_ptr());
    this->jing_ji_end(id);
  }else
    e_log->rinfo("unknow scp ev %d when %s", mb->data_type(), __func__);
}
void jing_ji_sub_scp::jing_ji_end(const int loser_id)
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

  jing_ji_module::on_jing_ji_end(this->master_scp_->owner_id(), win);
}
