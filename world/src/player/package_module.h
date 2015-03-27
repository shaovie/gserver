// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2012-10-15
 */
//========================================================================

#ifndef PACKAGE_MODULE_H_
#define PACKAGE_MODULE_H_

// Lib header
#include "ilist.h"

#include "item_obj.h"
#include "item_config.h"

// Forward declarations
class item_obj;
class player_obj;
class in_stream;
class out_stream;

enum
{
  UPD_ITEM          = 1,
  ADD_ITEM          = 2,
  DEL_ITEM          = 3,
};

/**
 * @class package_module
 *
 * @brief
 */
class package_module
{
public:
  enum
  {
    UPD_PKG         = 1,
    UPD_AMOUNT      = 2,
    UPD_BIND        = 3,
    UPD_EXTRA_INFO  = 4,
    UPD_CID         = 5,
    UPD_END
  };
  static void init(player_obj *player);
  static void destroy(player_obj *player);

  static int handle_db_get_item_list_result(player_obj *player, in_stream &is);

  // handle client msg
  static int dispatch_msg(player_obj *, const int msg_id, const char *msg, const int len);

  //= on event
  static void on_enter_game(player_obj *);
  static void on_char_dead(player_obj *, ilist<item_amount_bind_t> &drop_list);
  static void on_item_update(player_obj *player,
                             const int type,
                             item_obj *item,
                             const int arg[],
                             const int size,
                             const int behavior_sub_type);
  static void on_got_new_equip(item_obj *equip);

  //= logic
  static item_obj* alloc_new_item();
  static item_obj* alloc_new_item(const int char_id,
                                  const int item_cid,
                                  const int amount,
                                  const char bind_type);
  static void release_item(item_obj *item);

  static bool would_be_full(player_obj *player,
                            const char package,
                            const int item_cid[],
                            const int amount[],
                            const int bind_type[],
                            const int size);
  static bool would_be_full(player_obj *player,
                            const char package,
                            const int item_cid,
                            const int amount,
                            const int bind_type);
  static int used_space(player_obj *player, const char package);
  static short package_capacity(player_obj *player, const char package);

  static item_obj *find_item(player_obj *player, const int item_id);
  static item_obj *find_item_by_part(player_obj *player, const int part);
  static void find_all_item_in_pkg(player_obj *player, const int pkg, ilist<item_obj*> &list);
  static int calc_item_amount_all(player_obj *player, const int item_cid);
  static int calc_item_amount(player_obj *player, const int item_cid);
  static int calc_item_amount(player_obj *player, const int item_cid, const char bind);
  static void get_equip_for_view(player_obj *player, int &zhu_wu, int &fu_wu);

  static item_obj *do_insert_item(player_obj *player,
                                  const char package,
                                  const int item_cid,
                                  const int amount,
                                  const char bind_type,
                                  const int behavior_sub_type,
                                  const int src_id_1,
                                  const int src_id_2);
  static int do_insert_item(player_obj *player,
                            const char package,
                            item_obj *item,
                            const int behavior_sub_type,
                            const int src_id_1,
                            const int src_id_2);
  static int do_insert_award_item_list(player_obj *,
                                       ilist<item_amount_bind_t> *,
                                       const blog_t &);
  static int do_insert_or_mail_item(player_obj *player,
                                    const int item_cid,
                                    const int amount,
                                    const char bind,
                                    const int mail_cfg_id,
                                    const int behavior_sub_type_money,
                                    const int behavior_sub_type,
                                    const int src_id_1,
                                    const int src_id_2);
  static int do_remove_item(player_obj *player,
                            item_obj *item,
                            const int remove_amount,
                            const int behavior_sub_type,
                            const int src_id_1);
  static int do_remove_item(player_obj *player,
                            const int item_cid,
                            const int remove_amount,
                            const int behavior_sub_type,
                            const int src_id_1);
  static int do_remove_item(player_obj *player,
                            const int item_cid,
                            const char bind,
                            const int remove_amount,
                            const int behavior_sub_type,
                            const int src_id_1);
  static void do_move_item(player_obj *, item_obj *, const char );
  static void do_build_item_info(item_obj *io, out_stream &os);

private:
  //= client message
  static int clt_move_item(player_obj *player, const char *msg, const int len);
  static int clt_obtain_item_list(player_obj *player, const char *msg, const int len);
  static int clt_destroy_item(player_obj *player, const char *msg, const int len);
  static int clt_enlarge_capacity(player_obj *player, const char *msg, const int len);

  //
  static bool can_move_to(const int from, const int to);
  static int need_space(player_obj *player,
                        int &used_space,
                        const char package,
                        const int item_cid,
                        const int amount,
                        const char bind_type);
  static int on_item_update_for_db(player_obj *player,
                                   const int type,
                                   item_obj *item);
  static void on_item_update_for_clt(player_obj *player,
                                     const int type,
                                     item_obj *item,
                                     const int arg[],
                                     const int size,
                                     const int behavior_sub_type);
  static void on_add_item(player_obj *player,
                          const int item_cid,
                          const int amount,
                          const int bind_type,
                          const int behavior_sub_type,
                          const int src_id_1,
                          const int src_id_2);
  static void on_remove_item(player_obj *player,
                             const int item_cid,
                             const int amount,
                             const int bind_type,
                             const int behavior_sub_type,
                             const int src_id_1);
  static void do_build_item_list(player_obj *, const char , out_stream &os);
  static void do_insert_new_items(player_obj *, const char, item_obj *, const int behavior_sub_type);
  static void do_item_dj(player_obj *, const char, item_obj *, const int behavior_sub_type);
};

extern int g_item_amount_bind[][MAX_PACKAGE_CAPACITY];
extern int g_item_update_info[package_module::UPD_END];
#endif // PACKAGE_MODULE_H_

