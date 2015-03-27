// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-20 22:59
 */
//========================================================================

#ifndef BEHAVIOR_LOG_H_
#define BEHAVIOR_LOG_H_

#include <time.h>
#include <stdarg.h>

//= Lib header
#include "singleton.h"

#include "behavior_id.h"

// Forward declarations
//
class log_store;

#define BEHAVIOR_LOG_DIR                          "backlog"
#define BEHAVIOR_LOG_CFG_PATH                     "behavior_log.json"
#define MAX_LEN_OF_ONE_RECORD                     255 // 256 - 1

/**
 * @class behavior_log
 * 
 * @brief
 */
class behavior_log : public singleton<behavior_log>
{
  friend class singleton<behavior_log>;
public:
  int open(const char *cfg_root);

  int reload_config(const char *cfg_root);

  void store(const int id, const int now, const char *fmt, ...);

  void flush(const int now);
private:
  log_store *log_store_;

  behavior_log();
};

#endif // BEHAVIOR_LOG_H_

