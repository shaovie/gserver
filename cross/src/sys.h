// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-18 19:11
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

  static int reload_config(const char *msg, 
                           const int /*len*/, 
                           char *result, 
                           const int res_in_len,
                           int &result_len);
private:
  static int launch_net_module();

  static int load_config();

  static int dispatch_config(const char *name,
                             char *result,
                             const int res_in_len,
                             int &result_len);
public:
  static reactor *r;
};
#endif // SYS_H_

