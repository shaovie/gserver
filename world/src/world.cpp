// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-31 10:55
 */
//========================================================================

// Lib header
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "reactor.h"
#include "time_value.h"

#include "sys.h"
#include "daemon.h"
#include "sys_log.h"
#include "global_macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

const char *g_svc_name = "iworld";

void print_usage()
{
  printf("\033[0;32m\nUsage: %s [OPTION...]\n\n", g_svc_name);
  printf("  -g                    Guard mode\n");
  printf("  -u  user name         Run as user\n");
  printf("  -C                    Core dump enable\n");
  printf("  -p  platform name     \n"); 
  printf("  -s  service sn        \n");
  printf("  -v                    Output version info\n");
  printf("  -h                    Help info\n");
  printf("\n");
  printf("\033[0m\n");
  ::exit(0);
}
void svc_exit_signal(int )
{ sys::to_friendly_exit(); }
void svc_core_signal(int sig)
{ sys::emergency_exit(sig); }
int main(int argc, char *argv[])
{ 
  if (argc == 1)
    print_usage();
  
  int in_guard_mode = 0;
  int dump_core = 0;
  int children_instance = 0;
  char *run_as_user = NULL;

  // parse args
  int c = -1;
  extern char *optarg;
  extern int optopt;

  const char *opt = ":gvu:Cs:ep:h";
  while((c = getopt(argc, argv, opt)) != -1)
  {
    switch (c)
    {
    case 'v':        // version
      fprintf(stderr, "\033[0;32m%s %s\033[0m\n", SVC_EDITION, BIN_V);
      return 0;
    case 'g':
      in_guard_mode = 1;
      break;
    case 'u':
      run_as_user = optarg;
      break;
    case 'C':
      dump_core = 1;
      break;
    case 'e':
      children_instance = 1;
      break;
    case 'p':
    case 's':
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

  const int max_clients = 1024*4;
  set_max_fds(max_clients);

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
    if (children_instance == 0)
      guard_process(g_svc_name, argc, argv);

    clean_fds();
  }else
    output_pid(g_svc_name);

  //= child process
  child_sig_handle();
  ::signal(SIGHUP, svc_exit_signal);
  ::signal(SIGABRT, svc_core_signal);
  ::signal(SIGSEGV, svc_core_signal);

  sys::r = new reactor();
  if (sys::r->open(max_clients, 128) != 0)
  {
    fprintf(stderr, "Error: reactor - open failed!\n");
    return -1;
  }

  if (sys::init_svc() != 0)
  {
    fprintf(stderr, "Error: init_svc - init failed!\n");
    return -1;
  }

  s_log->rinfo("launch ok! max fds:%d", get_max_fds());
  e_log->rinfo("launch ok! max fds:%d", get_max_fds());
  sys::svc_launch_time = time_value::gettimeofday().sec();

  // reactor event loop
  sys::r->run_reactor_event_loop();

  return 1;
}
