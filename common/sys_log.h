// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-31 21:32
 */
//========================================================================

#ifndef SYS_LOG_H_
#define SYS_LOG_H_

// Lib header
#include "ilog.h"
#include "singleton.h"

// Forward declarations

#define LOG_CFG_DIR    "cfg"

/**
 * @class sys_log
 * 
 * @brief for output common log record
 */
class sys_log : public singleton<sys_log>, public ilog
{
  friend class singleton<sys_log>;
private:
  sys_log() {};
};

/**
 * @class err_log
 * 
 * @brief for output error/fatal log record
 */
class err_log : public singleton<err_log>, public ilog
{
  friend class singleton<err_log>;
private:
  err_log() {};
};

/**
 * @class imp_log
 * 
 * @brief for output important log record
 */
class imp_log : public singleton<imp_log>, public ilog
{
  friend class singleton<imp_log>;
private:
  imp_log() {};
};

extern int init_sys_log(const char *);
#endif // SYS_LOG_H_

