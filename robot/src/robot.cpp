#include "client.h"
#include "def.h"
#include "daemon.h"
#include "message.h"
#include "sys_log.h"
#include "istream.h"
#include "player.h"
#include "global_macros.h"
#include "time_util.h"
#include "scene_config.h"
#include "global_param_cfg.h"
#include "skill_config.h"
#include "item_config.h"
#include "buff_config.h"
#ifdef FOR_ONEROBOT
#include "robot_ncurses.h"
#include "director.h"
#endif

#include "reactor.h"
#include "connector.h"
#include "svc_handler.h"
#include "mblock.h"
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

extern int tolua_player_open(lua_State *);
extern int tolua_lua_istream_open (lua_State *);

static ilog_obj *s_log = sys_log::instance()->get_ilog("robot_player");
static ilog_obj *e_log = err_log::instance()->get_ilog("robot_player");

int g_total_payload = 800;
int g_curr_payload = 0;
static int begin_id = 1;
int port = 0;
char *host = NULL;

std::vector<std::string> ac_list;

static connector<player> robot_connector;

lua_State *L = NULL;

class timer : public ev_handler
{
public:
  explicit timer(int id) : id_(id) { }

  int handle_timeout(const time_value &now)
  {
    time_util::now = now.sec();
    for (int i = 0;
         i < 40 && g_curr_payload < g_total_payload;
         ++i, ++this->id_)
    {
      if (this->id_ >= begin_id + g_total_payload)
      {
        e_log->rinfo("may be kick out other robot, because:");
        e_log->rinfo("  robot id shoudle be [%d---%d], now is:%d", begin_id, begin_id + g_total_payload -1, this->id_);
      }

      inet_address remote_addr(port, host);
      time_value tv(30, 0);
      char ac_bf[32] = {0};
      ::snprintf(ac_bf, sizeof(ac_bf), "robot_%d", this->id_);
      char name_bf[32] = {0};
      ::snprintf(name_bf, sizeof(name_bf), "name_%d", this->id_);
      if (robot_connector.connect(new player(ac_bf, name_bf), remote_addr, &tv) != 0)
        e_log->rinfo("connect world failed!");
    }
    return 0;
  }

private:
  int id_;
};
static int init_log()
{
  char sys_ini[] = 
		"roller=5\n"
		"file_size=5M\n"
		"dir=log\n"
		"file_name=robot.sys\n"
		"base=ALLS|ERROR|RINFO\n";
  char sys_name[128] = {0};
  ::snprintf(sys_name, 
						 sizeof(sys_name), 
						 "1234dfa343s_%d", 
						 getpid());
	FILE *f = fopen(sys_name, "w");
	if (f == NULL)
	{
		fprintf(stderr, "fopen %s err.\n", sys_name);
		return -1;
	}
	fwrite(sys_ini, sizeof(sys_ini) - 1, 1, f);
	fclose(f);

  char err_ini[] = 
		"roller=5\n"
		"file_size=5M\n"
		"dir=log\n"
		"file_name=robot.err\n"
		"base=ALLS\n";
  char err_name[128] = {0};
  ::snprintf(err_name, 
						 sizeof(err_name),
             "1234dfa343e_%d",
             getpid());
	f = fopen(err_name, "w");
	if (f == NULL)
	{
		fprintf(stderr, "fopen %s err\n", err_name);
		return -1;
	}
  fwrite(err_ini, sizeof(err_ini) - 1, 1, f);
  fclose(f);

  if (sys_log::instance()->init(sys_name) != 0)
  {
    fprintf(stderr, "Error: ilog - init sys log failed!\n");
    unlink(sys_name);
    return -1;
  }
	unlink(sys_name);
  
  if (err_log::instance()->init(err_name) != 0)
  {
    fprintf(stderr, "Error: ilog - init err log failed!\n");
    unlink(err_name);
    return -1;
  }
  unlink(err_name);
  return 0;
}
void svc_core_signal(int sig)
{
#ifdef FOR_ONEROBOT
  director::instance()->exit();
  fprintf(stderr, "signal %d", sig);
  ::exit(0);
#endif
}
int main (int argc, char *argv[])
{
#ifdef FOR_ONEROBOT
  if (argc != 3)
  {
    fprintf(stderr, "%s 192.168.1.221 4001!\n", argv[0]);
    return -1;
  }
#else
  if (argc != 5)
  {
    fprintf(stderr, "%s 192.168.1.221 4001 payload begin_idx!\n", argv[0]);
    return -1;
  }
#endif

  signal(SIGPIPE ,SIG_IGN);
  signal(SIGINT ,SIG_IGN);

  set_max_fds(1024);

  reactor r;
  if (r.open(10000, 2192) != 0)
  {
    fprintf(stderr, "open failed![%s]\n", strerror(errno));
    return -1;
  }

  if (init_log() != 0)
	{
		fprintf(stderr, "init log err.\n");
		return -1;
	}

  host = argv[1];
  port = ::atoi(argv[2]);
#ifndef FOR_ONEROBOT
  g_total_payload = ::atoi(argv[3]);
  begin_id = ::atoi(argv[4]);
#endif

  srand(time_value::start_time.sec());
  time_util::init();

  robot_connector.open(&r);
#ifndef FOR_ONEROBOT
  r.schedule_timer(new timer(begin_id),
                   time_value(0, 800*1000),
                   time_value(0, 800*1000));
#endif

  if (global_param_cfg::instance()->load_config("../config") != 0)
  {
    fprintf(stderr, "Error: config - load global_param_cfg config failed!\n");
    return -1;
  }
  if (scene_config::instance()->load_config("../config") != 0)
  {
    fprintf(stderr, "Error: config - load scene config failed!\n");
    return -1;
  }
  if (buff_config::instance()->load_config("../config") != 0)
  {
    fprintf(stderr, "Error: config - load buff config failed!\n");
    return -1;
  }
  if (skill_config::instance()->load_config("../config") != 0)
  {
    fprintf(stderr, "Error: config - load skill config failed!\n");
    return -1;
  }
  if (item_config::instance()->load_config("../config") != 0)
  {
    fprintf(stderr, "Error: config - load item config failed!\n");
    return -1;
  }

  L = luaL_newstate();
  luaL_openlibs(L);
#ifdef FOR_LUA
  tolua_player_open(L);
  tolua_lua_istream_open(L);
  if (luaL_loadfile(L, "src/ai_func.lua")
      || lua_pcall(L, 0, 0, 0))
  {
    fprintf(stderr, "load ai_func lua file failed!\n");
    return -1;
  }
  if (luaL_loadfile(L, "src/dispatch_msg.lua")
      || lua_pcall(L, 0, 0, 0))
  {
    fprintf(stderr, "load dispatch_msg lua file failed!\n");
    return -1;
  }
#endif

	s_log->rinfo("=============================begin=================");
	e_log->rinfo("=============================begin=================");

#ifdef FOR_ONEROBOT
  ::signal(SIGABRT, svc_core_signal);
  //::signal(SIGSEGV, svc_core_signal);
  inet_address remote_addr(port, host);
  time_value tv(30, 0);
  char ac_bf[32] = {0};
  login_ncurses::input_account(ac_bf);

  if (robot_connector.connect(new player(ac_bf, ""), remote_addr, &tv) != 0)
  {
    e_log->rinfo("connect world failed!");
    return 0;
  }
#endif

  r.run_reactor_event_loop();

#ifdef FOR_ONEROBOT
  director::instance()->exit();
#endif

  lua_close(L);
  return 0;
}
