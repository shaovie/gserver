#include "global_timer.h"
#include "sys_log.h"
#include "time_value.h"
#include "time_util.h"
#include "world_svc_mgr.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("global_timer");
static ilog_obj *e_log = err_log::instance()->get_ilog("global_timer");

int global_timer::handle_timeout(const time_value &now)
{
  int sec = (int)now.sec();
  time_util::now = sec;

  world_svc_mgr::instance()->dump(sec);
  return 0;
}
