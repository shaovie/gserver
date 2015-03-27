// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-30 19:46
 */
//========================================================================

#ifndef FORBID_OPT_MODULE_H_
#define FORBID_OPT_MODULE_H_

// Forward declarations
class forbid_opt;

/**
 * @class forbid_opt_module
 * 
 * @brief
 */
class forbid_opt_module
{
public:
  static int  do_insert_forbid_opt(const forbid_opt &fo, const bool preload);
  static int  do_del_forbid_opt(const forbid_opt &fo);

  static bool can_login_for_char(const int );
  static bool can_login_for_ip(const char *);
  static bool can_login_for_account(const char *);
  static bool can_talk(const int);
private:
  static void db_insert_forbit_opt(const forbid_opt *fo);
  static int  do_insert_forbid_char_login(const forbid_opt &fo, const bool preload);
  static int  do_insert_forbid_ip_login(const forbid_opt &fo, const bool preload);
  static int  do_insert_forbid_ac_login(const forbid_opt &fo, const bool preload);
  static int  do_insert_forbid_char_talk(const forbid_opt &fo, const bool preload);
};

#endif // FORBID_OPT_MODULE_H_

