#include "sub_scp_obj.h"
#include "sys_log.h"
#include "scene_config.h"
#include "scene_mgr.h"
#include "monster_mgr.h"
#include "scp_mgr.h"
#include "time_util.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("sub_scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("sub_scp");

sub_scp_obj::sub_scp_obj(const int scene_id, const int scene_cid)
: scene_id_(scene_id),
  scene_cid_(scene_cid),
  open_time_(0),
  master_scp_(NULL)
{
  short x_len = scene_config::instance()->x_len(this->scene_cid_);
  short y_len = scene_config::instance()->y_len(this->scene_cid_);
  scene_mgr::instance()->insert(this->scene_cid_,
                                this->scene_id_,
                                x_len,
                                y_len);
}
sub_scp_obj::~sub_scp_obj()
{
  scp_mgr::instance()->release_scp_id(this->scene_id_);
  scene_mgr::instance()->remove(this->scene_id_);
}
int sub_scp_obj::open(scp_obj *master_scp)
{
  this->open_time_  = time_util::now;
  this->master_scp_ = master_scp;

  s_log->rinfo("sub scp %d:%d open ok!", this->scene_cid_, this->scene_id_);
  return 0;
}
int sub_scp_obj::close()
{
  s_log->rinfo("sub scp %d:%d closing ...", this->scene_cid_, this->scene_id_);
  this->do_clear();
  return 0;
}
int sub_scp_obj::run(const int )
{ return 0; }
void sub_scp_obj::do_clear()
{
  monster_mgr::instance()->destroy_scene_monster(this->scene_id_);
  return ;
}
