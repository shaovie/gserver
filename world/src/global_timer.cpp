#include "global_timer.h"
#include "behavior_log.h"
#include "player_mgr.h"
#include "monster_mgr.h"
#include "dropped_item_mgr.h"
#include "time_util.h"
#include "istream.h"
#include "sys.h"
#include "aev.h"
#include "message.h"
#include "sys_log.h"
#include "client.h"
#include "chat_module.h"
#include "sev_dispatch.h"
#include "svc_monitor.h"
#include "mail_module.h"
#include "wild_boss_refresh.h"
#include "timing_mst_module.h"
#include "guild_module.h"
#include "market_module.h"
#include "effect_obj_mgr.h"
#include "scp_mgr.h"
#include "activity_mgr.h"
#include "ltime_act_mgr.h"
#include "rank_module.h"
#include "gm_notice_module.h"
#include "jing_ji_module.h"
#include "service_info.h"

// Lib header
#include <stdlib.h>
#include "date_time.h"
#include "time_value.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("global_timer");
static ilog_obj *e_log = err_log::instance()->get_ilog("global_timer");

static bool s_is_close_new_day = false;
static int s_last_dump_online_time = 0;

global_timer::global_timer() { }
int global_timer::handle_timeout(const time_value &now)
{
  int sec = (int)now.sec();
  time_util::now = sec;

  if (sec % 60 == 0
      && s_last_dump_online_time > 0)
  {
    s_last_dump_online_time = 0;
    behavior_log::instance()->store(BL_ONLINE,
                                    time_util::now,
                                    "%d",
                                    player_mgr::instance()->size());
  }else
    s_last_dump_online_time = sec;

  this->check_svc_open(sec);

  behavior_log::instance()->flush(sec);

  dropped_item_mgr::instance()->do_timeout();
  effect_obj_mgr::instance()->do_timeout(now);

  chat_module::do_timeout(now);
  svc_monitor::instance()->run(sec);

  scp_mgr::instance()->run(sec);
  activity_mgr::instance()->run(sec);
  ltime_act_mgr::instance()->run(sec);

  if (this->is_new_day(sec))
    this->do_something_on_new_day();

  wild_boss_refresh::instance()->do_timeout(sec);
  timing_mst_module::do_timeout(sec);

  market_module::do_timeout(time_util::now);

  gm_notice_module::do_timeout(time_util::now);

  jing_ji_module::do_timeout(time_util::now);

  if (sys::friendly_exit)
  {
    s_log->rinfo("friendly exit!");
    e_log->rinfo("friendly exit!");
    sev_notify::instance()->notify(SEV_SHUTDOWN, NULL);
    behavior_log::instance()->flush(sec + 10000);
    ::exit(0);
  }
#ifndef PUBLISH
  s_log->rinfo("+++++++++++++++++++++++++++++++++++++");
#endif
  return 0;
}
void global_timer::check_svc_open(const int sec)
{
  if (sys::svc_info->opened) return ;

  if (sec > sys::svc_info->open_time)
    this->do_svc_open();
  return ;
}
void global_timer::do_svc_open()
{
  s_log->rinfo("open service !!!!!!!!!!!!!!!!!!");
  sys::svc_info->opened = 1;
  sys::update_svc_info();
}
bool global_timer::is_new_day(const int now)
{
  date_time dt(now);
  if (dt.hour() == 23)
  {
    s_is_close_new_day = true;
  }else if (s_is_close_new_day && dt.hour() == 0 && dt.min() == 0)
  {
    s_is_close_new_day = false;
    return true;
  }
  return false;
}
void global_timer::do_something_on_new_day()
{
  player_mgr::instance()->post_aev_to_world(AEV_ON_NEW_DAY, NULL);
  guild_module::on_new_day();
  mail_module::on_new_day();
}
