#include <stdio.h>
#include <signal.h>
#include "reactor.h"
#include "my_client.h"

int g_concurrency = 10,
    g_type = 1,
    g_good_cnt = 0,
    g_done_cnt = 0,
    g_success_cnt = 0; //全局名字命名规则，也要凸显用途，顾名知义
int64_t g_recv_cnt = 0;
time_value g_start_time;
inet_address g_addr;

void print_result()
{
  if (g_type == TYPE_CONNECT)
    fprintf(stdout, "Connect: %d, Success: %d\n", g_done_cnt, g_good_cnt);
  else if (g_type == TYPE_ONE_REQUEST)
    fprintf(stdout, "Connect: %d, Success: %d\n", g_done_cnt, g_success_cnt);
  else
  {
    time_value intv = time_value::gettimeofday() - g_start_time;
    double all_time = intv.sec() + intv.usec() / 1000000.0;
    double QPS = g_recv_cnt / all_time;
    fprintf(stdout, "Over %ld Packages\n", g_recv_cnt);
    fprintf(stdout, "Time %lf(s)\n", all_time);
    fprintf(stdout, "QPS  %.2lf\n", QPS);
  }
}
void reset()
{
  g_start_time = time_value::gettimeofday();
  g_recv_cnt = 0;
}
void see_result(int )
{
  print_result();
  reset();
}

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    fprintf(stdout, "Usage: %s `type` `host` `port` `conn`\n", argv[0]);
    return 0;
  }
  g_type = ::atoi(argv[1]);
  const int port = ::atoi(argv[3]);
  g_concurrency = ::atoi(argv[4]);
  g_addr.set(port, argv[2]);

  if (g_type < TYPE_CONNECT || g_type > TYPE_ALIVE_CONN)
  {
    fprintf(stderr, "Type is invalid!\n");
    return 0;
  }

  ::signal(SIGHUP, see_result);
  ::signal(SIGPIPE, SIG_IGN);

  reactor *r = new reactor();
  if (r->open(1024, 16) != 0)
  {
    fprintf(stderr, "Error: reactor - open failed!\n");
    return 0;
  }
  connect_timer::instance()->init(r);
  if (r->schedule_timer(connect_timer::instance(),
                        time_value(0,0),
                        time_value(0,300000)) == -1)
  {
    fprintf(stderr, "Error: schedule timer failed!\n");
    return 0;
  }
  reset();
  r->run_reactor_event_loop();
  return 0;
}
