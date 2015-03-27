#include "player_obj.h"
#include "sys_log.h"
#include "sev_dispatch.h"
#include "global_param_cfg.h"
#include "sev.h"
#include "scp_module.h"
#include "scp_obj.h"

// Lib header
#include "mblock.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

void player_obj::do_attach_sev()
{
  sev_notify::instance()->attach(SEV_SHUTDOWN, this);
}
void player_obj::do_deattach_sev()
{
  sev_notify::instance()->deattach(SEV_SHUTDOWN, this);
}
void player_obj::dispatch_sev(const int sev_id, mblock *mb)
{
#define SEV_SHORT_CODE(ID, FUNC) case ID:  \
  this->FUNC(mb);                          \
  break
#define SEV_SHORT_DEFAULT default:         \
  e_log->error("unknow sev id %d", sev_id);\
  break

  switch (sev_id)
  {
    SEV_SHORT_CODE(SEV_SHUTDOWN, sev_shutdown);
    SEV_SHORT_DEFAULT;
  }
}
void player_obj::sev_shutdown(mblock *)
{
  this->shutdown();
}
