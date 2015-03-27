// Lib header
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <execinfo.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/file.h>
#include "date_time.h"

#define PID_DIR      "pid"

char g_ver_info[128] = {0};
pid_t g_children_pid = 0;

char *build_version_info(const char *svc_name,
                         const char *svc_edition,
                         const char *bin_v,
                         const char *build_time,
                         const int launch_time,
                         int &result_len)
{
  static char result[512] = {0};
  result_len = 0;
  result_len += ::snprintf(result + result_len,
                           sizeof(result) - result_len,
                           "\"%s\":{", svc_name);
  result_len += ::snprintf(result + result_len,
                           sizeof(result) - result_len,
                           "\"%s\": \"%s\", ", "for", svc_edition);
  result_len += ::snprintf(result + result_len,
                           sizeof(result) - result_len, 
                           "\"%s\": \"%s\", ", "bin", bin_v);
  result_len += ::snprintf(result + result_len,
                           sizeof(result) - result_len, 
                           "\"%s\": \"%s\", ", "bin_build_time", build_time);

  int day = 0;
  int hour = 0;
  int min = 0;
  day  = (launch_time / (24 * 3600));
  hour = (launch_time % (24 * 3600)) / 3600;
  min  = ((launch_time % (24 * 3600)) % 3600) / 60;
  result_len += ::snprintf(result + result_len,
                           sizeof(result) - result_len, 
                           "\"%s\": \"%dd-%dh-%dm\", ", "up_time", day, hour, min);
  result_len -= 2;
  result_len += ::snprintf(result + result_len, sizeof(result) - result_len, "}");
  return result;
}
static void guard_log(const char *format, ...)
{
  va_list argptr;
  ::va_start(argptr, format);
  FILE *fp = ::fopen("debug.log", "a+");
  if (fp == NULL)
    return;
  ::vfprintf(fp, format, argptr);
  ::fclose(fp);
}
static void kill_children(int )
{
  if (g_children_pid != 0)
    ::kill(g_children_pid, SIGHUP);
  ::exit(0);
}
void child_sig_handle()
{
  ::signal(SIGPIPE, SIG_IGN);
  ::signal(SIGINT,  SIG_IGN);
}
int set_max_fds(int maxfds)
{
  struct rlimit limit;
  limit.rlim_cur = maxfds;
  limit.rlim_max = maxfds;

  return setrlimit(RLIMIT_NOFILE, &limit);
}
int get_max_fds()
{
  struct rlimit rl;
  ::memset((void *)&rl, 0, sizeof(rl));
  int r = ::getrlimit(RLIMIT_NOFILE, &rl);
  if (r == 0 && rl.rlim_cur != RLIM_INFINITY)
    return rl.rlim_cur;
  return -1;
}
int dump_corefile()
{
  if (prctl(PR_SET_DUMPABLE, 1) == -1)
    fprintf(stderr, "Error: prctl - failed to prctl!");

  struct rlimit rlim_new;
  struct rlimit rlim;
  if (getrlimit(RLIMIT_CORE, &rlim) == 0) 
  {
    rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &rlim_new) != 0) 
      return -1;
  }

  if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0) 
  {
    fprintf(stderr, "Error: getrlimit - failed to ensure corefile creation\n");
    return -1;
  }
  return 0;
}
int runas(const char *uid)
{
  struct passwd *pw = NULL;
  if ((pw = getpwnam(uid)) == NULL)
  {
    fprintf(stderr, "Error: getpwnam - unknown user [%s]!\n", uid);
    return -1;
  }else if (setgid(pw->pw_gid) == -1)
  {
    fprintf(stderr, "Error: setgid(%s): error !\n", uid);
    return -1;
  }else if (setuid(pw->pw_uid) == -1)
  {
    fprintf(stderr, "Error: setuid(%s): error!\n", uid);
    return -1;
  }
  endpwent();
  return 0;
} 
void clean_fds()
{
  for (int i = get_max_fds() - 1; i >= 0; --i)
    ::close(i);

  int fd = ::open ("/dev/null", O_RDWR, 0);
  if (fd != -1)
  {
    ::dup2(fd, 0);
    ::dup2(fd, 1);
    ::dup2(fd, 2);

    if (fd > 2) ::close(fd);
  }
}
char **g_children_argv = NULL;
void do_execv(int argc, char *argv[])
{
  g_children_argv = new char *[argc + 2];
  for (int n = 0; n < argc; ++n)
    g_children_argv[n] = strdup(argv[n]);
  g_children_argv[argc] = strdup("-e");
  g_children_argv[argc + 1] = NULL;
  ::execv(argv[0], g_children_argv);
  ::exit(1);
}
void output_pid(const char *svc_name)
{
  char bf[32] = {0};
  int len = ::snprintf(bf, sizeof(bf), "%d", getpid());

  char file_name[128] = {0};
  ::snprintf(file_name, sizeof(file_name), "%s/%s.pid", PID_DIR, svc_name);
  int fd = ::open(file_name, O_CREAT|O_WRONLY, 0777);
  if (::flock(fd, LOCK_EX|LOCK_NB) != 0)
  {
    guard_log("%s|%s|%d|file locked!\n",
              file_name, date_time().to_str(bf, sizeof(bf)), errno);
    ::exit(1);
  }
  ::ftruncate(fd, 0);
  ::write(fd, bf, len);
}
void guard_process(const char *svc_name, int argc, char *argv[])
{
  if (::mkdir(PID_DIR, 0755) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Error: guard - open dir %s failed!\n", PID_DIR);
    ::exit(-1);
  }

  output_pid(svc_name);

  pid_t retpid = 0;
  signal(SIGHUP, kill_children);
  ::signal(SIGPIPE, SIG_IGN);
  ::signal(SIGINT,  SIG_IGN);
  //
  while(1)
  {
    g_children_pid = ::fork();
    if (g_children_pid < 0)      // < 0 is wrong! 
    {
      char bf[64] = {0};
      guard_log("%s|%s|%d|fork failed: %s!\n",
                svc_name, date_time().to_str(bf, sizeof(bf)), errno);
      ::exit(-1);
    }else if (g_children_pid == 0)
    {
      if (argv == NULL) return ;
      if (g_children_argv  == NULL)
        do_execv(argc, argv);
    }
    // parent process !

    int status = 0;
    char time_bf[64] = {0};

    if ((retpid = waitpid(g_children_pid, &status, 0)) < 0)
    {
      date_time().to_str(time_bf, sizeof(time_bf));
      guard_log("%s|%s|%d|parent catch child: %u failed\n", 
                svc_name, time_bf, errno, g_children_pid);
      ::exit(-1);
    }
    date_time().to_str(time_bf, sizeof(time_bf));
    if (WIFEXITED(status)) //is non-zero if the child exited normally
    {
      guard_log("%s|%s|%d|child %d exit normally, status=%d!\n",
                svc_name, time_bf, 0, retpid, WEXITSTATUS(status));
      ::exit(0);
    }
    if (WIFSIGNALED(status)) //program exit by a siganl but not catched
    {
      guard_log("%s|%s|%d|child %d exit by signal %d!\n",
                svc_name, time_bf, 2, retpid, WTERMSIG(status));
      // goto fork
    }
    ::sleep(2);
  }
}
