// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-23 17:47
 */
//========================================================================

#ifndef GLOBAL_PARAM_CFG_H_
#define GLOBAL_PARAM_CFG_H_

#include "def.h"
#include "ilist.h"

// Lib header
#include "singleton.h"

// Forward declarations
class global_param_cfg_impl;

#define GLOBAL_PARAM_CFG_PATH                "global_param.json"

class sin_val_drop_param
{
public:
  int sin_min_;
  int sin_max_;
  int drop_rate_;
  int drop_cnt_min_;
  int drop_cnt_max_;
  int drop_coin_min_;
  int drop_coin_max_;
};

class rob_probability
{
public:
  short prob_;
  short min_per_;
  short max_per_;
};
/**
 * @class global_param_cfg
 *
 * @brief
 */
class global_param_cfg : public singleton<global_param_cfg>
{
  friend class singleton<global_param_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  static int lvl_limit;
  static int pk_lvl_limit;
  static int xiangke_param;
  static int hurt_base_val;
  static int hurt_base_area;
  static int mst_shanbi_per;
  static int mst_baoji_per;
  static int send_mail_cost;
  static int item_coin_cid;
  static int item_diamond_cid;
  static int item_bind_diamond_cid;
  static int relive_cid;
  static int relive_energy_p;
  static int birth_mission_cid;
  static scene_coord_t capital_relive_coord;
  static scene_coord_t birth_coord;
  static int dropped_item_free_time;
  static int dropped_item_destroy_time;
  static int mall_npc_cid;
  static int equip_price_coe;
  static int hate_val_coe;
  static int sin_val_decrease_time;
  static int add_sin_val;
  static int sin_val_upper_limit;
  static int drop_coin_system_recovery;
  static int create_guild_cost;
  static int guild_chairman_turn_days;
  static int guild_one_page_amt;
  static int guild_auto_dismiss_days;
  static int guild_create_min_char_lvl;
  static int guild_jb_init_cost;
  static int guild_jb_cost_add;
  static int guild_zhu_di_cid;
  static short guild_juan_xian_contrib_rate;
  static short guild_juan_xian_resource_rate;
  static char  guild_summon_boss_wday;
  static char  dxc_enter_cnt;
  static int char_common_cd[CHAR_CAREER_CNT + 1];
  static short pkg_cap;
  static short max_pkg_cap;
  static short storage_cap;
  static short max_storage_cap;
  static short enlarge_cap_num;
  static short enlarge_cap_cost;
  static ilist<sin_val_drop_param *> sin_val_drop_param_list;
  static int chat_channel_cd[CHANNEL_CNT];
  static short chat_lvl_limit;
  static unsigned int max_friend_cnt;
  static unsigned int max_blacklist_cnt;
  static short tili_val;
  static short tili_resume_interval;
  static short tili_resume_val;
  static int market_service_charge;
  static int market_one_page_amt;
  static int market_item_sale_time;
  static int market_min_service_charge;
  static int market_min_notice_charge;
  static int mian_bao_fu;
  static int xi_lian_lock_cid;
  static short vip_auto_be_at_lvl;
  static char vip_buy_add_exp_min_vip_lvl;
  static char tianfu_skill_upgrade_baoji_rate;
  static short tianfu_lvl_limit;
  static int tianfu_skill_cd;
  static ilist<int/*equip cid*/> birth_equip_list;
  static int bao_shi_jing_hua_cid;                 // 宝石精华CID
  static int bao_shi_activate_amt;                 // 宝石激活需要精华数量
  static int bao_shi_activate_cost;                // 宝石激活需要消耗金币
  static char bao_shi_pos_hole_amt;                // 宝石每个部位孔的数量
  static scene_coord_t energy_resume_pos;
  static int energy_resume_radius;
  static int energy_resume_per;
  static int energy_resume_interval;
  static int jing_ji_lvl;
  static int jing_ji_times;
  static int jing_ji_cd;
  static int jing_ji_reset_cost;
  static int jing_ji_map;
  static int jing_ji_prm[3];
  static coord_t jing_ji_mst;
  static ilist<pair_t<int>/*lvl, gift_cid*/> lvl_gift_list;
  static int ghz_scene_cid;
  static int ghz_prepare_time;
  static int ghz_lv_average;
  static int ghz_shop_npc;
  static int ghz_activate_shou_wei_cost;
  static int ghz_over_award[4];
  static int ghz_winner_award[4];
  static char *ghz_shou_wei_idx;
  static int zx_ten_cost;
  static int sao_dang_ka_cid;
  static int rank_amount;
  static pair_t<int> diamond_to_coin[4];
  static int tui_tu_perfect_p;
  static pair_t<int> tui_tu_star_p;
  static bool mail_valid;
  static bool market_valid;
  static bool jing_ji_valid;
  static ilist<int> diamond_not_cs_type_set;
  static int lucky_turn_cost;
  static int lucky_turn_score;
  static int lucky_turn_ten_cost;
  static int lucky_turn_ten_score;
  static pair_t<int> lucky_turn_award_diamond;
  static int lucky_turn_big_award_cid;
  static int back_town_cid;
  static int tili_recover;
  static int wang_zuo_cid;
  static int worship_cd;
  static short daily_worship_cnt;
  static int worship_coin_per;
  static int guild_scp_cid;
  static char guild_open_scp_wday;
  static int guangai_cooling_time;
  static int guangai_lvl_limit;
  static int chat_log_cnt;
  static int jing_ji_no_cd_times;
  static int guild_summon_mem_cost;
  static ilist<item_amount_bind_t> first_recharge_gift;
  static int world_lvl_rank;
  static int wei_xin_gift;
  static int skill_fight_score_coe;
  static int open_battle_room;
  static int over_battle_time;
  static int battle_scene_cid;
  static int battle_mst_fresh_time;
  static int each_battle_number;
  static int mid_leave_battle_time;
  static char jingli_value;
  static char jingli_cost;
  static short jingli_resume_interval;
  static short jingli_resume_value;
  static short battle_kill_build_honor;
  static short battle_kill_barracks_honor;
  static short battle_kill_tower_honor;
  static short battle_kill_mst_honor;
  static short battle_kill_role_honor;
  static short battle_win_honor;
  static short battle_draw_honor;
  static short battle_failure_honor;
  static int battle_win_gift;
  static int battle_draw_gift;
  static int battle_failure_gift;
  static char min_robbed_percent;
  static char max_robbed_percent;
  static ilist<int> robbed_items;
  static ilist<rob_probability> rob_prob_list;
  static short lue_duo_lvl;
  static int lue_duo_map;
  static int fabao_recovery_interval; // sec
  static pair_t<int> water_tree_award_good; // first:cid second:limit_cnt
  static pair_t<int> lucky_turn_award_good; // first:cid second:limit_cnt
  static int lucky_goods_turn_cnt;
  static int daily_luck_draw_award_cnt;
  static int free_relive_cnt;
  static int baowu_mall_refresh_diamond;
  static int baowu_mall_npc;
  static ilist<itime> baowu_mall_refresh_time;
  static short seven_day_login_open_at_lvl;
  static int mall_refresh_item;
  static int mushroom_coin_coe;
private:
  global_param_cfg();
  global_param_cfg(const global_param_cfg &);
  global_param_cfg& operator= (const global_param_cfg &);

  global_param_cfg_impl *impl_;
};

#endif // GLOBAL_PARAM_CFG_H_

