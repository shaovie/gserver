// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-10 09:52
 */
//========================================================================

#ifndef TUI_TU_MODULE_H_
#define TUI_TU_MODULE_H_

#include "ilist.h"

// Forward declarations
class player_obj;
class tui_tu_scp;
class tui_tu_log;
class in_stream;
class out_stream;
class char_brief_info;
class tui_tu_cfg_obj;
class tui_tu_chapter_cfg_obj;
class dropped_item;

/**
 * @class tui_tu_module
 * 
 * @brief
 */
class tui_tu_module
{
  friend class player_obj;
public:
  // handle client msg
  static int dispatch_msg(player_obj *,
                          const int msg_id,
                          const char *msg,
                          const int len);

  static void destroy(player_obj *player);
  static int handle_db_get_tui_tu_log_result(player_obj *player, in_stream &is);
  static void on_load_one_log(const tui_tu_log *l);
  static void on_char_login(player_obj *player);
  static void on_char_logout(player_obj *player);
  static void on_transfer_scene(player_obj *player,
                                const int from_scene_id,
                                const int from_scene_cid,
                                const int to_scene_id,
                                const int to_scene_cid);
  static void on_kill_mst(player_obj *player,
                          const int mst_cid,
                          ilist<dropped_item *> &dropped_item_list);
  static int get_total_star(player_obj *);
private:
  static int clt_obtain_tui_tu_chapter_info(player_obj *player, const char *msg, const int len);
  static int clt_obtain_tui_tu_info(player_obj *player, const char *msg, const int len);
  static int clt_enter(player_obj *player, const char *msg, const int len);
  static int clt_in_tui_tu_kill_mst(player_obj *player, const char *msg, const int len);
  static int clt_in_tui_tu_relive(player_obj *player, const char *msg, const int len);
  static int clt_exit(player_obj *player, const char *msg, const int len);
  static int clt_tui_tu_end(player_obj *player, const char *msg, const int len);
  static int clt_tui_tu_do_turn(player_obj *player, const char *msg, const int len);
  static int clt_tui_tu_sao_dang(player_obj *player, const char *msg, const int len);

  static char_brief_info *get_score_top_char(const int scene_cid, int &top_score);
  static void db_insert_tui_tu_log(player_obj *player, tui_tu_log *ttl);
  static void db_update_tui_tu_log(player_obj *player, tui_tu_log *ttl);
  static void do_clean_resource(player_obj *player);
  static void do_build_tui_tu_show_info(tui_tu_log *l, const int , out_stream &os);
  static int do_finish_tui_tu(player_obj *player,
                              tui_tu_log *ttl,
                              const tui_tu_cfg_obj *tui_tu_cfg,
                              const bool sao_dang);
  static int do_calc_star(const int used_time, const int limit_time);
  static int get_next_scene_cid(tui_tu_chapter_cfg_obj *, ilist<int> &);
  static int get_valid_max_chapter(player_obj *player, const int );
  static tui_tu_log *get_tui_tu_log(player_obj *player, const int scene_cid);
  static void on_star_update(player_obj *);
  static int get_chapter_idx(const int type, const int cid);
  static tui_tu_chapter_cfg_obj *get_chapter_cfg(const int type, const int idx);
  static void do_notify_boss_drop_items(player_obj *player, const int boss_cid);
};

#endif // TUI_TU_MODULE_H_

