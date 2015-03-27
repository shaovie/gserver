// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-19 14:08
 */
//========================================================================

#ifndef CROSS_SVC_H_
#define CROSS_SVC_H_

// Lib header
#include "client.h"
#include "singleton.h"
#include "global_macros.h"

#define GLOBAL_SID_COMMUNICATE_WITH_CROSS 0

// Forward declarations
//

/**
 * @class cross_svc
 * 
 * @brief
 */
class cross_svc : public client
                  , public singleton<cross_svc>
{
  friend class singleton<cross_svc>;
public:
  int init(const char *host, const int port);

  virtual int open(void *);

  virtual int handle_timeout(const time_value &);

  virtual int handle_close(const int , reactor_mask m);

protected:
  int dispatch_msg(const int id, 
                   const int res,
                   const char *msg,
                   const int len);

  int dispatch_res_from_other_world(const int id, 
                                    const int res,
                                    const char *msg,
                                    const int len);

  //= cross msg
  int handle_heart_beat_result(const char *, const int, const int);
  int handle_login_result(const char *, const int, const int);

  //= cross request
  int crs_get_other_detail(const char *, const int, const int);
  int crs_get_rival_list(const char *, const int, const int);

private:
  cross_svc();

  void shutdown();
  int  do_login();
  int  do_heart_beat();
  int  check_heart_beat(const time_value &);

private:
  bool login_ok_;
  bool be_connecting_;

  int last_heart_beat_time_;
  int port_;
  int sid_;
  char host_[MAX_HOST_NAME + 1];
};
#endif // CROSS_SVC_H_

