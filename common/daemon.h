// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-31 13:05
 */
//========================================================================

#ifndef DAEMON_H_
#define DAEMON_H_

extern const char *g_svc_name;

extern char *build_version_info(const char *svc_name,
                                const char *svc_edition,
                                const char *bin_v,
                                const char *build_time,
                                const int launch_time,
                                int &result_len);

extern void child_sig_handle();

extern void guard_process(const char *, int argc, char *argv[]);
extern void output_pid(const char *svc_name);

extern int set_max_fds(int );
extern int get_max_fds();

extern void clean_fds();

extern int runas(const char *uid);

extern int dump_corefile();

extern int check_qq_http_header(const char *msg, int &header_len);
#endif // DAEMON_H_

