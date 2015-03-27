// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-01-26 12:21
 */
//========================================================================

#ifndef SVC_MONITOR_H_
#define SVC_MONITOR_H_

// Lib header
#include "singleton.h"

// Forward declarations
class out_stream;
class svc_monitor_impl;

/**
 * @class svc_monitor
 * 
 * @brief
 */
class svc_monitor : public singleton<svc_monitor>
{
  friend class singleton<svc_monitor>;
public:
  void run(const int t);

  void handle_get_db_proxy_payload_result(const char *msg, const int len);

  int do_build_online(char *bf, const int bf_len);
  int do_build_scene_online(char *bf, const int bf_len);
  int do_build_db_proxy_payload(char *bf, const int bf_len);
private:
  svc_monitor();

  svc_monitor_impl *impl_;
};

#endif // SVC_MONITOR_H_

