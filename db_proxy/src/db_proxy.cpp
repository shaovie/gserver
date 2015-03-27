// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-08 14:55
 */
//========================================================================

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "ilog.h"
#include "reactor.h"
#include "acceptor.h"

#include "sys_log.h"
#include "daemon.h"
#include "proxy_obj.h"
#include "svc_config.h"
#include "global_macros.h"
#include "db_async_store.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");
const char *g_svc_name = "db_proxy";

void print_usage()
{
  printf("\033[0;32m\nUsage: %s [OPTION...]\n\n", g_svc_name);
  printf("  -g                    guard mode\n");
  printf("  -u  user name         run as user\n");
  printf("  -n  number of proxy   \n");
  printf("  -C                    core dump enable\n");
  printf("  -p  platform name     \n"); 
  printf("  -s  service sn        \n");
  printf("  -v                    Output version info\n");
  printf("  -h                    Help info\n");
  printf("\n");
  printf("\033[0m\n");
  ::exit(0);
}
void svc_exit_signal(int )
{
  ::exit(0);
}
int init_svc(reactor *r, const int number_of_proxy)
{
  srand(time_value::start_time.sec() + time_value::start_time.usec());

  if (init_sys_log(g_svc_name) != 0)
    return -1;

  s_log->rinfo("                          init %s", g_svc_name);

  if (svc_config::instance()->load_config(".") != 0)
  {
    e_log->rinfo("load svc config failed!");
    return -1;
  }

  if (db_async_store::instance()->open(r,
                                       number_of_proxy,
                                       svc_config::instance()->db_port(),
                                       svc_config::instance()->db_host(),
                                       svc_config::instance()->db_user(),
                                       svc_config::instance()->db_passwd(),
                                       svc_config::instance()->db_name()) != 0)
  {
    e_log->rinfo("db_async_store open failed!");
    return -1;
  }

  static acceptor<proxy_obj> proxy_acceptor;
  inet_address local_addr(svc_config::instance()->db_proxy_port(),
                          svc_config::instance()->db_proxy_host());
  if (proxy_acceptor.open(local_addr, r, MAX_INNER_SOCKET_BUF_SIZE) != 0)
  {
    e_log->rinfo("proxy acceptor open failed! %s:%d[%s]",
                 svc_config::instance()->db_proxy_host(),
                 svc_config::instance()->db_proxy_port(),
                 strerror(errno));
    return -1;
  }
  return 0;
}
int main(int argc, char *argv[])
{ 
  int in_guard_mode = 0;
  int dump_core = 0;
  int children_instance = 0;
  int number_of_proxy = 0;
  char *run_as_user = NULL;

  // parse args
  int c = -1;
  extern char *optarg;
  extern int optopt;

  const char *opt = ":gvu:n:Cep:s:h";
  while((c = getopt(argc, argv, opt)) != -1)
  {
    switch (c)
    {
    case 'v':        // version
      fprintf(stderr, "\033[0;32m%s %s\033[0m\n", SVC_EDITION, BIN_V);
      return 0;
    case 'n':
      number_of_proxy = ::atoi(optarg);
      if (number_of_proxy <= 0)
      {
        fprintf(stderr, "\033[0;35mnumber of proxy is invalid! 1~32\033[0m\n");
        return 0;
      }
      break;
    case 'g':
      in_guard_mode = 1;
      break;
    case 'u':
      run_as_user = optarg;
      break;
    case 'C':
      dump_core = 1;
      break;
    case 'p':
    case 's':
      break;
    case 'e':
      children_instance = 1;
      break;
    case ':':
      fprintf(stderr, 
              "\n\033[0;35mOption -%c requires an operand\033[0m\n",
              optopt);
    case 'h':
    default:
      print_usage();
    }
  }

  set_max_fds(1024);

  if (run_as_user && runas(run_as_user) != 0)
  {
    fprintf(stderr, "\033[0;35mSudo to %s error!\033[0m\n", 
            run_as_user);
    return -1;
  }

  if (dump_core && dump_corefile() != 0)
  {
    fprintf(stderr, "\033[0;35mSet dump corefile error!\033[0m\n");
    return -1;
  }

  if (in_guard_mode)
  {
    // guard
    if (children_instance == 0)
      guard_process(g_svc_name, argc, argv);

    clean_fds();
  }else
    output_pid(g_svc_name);

  // handle signal
  child_sig_handle();
  ::signal(SIGHUP, svc_exit_signal);

  reactor *r = new reactor();
  if (r->open(256, 16) != 0)
  {
    fprintf(stderr, "Error: reactor - open failed!\n");
    return -1;
  }

  if (init_svc(r, number_of_proxy) != 0)
  {
    fprintf(stderr, "Error: init_svc - init failed!\n");
    return -1;
  }

  s_log->rinfo("========================== launch ok =========================");
  e_log->rinfo("========================== launch ok =========================");

  r->run_reactor_event_loop();

  return 1;
}
