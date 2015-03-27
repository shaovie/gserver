#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <time.h>

#include "client_no_msg.h"
#include "client_1_msg.h"
#include "client_keep_alive.h"
#include "connector.h"
#include "reactor.h"
#include "time_value.h"

const int PORT = 4010;
const char *HOST = "192.168.1.221";

int g_all_connect_num;
int g_once_connect_num;
int g_success_connect_num;
int g_failed_connect_num;

int g_all_send_num;
int g_receive_right_num;
int g_discard_num;

int g_begin_clock;
int g_end_clock;

void print_useage()
{
  std::cout << "usage [option...] follows" << std::endl;
  std::cout << " -t     1: only connect 2: send one message 3:keep alive" << std::endl;
  std::cout << " -c     connct num at one time " << std::endl;
  std::cout << " -n     all connect num " << std::endl;
}

void handle_signal_quit(int signu)
{
  std::cout << "success connect num:  " << g_success_connect_num << std::endl;
  std::cout << "failed connect num:   " << g_failed_connect_num << std::endl;

  g_end_clock = clock();
  int use_clock = g_end_clock - g_begin_clock;
  double use_time = (double)use_clock/CLOCKS_PER_SEC;
  if ( use_time < 0.00001 &&  use_time > -0.00001)
    return ;
  std::cout << "all send package:     " << g_all_send_num << std::endl;
  std::cout << "receive right msg num:" << g_receive_right_num << std::endl;
  std::cout << "discard msg num:      " << g_discard_num << std::endl;
  std::cout << "QPS:                  " << g_receive_right_num/use_time << std::endl;

  g_all_send_num = 0;
  g_receive_right_num = 0;
  g_discard_num = 0;

  g_begin_clock = clock();
  g_end_clock = 0;
}
int main(int argc, char *argv[])
{
  if (argc < 7)
  {
    print_useage();
    return 0;
  }
  signal(SIGPIPE, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, handle_signal_quit);

  int connect_type = 0;
  int c = -1;
  const char *opt = "t:c:n:";
  while((c = getopt(argc, argv, opt)) != -1)
  {
    switch(c)
    {
    case 'c':
      g_once_connect_num = ::atoi(optarg);
      break;
    case 'n':
      g_all_connect_num = ::atoi(optarg);
      break;
    case 't':
      connect_type = ::atoi(optarg);
      break;
    case '?':
    default:
      print_useage();
      return 0;
    }
  }

  if (connect_type > 3 || connect_type < 1 || g_once_connect_num < 1 || g_all_connect_num < g_once_connect_num)
  {
    print_useage();
    return 0;
  }
  int log_fd = open("./log", O_RDWR | O_CREAT | O_APPEND | O_APPEND);
  if (log_fd == -1)
  {
    std::cout << "init log fd error" << std::endl;
    return 0;
  }
  if (dup2(log_fd, STDOUT_FILENO) < 0)
  {
    std::cout << "dup stdout to log" << std::endl;
    return 0;
  }

  reactor *epoll_reactor = new reactor();
  if (epoll_reactor->open(1000, 100) == -1)
  {
    std::cout << "epoll reactor open failed" << std::endl;
    return 0;
  }
  int ret = 0;
  if (connect_type == 1)
    ret = connect_mgr<client_no_msg>::instance()->init(epoll_reactor, PORT, HOST);
  else if (connect_type == 2)
    ret = connect_mgr<client_1_msg>::instance()->init(epoll_reactor, PORT, HOST);
  else
    ret = connect_mgr<client_keep_alive>::instance()->init(epoll_reactor, PORT, HOST);

  if (ret < 0)
  {
    std::cout << "connect mgr init failed " << std::endl;
    return 0;
  }

  g_begin_clock = clock();
  epoll_reactor->run_reactor_event_loop();
  return 0;
}
