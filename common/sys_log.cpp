#include "sys_log.h"

// Lib header
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int init_sys_log(const char *svc_name)
{
  if (::mkdir(LOG_CFG_DIR, 0755) == -1 && errno != EEXIST)
    return -1;

  //= init log
  char bf[128] = {0};
  ::snprintf(bf, sizeof(bf), "%s/sys_log.%s", LOG_CFG_DIR, svc_name);
  if (sys_log::instance()->init(bf) != 0)
  {
    fprintf(stderr, "Error: ilog - init sys log failed!\n");
    return -1;
  }
  //
  ::snprintf(bf, sizeof(bf), "%s/err_log.%s", LOG_CFG_DIR, svc_name);
  if (err_log::instance()->init(bf) != 0)
  {
    fprintf(stderr, "Error: ilog - init err log failed!\n");
    return -1;
  }
  //
  ::snprintf(bf, sizeof(bf), "%s/imp_log.%s", LOG_CFG_DIR, svc_name);
  if (imp_log::instance()->init(bf) != 0)
  {
    fprintf(stderr, "Error: ilog - init imp log failed!\n");
    return -1;
  }
  return 0;
}
