// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-07-03 15:08
 */
//========================================================================

#ifndef CROSS_MODULE_H_
#define CROSS_MODULE_H_

// Forward declarations
class player_obj;

/**
 * @class cross_module
 *
 * @brief
 */
class cross_module
{
public:
  static int dispatch_msg(player_obj *player,
                          const int msg_id,
                          const char *msg,
                          const int len,
                          const int res);

  // db response
  static void handle_db_cross_get_item_single_pkg_result(const char *msg, const int len);

private:
  // client request
  static int clt_get_rival_list(player_obj *, const char *, const int);
  static int clt_get_other_detail(player_obj *, const char *, const int);

  // cross response
  static int handle_get_rival_list(player_obj *, const char *, const int, const int);
  static int handle_get_other_detail(player_obj *, const char *, const int, const int);
};
#endif // CROSS_MODULE_H_
