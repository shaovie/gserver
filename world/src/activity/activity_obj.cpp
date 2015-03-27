#include "activity_obj.h"
#include "activity_cfg.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "activity_mgr.h"
#include "time_util.h"
#include "sys_log.h"
#include "error.h"
#include "def.h"

// Lib header
#include "date_time.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("act");
static ilog_obj *e_log = err_log::instance()->get_ilog("act");

activity_obj::activity_obj(const int act_id) :
  status_(activity_obj::ST_CLOSED),
  act_id_(act_id),
  begin_time_(0),
  left_time_(0)
{ }
activity_obj::~activity_obj()
{ }
bool activity_obj::do_check_if_opened(const int now)
{
  activity_cfg_obj *aco = activity_cfg::instance()->get_act_cfg_obj(this->act_id_);
  if (aco == NULL || !aco->is_valid_) return false;

  date_time now_dt((time_t)now);

  int wday = time_util::wday(now_dt);

  bool is_closed = true;
  // week
  for (ilist_node<int> *witor = aco->week_list_.head();
       witor != NULL && is_closed;
       witor = witor->next_)
  {
    if (witor->value_ == wday)
      is_closed = false;
  }
  if (is_closed) return false;

  itime it(now_dt.hour(), now_dt.min(), now_dt.sec());
  // time
  is_closed = true;
  for (ilist_node<pair_t<itime> > *itor = aco->time_list_.head();
       itor != NULL && is_closed;
       itor = itor->next_)
  {
    if (it > itor->value_.first_
        && it < itor->value_.second_)
    {
      is_closed = false;
      this->left_time_ = itor->value_.second_ - it;
    }
  }
  if (is_closed)
    this->left_time_ = 0;
  return !is_closed;
}
void activity_obj::run(const int now)
{
  if (this->status_ == activity_obj::ST_CLOSED)
  {
    if (this->do_check_if_opened(now))
    {
      if (this->open() == 0)
      {
        this->on_activity_opened();
        this->status_ = activity_obj::ST_OPENED;
        this->begin_time_ = now;
      }else
        e_log->rinfo("act %d open failed!", this->act_id_);
    }
  }else if (this->status_ == activity_obj::ST_OPENED)
  {
    if (this->do_ai(now) != 0
        || !this->do_check_if_opened(now))
    {
      this->close();
      this->on_activity_closed();
      this->status_ = activity_obj::ST_CLOSED;
    }
  }
}
void activity_obj::on_activity_opened()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->act_id_ << this->left_time();
  player_mgr::instance()->broadcast_to_world(NTF_ACTIVITY_OPENED, &mb);
}
void activity_obj::on_activity_closed()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->act_id_;
  player_mgr::instance()->broadcast_to_world(NTF_ACTIVITY_CLOSED, &mb);
}
