#include "sys.h"
#include "sys_log.h"
#include "time_util.h"
#include "svc_config.h"
#include "global_timer.h"
#include "world_svc.h"
#include "unused_arg.h"

// Lib header
#include "reactor.h"
#include "acceptor.h"
#include "time_value.h"

reactor *sys::r = NULL;

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

extern const char *g_svc_name;

int sys::init_svc()
{
  time_util::init();
  srand(time_value::start_time.sec() + time_value::start_time.usec());

  // check sys config
  if (init_sys_log(g_svc_name) != 0)
    return -1;

  s_log->rinfo("                          init %s", g_svc_name);

  if (svc_config::instance()->load_config(".") != 0)
  {
    e_log->rinfo("load svc config failed!");
    return -1;
  }

  if (sys::load_config() != 0
      || sys::r->schedule_timer(global_timer::instance(),
                                time_value(0, 600*1000),
                                time_value(0, 600*1000)) == -1
     )
    return -1;

  // MUST 上面的逻辑可能会很耗时，导致time_util中缓存的时间不准
  time_util::now = time_value::gettimeofday().sec();
  return sys::launch_net_module();
}
int sys::launch_net_module()
{
  static acceptor<world_svc> gc_acceptor;
  inet_address local_addr(svc_config::instance()->cross_port(), "0.0.0.0");
  if (gc_acceptor.open(local_addr, sys::r) != 0)
  {
    e_log->error("game client acceptor open failed! port: %d",
                 svc_config::instance()->cross_port());
    return -1;
  }

  s_log->rinfo("launch net module ok!");
  return 0;
}
int sys::load_config()
{
#define CFG_LOAD_CODE(OBJ_NAME) \
  if (OBJ_NAME::instance()->load_config(svc_config::instance()->cfg_root()) != 0) \
  { \
    e_log->error("load " #OBJ_NAME " failed!"); \
    return -1; \
  }

  s_log->rinfo("load all game config ok!");
  return 0;
}
int sys::reload_config(const char *msg,
                       const int /*len*/,
                       char *result,
                       const int res_in_len,
                       int &result_len)
{
  result_len += ::snprintf(result + result_len,
                           res_in_len - result_len,
                           "\"%s\":{", g_svc_name);

  char *tok_p = NULL;
  char *token = NULL;
  char bf[256] = {0};
  ::strncpy(bf, msg, sizeof(bf) - 1);

  int reload_cnt = 0;
  for (token = ::strtok_r(bf, ",", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, ",", &tok_p))
  {
    if (sys::dispatch_config(token, result, res_in_len, result_len) != -2)
      ++reload_cnt;
  }
  if (reload_cnt > 0)
    result_len -= 2;
  result_len += ::snprintf(result + result_len, res_in_len - result_len, "}");

  return 0;
}
int sys::dispatch_config(const char *name,
                         char *result,
                         const int res_in_len,
                         int &result_len)
{
  int ret = -2;
  bool all = false;
  if (::strncmp(name, "all", 3) == 0) all = true;

#define CFG_RELOAD_CODE(CFG_NAME, OBJ_NAME) \
  const char * OBJ_NAME##_base_name = util::istrchr(CFG_NAME, '/'); \
  if (OBJ_NAME##_base_name == NULL) \
  { OBJ_NAME##_base_name = CFG_NAME; } \
  else \
  { OBJ_NAME##_base_name = OBJ_NAME##_base_name + 1; } \
  if (all || ::strcmp(OBJ_NAME##_base_name, name) == 0) \
  { \
    ret = OBJ_NAME::instance()->reload_config(svc_config::instance()->cfg_root()); \
    result_len += ::snprintf(result + result_len, \
                             res_in_len - result_len, \
                             "\"%s\": %d, ", CFG_NAME, ret); \
  }

  unused_args(result, result_len, res_in_len);
  return ret;
}
