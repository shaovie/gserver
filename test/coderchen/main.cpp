#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>

#include "time_value.h"
#include "client_obj.h"
#include "conn_mgr.h"

static void signal_handler(int);

uint64_t g_total_conn_success_times = 0;
uint64_t g_total_conn_failed_times = 0;
uint64_t g_total_rcv_pkg_cnt = 0;
static std::ofstream g_out_file;
static time_value g_begin_time;

int main(int argc, char *argv[])
{
  char host[16] = {0};
  ::strncpy(host, argv[1], sizeof(host) - 1);
  int port = ::atoi(argv[2]);

  ::srand(::time(NULL));
  ::signal(SIGINT, signal_handler);
  ::signal(SIGHUP, SIG_IGN);
  ::signal(SIGPIPE, SIG_IGN);
  ::signal(SIGQUIT, signal_handler);

  reactor r; 
  if (r.open(10000, 200) != 0)
    return 0;

  g_out_file.open("./log", std::ios::app);
  g_out_file << "---------------------begin--------------------" << std::endl;
  
  conn_mgr<client_only_connect> only_connect_mgr(host, port, 30);
  only_connect_mgr.open(&r);
  conn_mgr<client_send_1_msg> one_msg_connect_mgr(host, port, 20);
  one_msg_connect_mgr.open(&r);
  conn_mgr<client_keep_alive> keep_alive_connect_mgr(host, port, 5);
  keep_alive_connect_mgr.open(&r);

  g_begin_time = time_value::gettimeofday();
  r.run_reactor_event_loop();
  return 0;
}
static void signal_handler(int signum)
{
  time_value now = time_value::gettimeofday();
  int run_time_ms = (now - g_begin_time).msec();

  g_out_file << std::endl;
  g_out_file << "[timestamp]: " << g_begin_time.sec() << " --- " << now.sec() << std::endl;
  g_out_file << "   conn success times: " << g_total_conn_success_times << std::endl;
  g_out_file << "   conn failed times:  " << g_total_conn_failed_times << std::endl << std::endl;
  g_out_file << "   total recv pkg cnt: " << g_total_rcv_pkg_cnt << std::endl;
  g_out_file << "   use time:           " << run_time_ms / 1000.0 << "(s)" << std::endl;

  if (run_time_ms)
    g_out_file << "   QPS:                " 
               << g_total_rcv_pkg_cnt * 1000.0 / run_time_ms 
               << "(req/s)" 
               << std::endl;

  g_total_conn_success_times = 0;
  g_total_conn_failed_times = 0;
  g_total_rcv_pkg_cnt = 0;
  g_begin_time = now;
  
  if (signum == SIGQUIT)
    ::exit(0);
}
