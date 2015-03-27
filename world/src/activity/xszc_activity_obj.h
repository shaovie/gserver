// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-01 14:10
 */
//========================================================================

#ifndef XSZC_ACTIVITY_OBJ_H_
#define XSZC_ACTIVITY_OBJ_H_

#include "activity_obj.h"
#include "ilist.h"

// Lib header

// Forward declarations
class xszc_scp_info;
class mblock;
class player_obj;
class out_stream;
class scene_coord_t;

/**
 * @class xszc_activity_obj
 * 
 * @brief
 */
class xszc_activity_obj : public activity_obj
{
public:
  xszc_activity_obj();
  ~xszc_activity_obj();

  virtual int open();
  virtual int close();

  virtual void do_something(mblock *, int *, mblock *);

  void obtain_xszc_act_info(mblock *ret_mb);
  void obtain_xszc_group_pos(mblock *mb, mblock *ret_mb);

public:
  static int parse_cfg_param(const char *param);
  static scene_coord_t get_relive_coord(player_obj *player);
  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *player);

  static int do_enter_xszc(player_obj *player, const int scene_id);
  static int do_obtain_xszc_act_info(player_obj *player);
  static int do_obtain_xszc_group_pos(player_obj *player);
private:
  int construct_xszc_scp();
  int enter_xszc_scp(mblock *mb);
  void assign_group(player_obj *player, xszc_scp_info *xsi);
  int exit_xszc_scp(mblock *mb);
  int xszc_end(mblock *mb);

private:
  ilist<xszc_scp_info *> scp_list_;
};


#endif  // XSZC_ACTIVITY_OBJ_H_

