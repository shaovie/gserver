// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-10-09 11:51
 */
//========================================================================

#ifndef SVC_CONFIG_H_
#define SVC_CONFIG_H_

// Lib header
#include "singleton.h"
#include "ilist.h"

#define SVC_CFG_PATH                     "svc.json"

// Forward declarations
class svc_config_impl;

/**
 * @class svc_config
 * 
 * @brief
 */
class svc_config : public singleton<svc_config>
{
  friend class singleton<svc_config>;
public:
  int load_config(const char *cfg_root);

  int service_sn();
  int max_online();
  const char *cfg_root();
  const char *group();

  int db_port();
  const char *db_host();
  const char *db_user();
  const char *db_passwd();
  const char *db_name();

  int db_proxy_port();
  const char *db_proxy_host();

  int world_port();
  int world_gm_port();
  const char *world_gm_host();

  int cross_port();
  const char *cross_host();
private:
  svc_config();
  svc_config(const svc_config &);
  svc_config& operator= (const svc_config&);

  svc_config_impl *impl_;
};

#endif // SVC_CONFIG_H_

