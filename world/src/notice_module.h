// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-11 16:44
 */
//========================================================================

#ifndef NOTICE_MODULE_H_
#define NOTICE_MODULE_H_

// Lib header
#include "ilist.h"

// Forward declarations
class item_obj;
class out_stream;
/**
 * @class notice_module
 *
 * @brief
 */
class notice_module
{
public:
  static void pick_up_dropped_item(const int char_id,
                                   const char *name,
                                   const int scene_cid,
                                   const int mst_cid,
                                   const item_obj *item);
  static void boss_appear(const int mst_cid, const int scene_cid);
  static void equip_strengthen(const int char_id,
                               const char *name,
                               const item_obj *,
                               const int qh_lvl);
  static void equip_rong_he(const int char_id,
                            const char *name,
                            const item_obj *,
                            const item_obj *,
                            const item_obj *);
  static void equip_xi_lian(const int char_id,
                            const char *name,
                            const item_obj *,
                            const int star,
                            const int attr);
  static void ghz_before_opened(const int min);
  static void ghz_opened();
  static void ghz_shou_wei_dead(const char *guild_name,
                                const int char_id,
                                const char *name,
                                const int idx);
  static void ghz_shou_wei_live(const char *guild_name,
                                const int char_id,
                                const char *name,
                                const int idx);
  static void ghz_over_win(const char *guild_name,
                           const int char_id,
                           const char *name);
  static void ghz_over_wei_mian(const char *guild_name);
  static void guild_summon_boss(const int guild_id,
                                const int char_id,
                                const char *name);
  static void zhan_xing(const int char_id, const char *name, const item_obj *);
  static void passive_skill_upgrade(const int char_id,
                                    const char *name,
                                    const int skill_cid,
                                    const int lvl);
  static void equip_strengthen_lucky_egg(const int char_id,
                                         const char *name,
                                         const item_obj *item,
                                         const int lvl);
  static void passive_skill_upgrade_lucky_egg(const int char_id,
                                              const char *name,
                                              const int skill_cid,
                                              const int lvl);
  static void cheng_jiu_lucky_egg(const int char_id, const char *name);
  static void bao_shi_up_to_10th(const int char_id,
                                 const char *name,
                                 const char part,
                                 const short lvl);
  static void bao_shi_upgrade_lucky_egg(const int char_id,
                                        const char *name,
                                        const char part,
                                        const short lvl);
  static void guild_lvl_up(const int guild_id,
                           const int char_id,
                           const char *name,
                           const short lvl);
  static void lucky_turn_award(const int char_id,
                               const char *name,
                               const item_obj *item);
  static void lucky_turn_big_award(const int char_id,
                                   const char *name,
                                   const int diamond);
  static void guild_mem_exit(const int guild_id,
                             const int char_id,
                             const char *name);
  static void produce_special_item(const int char_id,
                                   const char *name,
                                   const item_obj *item);
  static void jing_ji_rank_top(const int char_id, const char *name);
  static void jing_ji_win(const int char_id_1, const char *name_1,
                          const int char_id_2, const char *name_2,
                          const int rank);
  static void got_seven_day_login_award(const int char_id,
                                        const char *name,
                                        const item_obj *item);
  static void got_vip_equip_award(const int char_id,
                                  const char *name,
                                  const char vip_lvl,
                                  const item_obj *item);
  static void tianfu_skill_upgrade(const int char_id,
                                   const char *name,
                                   const int skill_cid,
                                   const short lvl);
  static void got_first_recharge_equip_award(const int char_id,
                                             const char *name,
                                             const item_obj *item);
  static void water_tree_goods_award(const int char_id,
                                     const char *name,
                                     const item_obj *item);
  static void lucky_goods_turn(const int char_id,
                               const char *name,
                               const item_obj *item);
  static void market_sale_item(const int char_id,
                               const char *name,
                               const int price,
                               const item_obj *item);
  static void market_sale_money(const int char_id,
                                const char *name,
                                const int value,
                                const int price);
public:
  static int  build_item_str(const item_obj *item, char *bf, const int bf_len);
private:
  static void build_char(const int char_id, const char *name, out_stream &os);
  static void build_guild(const char *name, out_stream &os);
  static void build_item(const item_obj *item, out_stream &os);
  static void build_number(const int number, out_stream &os);
  static void build_attr(const int attr, out_stream &os);
  static void build_scene(const int scene_cid, out_stream &os);
  static void build_monster(const int mst_cid, out_stream &os);
  static void build_npc(const int npc_cid, out_stream &os);
  static void build_part(const int npc_cid, out_stream &os);
  static void build_pskill(const int npc_cid, out_stream &os);
  static void build_tskill(const int skill_cid, out_stream &os);
};
#endif // NOTICE_MODULE_H_
