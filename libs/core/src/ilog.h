// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-16 15:05
 */
//========================================================================

#ifndef ILOG_H_
#define ILOG_H_

// Forward declarations
class ilog_impl;
class ilog_obj_impl;

/**
 * @class ilog_obj
 * 
 * @brief 
 */
class ilog_obj
{
  friend class ilog_impl;
public:
  // Without any format to be output.
  int log(const char *record, const int len);

  int trace(const char *format, ...);
  int debug(const char *format, ...);
  int wning(const char *format, ...);
  int error(const char *format, ...);
  int rinfo(const char *format, ...);
  int fatal(const char *format, ...);
private:
  ilog_obj(ilog_obj_impl *);

  ilog_obj_impl *impl_;
};

/**
 * @class ilog
 * 
 * @brief 
 */
class ilog
{
public:
  ilog();

  int init(const char *config_name);

  ilog_obj *get_ilog(const char *module_name);
private:
  ilog_impl *impl_;
};

#endif // ILOG_H_

