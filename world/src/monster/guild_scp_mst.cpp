#include "guild_scp_mst.h"
#include "scp_mgr.h"
#include "scp_module.h"
#include "scene_mgr.h"
#include "guild_config.h"
#include "def.h"
#include "sys_log.h"

// Lib header

// Defines

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

#define MST_DEAY_DESTORY_TIEM 100

guild_scp_had_hate_mst::guild_scp_had_hate_mst()
{
  coord_t &target_pos = guild_scp_config::instance()->target_pos();
  this->target_x_ = target_pos.x_;
  this->target_y_ = target_pos.y_;
  this->range_ = 0;
  this->reach_time_ = time_value::zero;
}
void guild_scp_had_hate_mst::do_patrol(const time_value &now)
{
  if (this->can_patrol(now) != 0) return ;
  if (this->if_reach_target())
  {
    this->on_reach_target(now);
    return ;
  }else
  {
    if (this->path_.empty())
    {
      this->do_find_path(coord_t(this->coord_x_, this->coord_y_),
                         coord_t(this->target_x_, this->target_y_),
                         true);
      if (this->path_.empty())
      {
        this->to_destroy();
        return ;
      }
    }
    this->do_move_i(now);
  }

  if (this->att_type_ == ACTIVE_ATT
      // optimize: 场景内没有人，就不用执行AI了
      && scene_mgr::instance()->unit_cnt(this->scene_id_, scene_unit::PLAYER) > 0)
    this->do_search_in_view_area(now);
}
void guild_scp_had_hate_mst::to_back(const time_value &)
{
  this->do_status_  = DO_PATROLING;
  this->att_obj_id_ = 0;
  this->do_clear_hate();

  this->path_.clear();
}
int guild_scp_had_hate_mst::do_exit_scene()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_GUILD_SCP_MST_EXIT);
  scp_mgr::instance()->do_something(this->scene_id(), &mb, NULL, NULL);
  return super::do_exit_scene();
}
bool guild_scp_had_hate_mst::if_reach_target()
{
  return util::is_inside_of_redius(this->coord_x_,
                                   this->coord_y_,
                                   this->target_x_,
                                   this->target_y_,
                                   this->range_);
}
void guild_scp_had_hate_mst::on_reach_target(const time_value &now)
{
  this->reach_time_ = now;
}
int guild_scp_had_hate_mst::do_action_in_scene(const time_value &now)
{
  if (this->reach_time_ == time_value::zero)
  {
    super::do_action_in_scene(now);
  }else if ((now - this->reach_time_).msec() > MST_DEAY_DESTORY_TIEM)
  {
    this->to_destroy();
    mblock mb(client::send_buf, client::send_buf_len);
    mb.data_type(SCP_EV_GUILD_SCP_MST_REACH);
    mb << this->sort_;
    scp_mgr::instance()->do_something(this->scene_id(), &mb, NULL, NULL);
  }
  return 0;
}
