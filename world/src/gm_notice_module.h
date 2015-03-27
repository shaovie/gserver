// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2013-01-15 11:59
 */
//========================================================================

#ifndef GM_NOTICE_MODULE_H_
#define GM_NOTICE_MODULE_H_

// Forward declarations
class notice_info;

/**
 * @class gm_notice_module
 *
 * @brief
 */
class gm_notice_module
{
public:
  static void load_notice_from_db(notice_info *ni);
  static void do_timeout(const int now);

  static void replace_notice(notice_info &ni);
private:
  static int assign_notice_id();
  static void delete_notice(const int id);
private:
  static bool first_check;
  static int last_check_time;
  static int max_notice_id;
};

#endif // GM_NOTICE_MODULE_H_

