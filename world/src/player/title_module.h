// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-21 11:23
 */
//========================================================================

#ifndef TITLE_MODULE_H_
#define TITLE_MODULE_H_

// Forward declarations

class player_obj;
class in_stream;

/**
 * @class title_module
 *
 * @brief
 */
class title_module
{
public:
  static int handle_db_get_title_result(player_obj *player, in_stream &is);

  static void add_new_title(player_obj *player, const short title_cid);
};

#endif // TITLE_MODULE_H_

