// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-07-29 15:40
 */
//========================================================================

#ifndef MOBAI_MODULE_H_
#define MOBAI_MODULE_H_

// Lib header
#include "ilist.h"

// Forward declarations
class player_obj;
class in_stream;
class out_stream;
class item_obj;
class worship_info;

/**
 * @class mobai_module
 *
 * @brief
 */
class mobai_module
{
public:
  static int  dispatch_msg(player_obj *, const int , const char *, const int );
  static void on_guild_demise(const int guild_id);
  static void on_guild_dismiss(const int guild_id);
  static void on_ghz_result(const int old_guild_id, const int win_guild_id);
  static void on_transfer_scene(player_obj *,
                                const int from_scene_cid,
                                const int to_scene_cid);
  static void handle_db_get_castellan_item_single_pkg_result(player_obj *, in_stream &);
  static void on_enter_game(player_obj *);
private:
  static void do_replace_castellan(const int new_id);
  static void do_bro_worsip_statue_info_to_clt(const char career);
  static void do_notify_worsip_statue_info_to_clt(player_obj *);
  static void update_wrc_info();
  static void do_build_worship_info(player_obj *slef,
                                    player_obj *target,
                                    const int target_id,
                                    ilist<item_obj*> *list,
                                    out_stream &);

  //= clt
  static int  clt_obtain_worship_info(player_obj *, const char *, const int);
  static int  clt_worship_castellan(player_obj *, const char *, const int);
  static int  clt_get_worship_coin(player_obj *, const char *, const int);
public:
  static worship_info *wrc_info;
};

#endif // MOBAI_MODULE_H_
