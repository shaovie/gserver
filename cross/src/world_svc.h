// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-18 19:48
 */
//========================================================================

#ifndef WORLD_SVC_H_
#define WORLD_SVC_H_

// Lib header
#include "client.h"
#include "global_macros.h"

// Forward declarations
//

/**
 * @class world_svc
 * 
 * @brief
 */
class world_svc : public client
{
public:
  world_svc();

  virtual ~world_svc();

  virtual int open(void *);

  virtual int handle_timeout(const time_value & /*now*/);

  const char *group() const { return this->group_; }
  int svc_id() const { return this->svc_id_; }

protected:
  int dispatch_msg(const int id, 
                   const int res,
                   const char *msg,
                   const int len);

  //= world msg
  int wld_heart_beat(const char *msg, const int len);
  int wld_login(const char *msg, const int len);

  int wld_route_req_msg(const int res,
                        const char *msg,
                        const int len);
  int wld_route_res_msg(const int res,
                        const char *msg,
                        const int len);
private:
  bool login_ok_;
  int sid_;
  int svc_id_;
  int last_heart_beat_;
  char group_[MAX_GROUP_NAME_LEN + 1];
};
#endif // WORLD_SVC_H_

