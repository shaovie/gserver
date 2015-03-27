// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-31 21:32
 */
//========================================================================

#ifndef SYS_H_
#define SYS_H_

#include "ilist.h"
#include "global_macros.h"

// Forward declarations
class reactor;
class service_info;

/**
 * @class sys
 *
 * @brief
 */
class sys
{
public:
  static int init_svc();

  static char assign_career();
  static void select_career(const char career);

  static int assign_char_id();
  static int assign_guild_id();
  static int assign_forbid_id();

  static void to_friendly_exit();
  static void emergency_exit(const int sig);

  static int reload_config(const char *msg,
                           const int /*len*/,
                           char *result,
                           const int res_in_len,
                           int &result_len);
  static void update_svc_info();
private:
  static int launch_net_module();

  static int load_config();

  static int do_check_config();

  static int dispatch_config(const char *name,
                             char *result,
                             const int res_in_len,
                             int &result_len);
public:
  static bool  friendly_exit;

  static short world_lvl;
  static int   last_char_id;
  static int   last_guild_id;
  static int   last_forbid_id;

  static int   svc_launch_time;
  static service_info *svc_info;

  static reactor *r;
};
#endif // SYS_H_

