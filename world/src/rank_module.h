// -*- C++ -*-

//========================================================================
/**
 * Author   : sparkcui
 * Date     : 2013-06-21 16:23
 */
//========================================================================

#ifndef RANK_MODULE_H_
#define RANK_MODULE_H_

#include <stdint.h>

#include "ilist.h"

typedef bool (*lueduo_check_handler)(const int, const int);

// Forward declarations
class player_obj;
class in_stream;

/**
 * @class rank_module
 *
 * @brief
 */
class rank_module
{
public:
  enum
  {
    RANK_ZHANLI          = 1,
    RANK_LVL             = 2,
    RANK_JING_JI         = 3,
    RANK_MSTAR           = 4,

    RANK_END
  };
public:
  static int dispatch_msg(player_obj *, const int, const char *, const int);

  static int init();
  static void update_world_lvl(const int lvl);
  static void do_timeout(const int);
  static void on_new_day(const int);
  static void on_char_get_exp(const int, const short, const int64_t,
                              const short, const int64_t, const bool preload = false);
  static void on_char_zhanli_change(const int, const int,
                                    const int, const bool preload = false);
  static void on_char_mstar_change(const int, const int,
                                   const int, const bool preload = false);

  static void get_char_list_before_rank(const int type, ilist<int> &, const int rank);

  static int rand_players_for_lue_duo(player_obj *player,
                                      const int material_cid,
                                      lueduo_check_handler handler,
                                      const int need_cnt,
                                      int result[]);
private:
  static int clt_obtain_rank_info(player_obj *, const char *, const int);
  static int clt_obtain_player_detail_info(player_obj *, const char *, const int);
};

#endif // RANK_MODULE_H_

