#include "global_param_cfg.h"
#include "load_json.h"

int global_param_cfg::lvl_limit                      = 200;
int global_param_cfg::pk_lvl_limit                   = 30;
int global_param_cfg::xiangke_param                  = 250;
int global_param_cfg::hurt_base_val                  = 90;
int global_param_cfg::hurt_base_area                 = 20;
int global_param_cfg::mst_shanbi_per                 = 10;
int global_param_cfg::mst_baoji_per                  = 10;
int global_param_cfg::send_mail_cost                 = 100;
int global_param_cfg::mall_npc_cid                   = 41210002;
int global_param_cfg::equip_price_coe                = 1000;
int global_param_cfg::hate_val_coe                   = 1000;
int global_param_cfg::sin_val_decrease_time          = 900;
int global_param_cfg::add_sin_val                    = 2;
int global_param_cfg::sin_val_upper_limit            = 999;
int global_param_cfg::item_coin_cid                  = 0;
int global_param_cfg::item_diamond_cid               = 0;
int global_param_cfg::item_bind_diamond_cid          = 0;
int global_param_cfg::relive_cid                     = 0;
int global_param_cfg::relive_energy_p                = 20;
int global_param_cfg::birth_mission_cid              = 2000;
scene_coord_t global_param_cfg::birth_coord;
scene_coord_t global_param_cfg::capital_relive_coord;
scene_coord_t global_param_cfg::energy_resume_pos;
int global_param_cfg::dropped_item_free_time         = 60;
int global_param_cfg::dropped_item_destroy_time      = 120;
int global_param_cfg::drop_coin_system_recovery      = 10;
int global_param_cfg::create_guild_cost              = 100000;
int global_param_cfg::guild_chairman_turn_days       = 5;
int global_param_cfg::guild_one_page_amt             = 20;
int global_param_cfg::guild_auto_dismiss_days        = 0;
int global_param_cfg::guild_create_min_char_lvl      = 0;
int global_param_cfg::guild_jb_init_cost             = 0;
int global_param_cfg::guild_jb_cost_add              = 0;
int global_param_cfg::guild_zhu_di_cid               = 0;
short global_param_cfg::guild_juan_xian_contrib_rate = 100;
short global_param_cfg::guild_juan_xian_resource_rate = 100;
char global_param_cfg::guild_summon_boss_wday        = 3;
char global_param_cfg::dxc_enter_cnt                 = 3;
ilist<sin_val_drop_param *> global_param_cfg::sin_val_drop_param_list;
int global_param_cfg::char_common_cd[CHAR_CAREER_CNT + 1] = {0, 500, 400, 500};
int global_param_cfg::chat_channel_cd[CHANNEL_CNT];
short global_param_cfg::pkg_cap                      = 16;
short global_param_cfg::max_pkg_cap                  = 128;
short global_param_cfg::storage_cap                  = 16;
short global_param_cfg::max_storage_cap              = 128;
short global_param_cfg::enlarge_cap_num              = 4;
short global_param_cfg::enlarge_cap_cost             = 4;
short global_param_cfg::chat_lvl_limit               = 20;
unsigned int global_param_cfg::max_friend_cnt        = 20;
unsigned int global_param_cfg::max_blacklist_cnt     = 20;
short global_param_cfg::tili_val                     = 300;
short global_param_cfg::tili_resume_interval         = 30;
short global_param_cfg::tili_resume_val              = 5;
int global_param_cfg::market_service_charge          = 0;
int global_param_cfg::market_one_page_amt            = 0;
int global_param_cfg::market_item_sale_time          = 48;
int global_param_cfg::market_min_service_charge      = 0;
int global_param_cfg::market_min_notice_charge       = 0;
int global_param_cfg::mian_bao_fu                    = 0;
int global_param_cfg::xi_lian_lock_cid               = 0;
short global_param_cfg::vip_auto_be_at_lvl           = 30;
char global_param_cfg::vip_buy_add_exp_min_vip_lvl   = 2;
char global_param_cfg::tianfu_skill_upgrade_baoji_rate = 10;
short global_param_cfg::tianfu_lvl_limit             = 100;
int global_param_cfg::tianfu_skill_cd                = 30000;
ilist<int/*equip cid*/> global_param_cfg::birth_equip_list;
int global_param_cfg::bao_shi_jing_hua_cid           = 0;
int global_param_cfg::bao_shi_activate_amt           = 0;
int global_param_cfg::bao_shi_activate_cost          = 0;
char global_param_cfg::bao_shi_pos_hole_amt          = 3;
int global_param_cfg::energy_resume_radius           = 10;
int global_param_cfg::energy_resume_per              = 5;
int global_param_cfg::energy_resume_interval         = 2000;
int global_param_cfg::jing_ji_lvl                    = 32;
int global_param_cfg::jing_ji_times                  = 0;
int global_param_cfg::jing_ji_cd                     = 0;
int global_param_cfg::jing_ji_reset_cost             = 0;
int global_param_cfg::jing_ji_prm[3]                 = {0, 0, 0};
int global_param_cfg::jing_ji_map                    = 0;
int global_param_cfg::ghz_scene_cid                  = 0;
int global_param_cfg::ghz_prepare_time               = 60;
int global_param_cfg::ghz_lv_average                 = 5;
int global_param_cfg::ghz_shop_npc                   = 0;
int global_param_cfg::ghz_activate_shou_wei_cost     = 5;
int global_param_cfg::ghz_over_award[4]              = {0, 0, 0, 0};
int global_param_cfg::ghz_winner_award[4]              = {0, 0, 0, 0};
char *global_param_cfg::ghz_shou_wei_idx             = NULL;
coord_t global_param_cfg::jing_ji_mst;
ilist<pair_t<int> > global_param_cfg::lvl_gift_list;
int global_param_cfg::zx_ten_cost                    = 0;
int global_param_cfg::sao_dang_ka_cid                = 0;
int global_param_cfg::rank_amount                    = 50;
pair_t<int> global_param_cfg::diamond_to_coin[4]     = {pair_t<int>(0, 0), pair_t<int>(0, 0), pair_t<int>(0, 0), pair_t<int>(0, 0)};
int global_param_cfg::tui_tu_perfect_p               = 20;
pair_t<int> global_param_cfg::tui_tu_star_p;
bool global_param_cfg::mail_valid                    = true;
bool global_param_cfg::market_valid                  = true;
bool global_param_cfg::jing_ji_valid                 = true;
ilist<int> global_param_cfg::diamond_not_cs_type_set;
int global_param_cfg::lucky_turn_cost                = 0;
int global_param_cfg::lucky_turn_score               = 0;
int global_param_cfg::lucky_turn_ten_cost            = 0;
int global_param_cfg::lucky_turn_ten_score           = 0;
int global_param_cfg::lucky_turn_big_award_cid       = 0;
pair_t<int> global_param_cfg::lucky_turn_award_diamond;
int global_param_cfg::back_town_cid                  = 0;
int global_param_cfg::tili_recover                   = 0;
int global_param_cfg::wang_zuo_cid                   = 0;
int global_param_cfg::worship_cd                     = 0;
short global_param_cfg::daily_worship_cnt            = 0;
int global_param_cfg::worship_coin_per               = 0;
int global_param_cfg::guild_scp_cid                  = 0;
char global_param_cfg::guild_open_scp_wday           = 6;
int global_param_cfg::guangai_cooling_time           = 0;
int global_param_cfg::guangai_lvl_limit              = 0;
int global_param_cfg::chat_log_cnt                   = 0;
int global_param_cfg::jing_ji_no_cd_times            = 0;
int global_param_cfg::guild_summon_mem_cost          = 0;
ilist<item_amount_bind_t> global_param_cfg::first_recharge_gift;
int global_param_cfg::world_lvl_rank                 = 0;
int global_param_cfg::wei_xin_gift                   = 0;
int global_param_cfg::skill_fight_score_coe          = 0;
int global_param_cfg::open_battle_room               = 0;
int global_param_cfg::over_battle_time               = 0;
int global_param_cfg::battle_scene_cid               = 0;
int global_param_cfg::battle_mst_fresh_time          = 0;
int global_param_cfg::each_battle_number             = 0;
int global_param_cfg::mid_leave_battle_time          = 0;
char global_param_cfg::jingli_value                  = 20;
char global_param_cfg::jingli_cost                   = 2;
short global_param_cfg::jingli_resume_interval       = 900;
short global_param_cfg::jingli_resume_value          = 1;
short global_param_cfg::battle_kill_build_honor      = 0;
short global_param_cfg::battle_kill_barracks_honor   = 0;
short global_param_cfg::battle_kill_tower_honor      = 0;
short global_param_cfg::battle_kill_mst_honor        = 0;
short global_param_cfg::battle_kill_role_honor       = 0;
short global_param_cfg::battle_win_honor             = 0;
short global_param_cfg::battle_draw_honor            = 0;
short global_param_cfg::battle_failure_honor         = 0;
int global_param_cfg::battle_win_gift                = 0;
int global_param_cfg::battle_draw_gift               = 0;
int global_param_cfg::battle_failure_gift            = 0;
char global_param_cfg::min_robbed_percent            = 5;
char global_param_cfg::max_robbed_percent            = 15;
ilist<int> global_param_cfg::robbed_items;
ilist<rob_probability> global_param_cfg::rob_prob_list;
short global_param_cfg::lue_duo_lvl                  = 35;
int global_param_cfg::lue_duo_map                    = 3004;
int global_param_cfg::fabao_recovery_interval        = 5;
pair_t<int> global_param_cfg::water_tree_award_good;
pair_t<int> global_param_cfg::lucky_turn_award_good;
int global_param_cfg::lucky_goods_turn_cnt           = 1;
int global_param_cfg::daily_luck_draw_award_cnt      = 1;
int global_param_cfg::free_relive_cnt                = 10;
int global_param_cfg::baowu_mall_refresh_diamond     = 0;
int global_param_cfg::baowu_mall_npc                 = 0;
ilist<itime> global_param_cfg::baowu_mall_refresh_time;
short global_param_cfg::seven_day_login_open_at_lvl  = 20;
int global_param_cfg::mall_refresh_item              = 0;
int global_param_cfg::mushroom_coin_coe              = 500;

/**
 * @class global_param_cfg_impl
 *
 * @brief implement of global_param_cfg
 */
class global_param_cfg_impl : public load_json
{
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, GLOBAL_PARAM_CFG_PATH, root) != 0)
      return -1;

    int lvl_lmt = ::atoi(root["lvl_limit"].asCString());
    if (global_param_cfg::lvl_limit < lvl_lmt) return -1;
    global_param_cfg::lvl_limit = lvl_lmt;
    global_param_cfg::pk_lvl_limit          = ::atoi(root["pk_lvl_limit"].asCString());
    global_param_cfg::xiangke_param         = ::atoi(root["xiangke_param"].asCString());
    global_param_cfg::hurt_base_val         = ::atoi(root["hurt_base_val"].asCString());
    global_param_cfg::hurt_base_area        = ::atoi(root["hurt_base_area"].asCString());
    global_param_cfg::mst_shanbi_per        = ::atoi(root["mst_shanbi_per"].asCString());
    global_param_cfg::mst_baoji_per         = ::atoi(root["mst_baoji_per"].asCString());
    if (::sscanf(root["birth_coord"].asCString(), "%d:%hd,%hd",
                 &global_param_cfg::birth_coord.cid_,
                 &global_param_cfg::birth_coord.coord_.x_,
                 &global_param_cfg::birth_coord.coord_.y_) != 3)
      return -1;
    global_param_cfg::pkg_cap               = ::atoi(root["pkg_cap"].asCString());
    global_param_cfg::max_pkg_cap           = ::atoi(root["max_pkg_cap"].asCString());
    global_param_cfg::storage_cap           = ::atoi(root["storage_cap"].asCString());
    global_param_cfg::max_storage_cap       = ::atoi(root["max_storage_cap"].asCString());
    global_param_cfg::enlarge_cap_num       = ::atoi(root["enlarge_cap_num"].asCString());
    global_param_cfg::enlarge_cap_cost      = ::atoi(root["enlarge_cap_cost"].asCString());
    if (::sscanf(root["skill_common_cd"].asCString(), "%d,%d,%d",
                 &(global_param_cfg::char_common_cd[CAREER_LI_LIANG]),
                 &(global_param_cfg::char_common_cd[CAREER_MIN_JIE]),
                 &(global_param_cfg::char_common_cd[CAREER_ZHI_LI])) != 3)
      return -1;
    global_param_cfg::send_mail_cost        = ::atoi(root["send_mail_cost"].asCString());
    global_param_cfg::sin_val_decrease_time = ::atoi(root["sin_val_decrease_time"].asCString());
    global_param_cfg::add_sin_val           = ::atoi(root["add_sin_val"].asCString());
    global_param_cfg::sin_val_upper_limit   = ::atoi(root["sin_val_upper_limit"].asCString());
    global_param_cfg::item_coin_cid         = ::atoi(root["item_coin_cid"].asCString());
    global_param_cfg::item_diamond_cid      = ::atoi(root["item_diamond_cid"].asCString());
    global_param_cfg::item_bind_diamond_cid = ::atoi(root["item_bind_diamond_cid"].asCString());
    global_param_cfg::relive_cid            = ::atoi(root["relive_cid"].asCString());
    global_param_cfg::relive_energy_p       = ::atoi(root["relive_energy_p"].asCString());
    global_param_cfg::birth_mission_cid     = ::atoi(root["birth_mission_cid"].asCString());
    global_param_cfg::dropped_item_free_time= ::atoi(root["dropped_item_free_time"].asCString());
    global_param_cfg::dropped_item_destroy_time = ::atoi(root["dropped_item_destroy_time"].asCString());
    global_param_cfg::mall_npc_cid          = ::atoi(root["mall_npc_cid"].asCString());
    global_param_cfg::equip_price_coe       = ::atoi(root["equip_price_coe"].asCString());
    global_param_cfg::hate_val_coe          = ::atoi(root["hate_val_coe"].asCString());
    global_param_cfg::drop_coin_system_recovery = ::atoi(root["drop_coin_system_recovery"].asCString());
    global_param_cfg::create_guild_cost     = ::atoi(root["create_guild_cost"].asCString());
    global_param_cfg::guild_chairman_turn_days = ::atoi(root["guild_chairman_turn_days"].asCString());
    global_param_cfg::guild_one_page_amt    = ::atoi(root["guild_one_page_amt"].asCString());
    global_param_cfg::guild_auto_dismiss_days = ::atoi(root["guild_auto_dismiss_days"].asCString());
    global_param_cfg::guild_create_min_char_lvl = ::atoi(root["guild_lvl_limit"].asCString());
    global_param_cfg::guild_jb_init_cost    = ::atoi(root["guild_jb_init_cost"].asCString());
    global_param_cfg::guild_jb_cost_add     = ::atoi(root["guild_jb_cost_add"].asCString());
    global_param_cfg::guild_zhu_di_cid      = ::atoi(root["guild_zhu_di_cid"].asCString());
    global_param_cfg::guild_juan_xian_contrib_rate = ::atoi(root["guild_juan_xian_contrib_rate"].asCString());
    global_param_cfg::guild_juan_xian_resource_rate = ::atoi(root["guild_juan_xian_resource_rate"].asCString());
    global_param_cfg::guild_summon_boss_wday = ::atoi(root["guild_summon_boss_wday"].asCString());
    global_param_cfg::dxc_enter_cnt         = ::atoi(root["di_xia_cheng_times"].asCString());
    global_param_cfg::chat_lvl_limit        = ::atoi(root["chat_lvl_limit"].asCString());
    global_param_cfg::max_friend_cnt        = ::atoi(root["max_friend_cnt"].asCString());
    global_param_cfg::max_blacklist_cnt     = ::atoi(root["max_blacklist_cnt"].asCString());
    global_param_cfg::tili_val              = ::atoi(root["tili_val"].asCString());
    global_param_cfg::tili_resume_interval  = ::atoi(root["tili_resume_interval"].asCString());
    global_param_cfg::tili_resume_val       = ::atoi(root["tili_resume_val"].asCString());
    global_param_cfg::market_service_charge = ::atoi(root["market_service_charge"].asCString());
    global_param_cfg::market_min_service_charge = ::atoi(root["market_min_service_charge"].asCString());
    global_param_cfg::market_min_notice_charge  = ::atoi(root["market_min_notice_charge"].asCString());
    global_param_cfg::market_one_page_amt   = ::atoi(root["market_one_page_amt"].asCString());
    global_param_cfg::market_item_sale_time = ::atoi(root["market_item_sale_time"].asCString());
    global_param_cfg::mian_bao_fu           = ::atoi(root["mian_bao_fu"].asCString());
    global_param_cfg::xi_lian_lock_cid      = ::atoi(root["wash_lock_cid"].asCString());
    global_param_cfg::vip_auto_be_at_lvl    = ::atoi(root["vip_auto_be_at_lvl"].asCString());
    global_param_cfg::vip_buy_add_exp_min_vip_lvl = ::atoi(root["vip_buy_add_exp_min_vip_lvl"].asCString());
    global_param_cfg::tianfu_skill_upgrade_baoji_rate = ::atoi(root["tianfu_percent"].asCString());
    global_param_cfg::tianfu_lvl_limit      = ::atoi(root["tianfu_lv_limit"].asCString());
    global_param_cfg::tianfu_skill_cd       = ::atoi(root["tianfu_cd"].asCString());
    global_param_cfg::jing_ji_lvl           = ::atoi(root["jing_ji_lvl"].asCString());
    global_param_cfg::jing_ji_times         = ::atoi(root["jing_ji_times"].asCString());
    global_param_cfg::jing_ji_cd            = ::atoi(root["jing_ji_cd"].asCString());
    global_param_cfg::jing_ji_reset_cost    = ::atoi(root["jing_ji_reset_cost"].asCString());
    global_param_cfg::jing_ji_map           = ::atoi(root["jing_ji_map"].asCString());
    global_param_cfg::zx_ten_cost           = ::atoi(root["zx_shi_cost"].asCString());
    global_param_cfg::ghz_scene_cid         = ::atoi(root["ghz_scene_cid"].asCString());
    global_param_cfg::ghz_prepare_time      = ::atoi(root["ghz_prepare_time"].asCString());
    global_param_cfg::ghz_lv_average        = ::atoi(root["ghz_lv_average"].asCString());
    global_param_cfg::ghz_shop_npc          = ::atoi(root["ghz_shop_npc"].asCString());
    global_param_cfg::ghz_activate_shou_wei_cost = ::atoi(root["activate_shou_wei_cost"].asCString());
    global_param_cfg::lucky_turn_big_award_cid = ::atoi(root["lucky_turn_big_award_cid"].asCString());
    global_param_cfg::daily_worship_cnt     = ::atoi(root["mo_bai_count"].asCString());
    global_param_cfg::worship_cd            = ::atoi(root["mo_bai_interval_time"].asCString());
    global_param_cfg::worship_coin_per      = ::atoi(root["mo_bai_xi_shu"].asCString());
    // 为了减少global_param_cfg对其他模块的关联
    if (root["ghz_shou_wei_idx"].empty()
        || ::strlen(root["ghz_shou_wei_idx"].asCString()) == 0
        || ::strlen(root["ghz_shou_wei_idx"].asCString()) > 256)
      return -1;
    if (global_param_cfg::ghz_shou_wei_idx != NULL)
      delete []global_param_cfg::ghz_shou_wei_idx;
    global_param_cfg::ghz_shou_wei_idx = new char[::strlen(root["ghz_shou_wei_idx"].asCString()) + 1];
    ::strcpy(global_param_cfg::ghz_shou_wei_idx, root["ghz_shou_wei_idx"].asCString());

    global_param_cfg::sao_dang_ka_cid       = ::atoi(root["sao_dang_ka_cid"].asCString());
    global_param_cfg::back_town_cid         = ::atoi(root["back_town_cid"].asCString());
    global_param_cfg::rank_amount           = ::atoi(root["rank_amount"].asCString());
    global_param_cfg::tui_tu_perfect_p      = ::atoi(root["perfect_p"].asCString());
    global_param_cfg::tili_recover          = ::atoi(root["tili_recover"].asCString());
    global_param_cfg::wang_zuo_cid          = ::atoi(root["wang_zuo_cid"].asCString());
    global_param_cfg::mail_valid            = ::atoi(root["mail_valid"].asCString()) != 0;
    global_param_cfg::market_valid          = ::atoi(root["market_valid"].asCString()) != 0;
    global_param_cfg::jing_ji_valid         = ::atoi(root["jing_ji_valid"].asCString()) != 0;
    global_param_cfg::guangai_cooling_time  = ::atoi(root["guangai_cooling_time"].asCString());
    global_param_cfg::guild_scp_cid         = ::atoi(root["guild_scp_cid"].asCString());
    global_param_cfg::guild_open_scp_wday   = ::atoi(root["guild_open_scp_wday"].asCString());
    global_param_cfg::guangai_lvl_limit     = ::atoi(root["guangai_lvl_limit"].asCString());
    global_param_cfg::chat_log_cnt          = ::atoi(root["chat_log_cnt"].asCString());
    global_param_cfg::jing_ji_no_cd_times   = ::atoi(root["jing_ji_no_cd_times"].asCString());
    global_param_cfg::guild_summon_mem_cost = ::atoi(root["zhao_huan_cost"].asCString());
    global_param_cfg::world_lvl_rank        = ::atoi(root["world_lvl_rank"].asCString());
    global_param_cfg::wei_xin_gift          = ::atoi(root["wei_xin_gift"].asCString());
    global_param_cfg::skill_fight_score_coe = ::atoi(root["skill_fight_score_coe"].asCString());
    global_param_cfg::open_battle_room      = ::atoi(root["open_battle_room"].asCString());
    global_param_cfg::over_battle_time      = ::atoi(root["over_battle_time"].asCString());
    global_param_cfg::battle_scene_cid      = ::atoi(root["battle_scene_cid"].asCString());
    global_param_cfg::battle_mst_fresh_time = ::atoi(root["battle_mst_fresh_time"].asCString());
    global_param_cfg::each_battle_number    = ::atoi(root["each_battle_number"].asCString());
    global_param_cfg::mid_leave_battle_time = ::atoi(root["mid_leave_battle_time"].asCString());
    global_param_cfg::jingli_value          = ::atoi(root["jingli_val"].asCString());
    global_param_cfg::jingli_cost           = ::atoi(root["rob_jingli_cost"].asCString());
    global_param_cfg::jingli_resume_interval = ::atoi(root["jingli_resume_interval"].asCString());
    global_param_cfg::jingli_resume_value   = ::atoi(root["jingli_resume_val"].asCString());
    global_param_cfg::battle_kill_build_honor     = ::atoi(root["battle_kill_build_honor"].asCString());
    global_param_cfg::battle_kill_barracks_honor  = ::atoi(root["battle_kill_barracks_honor"].asCString());
    global_param_cfg::battle_kill_tower_honor     = ::atoi(root["battle_kill_tower_honor"].asCString());
    global_param_cfg::battle_kill_mst_honor       = ::atoi(root["battle_kill_mst_honor"].asCString());
    global_param_cfg::battle_kill_role_honor      = ::atoi(root["battle_kill_role_honor"].asCString());
    global_param_cfg::battle_win_honor            = ::atoi(root["battle_win_honor"].asCString());
    global_param_cfg::battle_draw_honor           = ::atoi(root["battle_draw_honor"].asCString());
    global_param_cfg::battle_failure_honor        = ::atoi(root["battle_failure_honor"].asCString());
    global_param_cfg::battle_win_gift             = ::atoi(root["battle_win_gift"].asCString());
    global_param_cfg::battle_draw_gift            = ::atoi(root["battle_draw_gift"].asCString());
    global_param_cfg::battle_failure_gift         = ::atoi(root["battle_failure_gift"].asCString());
    global_param_cfg::lue_duo_lvl                 = ::atoi(root["rob_lvl"].asCString());
    global_param_cfg::lue_duo_map                 = ::atoi(root["rob_map"].asCString());
    global_param_cfg::fabao_recovery_interval     = ::atoi(root["fabao_recovery_interval"].asCString());
    global_param_cfg::lucky_goods_turn_cnt        = ::atoi(root["lucky_goods_turntable_cnt"].asCString());
    global_param_cfg::daily_luck_draw_award_cnt   = ::atoi(root["daily_luck_draw_award_cnt"].asCString());
    global_param_cfg::free_relive_cnt             = ::atoi(root["unvip_every_day_free_revive_cnt"].asCString());
    global_param_cfg::baowu_mall_refresh_diamond  = ::atoi(root["baowu_mall_refresh_diamond"].asCString());
    global_param_cfg::baowu_mall_npc              = ::atoi(root["baowu_mall_npc"].asCString());
    global_param_cfg::seven_day_login_open_at_lvl = ::atoi(root["seven_day_login_open_at_lvl"].asCString());
    global_param_cfg::mall_refresh_item           = ::atoi(root["mall_refresh_item"].asCString());
    global_param_cfg::mushroom_coin_coe           = ::atoi(root["mushroom_coin_coe"].asCString());
    if (::sscanf(root["water_tree_award_good"].asCString(), "%d,%d",
                 &(global_param_cfg::water_tree_award_good.first_),
                 &(global_param_cfg::water_tree_award_good.second_))
        != 2
        || global_param_cfg::water_tree_award_good.second_ > 100)
      return -1;
    if (::sscanf(root["wei_xin_shared_award_good"].asCString(), "%d,%d",
                 &(global_param_cfg::lucky_turn_award_good.first_),
                 &(global_param_cfg::lucky_turn_award_good.second_))
        != 2
        || global_param_cfg::lucky_turn_award_good.second_ > 100)
      return -1;
    if (::sscanf(root["ghz_over_award"].asCString(), "%d,%d,%d,%d",
                 &(global_param_cfg::ghz_over_award[0]), // bind diamond
                 &(global_param_cfg::ghz_over_award[1]), // coin
                 &(global_param_cfg::ghz_over_award[2]), // contrib
                 &(global_param_cfg::ghz_over_award[3])) // gift
        != 4)
      return -1;
    if (::sscanf(root["ghz_win_award"].asCString(), "%d,%d,%d,%d",
                 &(global_param_cfg::ghz_winner_award[0]), // bind diamond
                 &(global_param_cfg::ghz_winner_award[1]), // coin
                 &(global_param_cfg::ghz_winner_award[2]), // contrib
                 &(global_param_cfg::ghz_winner_award[3])) // gift
        != 4)
      return -1;
    if (::sscanf(root["star_p"].asCString(), "%d:%d",
                 &(global_param_cfg::tui_tu_star_p.first_),
                 &(global_param_cfg::tui_tu_star_p.second_))
        != 2)
      return -1;

    if (::sscanf(root["jing_ji_prm"].asCString(), "%d,%d,%d",
                 &(global_param_cfg::jing_ji_prm[0]),
                 &(global_param_cfg::jing_ji_prm[1]),
                 &(global_param_cfg::jing_ji_prm[2])) != 3)
      return -1;
    if (::sscanf(root["diamond_to_coin"].asCString(), "%d:%d,%d:%d,%d:%d",
                 &(global_param_cfg::diamond_to_coin[1].first_), &(global_param_cfg::diamond_to_coin[1].second_),
                 &(global_param_cfg::diamond_to_coin[2].first_), &(global_param_cfg::diamond_to_coin[2].second_),
                 &(global_param_cfg::diamond_to_coin[3].first_), &(global_param_cfg::diamond_to_coin[3].second_)) != 6)
      return -1;

    int x = 0, y = 0;
    if (::sscanf(root["jing_ji_entrance"].asCString(), "%d,%d", &x, &y) != 2)
      return -1;
    global_param_cfg::jing_ji_mst.x_ = x;
    global_param_cfg::jing_ji_mst.y_ = y;

    if (::sscanf(root["chat_channel_cd"].asCString(), "%d,%d,%d,%d",
                 &(global_param_cfg::chat_channel_cd[WORLD_CHANNEL]),
                 &(global_param_cfg::chat_channel_cd[GUILD_CHANNEL]),
                 &(global_param_cfg::chat_channel_cd[TEAM_CHANNEL]),
                 &(global_param_cfg::chat_channel_cd[PRIVATE_CHANNEL])) != 4)
      return -1;

    if (::sscanf(root["lucky_turn_cost"].asCString(), "%d,%d",
                 &(global_param_cfg::lucky_turn_cost),
                 &(global_param_cfg::lucky_turn_score)) != 2)
      return -1;
    if (::sscanf(root["lucky_turn_ten_cost"].asCString(), "%d,%d",
                 &(global_param_cfg::lucky_turn_ten_cost),
                 &(global_param_cfg::lucky_turn_ten_score)) != 2)
      return -1;
    if (::sscanf(root["lucky_turn_diamond"].asCString(), "%d,%d",
                 &(global_param_cfg::lucky_turn_award_diamond.first_),
                 &(global_param_cfg::lucky_turn_award_diamond.second_)) != 2)
      return -1;

    {
      int min_rob_per = 0, max_rob_per = 0;
      if (::sscanf(root["resource_cnt"].asCString(), "%d,%d",
                   &min_rob_per, &max_rob_per) != 2)
        return -1;
      global_param_cfg::min_robbed_percent = min_rob_per;
      global_param_cfg::max_robbed_percent = max_rob_per;
    }

    { //
      while (!global_param_cfg::sin_val_drop_param_list.empty()) // for reload
        delete global_param_cfg::sin_val_drop_param_list.pop_front();
      char bf[512] = {0};
      ::strncpy(bf, root["sin_val_info"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        sin_val_drop_param *p = new sin_val_drop_param;
        if (::sscanf(token, "%d~%d:%d:%d~%d:%d~%d",
                     &p->sin_min_, &p->sin_max_,
                     &p->drop_rate_,
                     &p->drop_cnt_min_, &p->drop_cnt_max_,
                     &p->drop_coin_min_, &p->drop_coin_max_) != 7)
          return -1;
        if ((p->sin_max_ < p->sin_min_)
            || (p->drop_cnt_max_ < p->drop_cnt_max_)
            || (p->drop_coin_max_ < p->drop_coin_min_))
          return -1;
        global_param_cfg::sin_val_drop_param_list.push_back(p);
      }
    }
    { //
      global_param_cfg::birth_equip_list.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["birth_equip"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
        global_param_cfg::birth_equip_list.push_back(::atoi(token));
    }
    if (::sscanf(root["capital_relive_coord"].asCString(), "%d:%hd,%hd",
                 &global_param_cfg::capital_relive_coord.cid_,
                 &global_param_cfg::capital_relive_coord.coord_.x_,
                 &global_param_cfg::capital_relive_coord.coord_.y_) != 3)
      return -1;

    if (::sscanf(root["gem_activate"].asCString(), "%d,%d,%d",
                 &global_param_cfg::bao_shi_jing_hua_cid,
                 &global_param_cfg::bao_shi_activate_amt,
                 &global_param_cfg::bao_shi_activate_cost) != 3)
      return -1;

    if (::sscanf(root["energy_resume_pos"].asCString(), "%d:%hd,%hd:%d:%d:%d",
                 &global_param_cfg::energy_resume_pos.cid_,
                 &global_param_cfg::energy_resume_pos.coord_.x_,
                 &global_param_cfg::energy_resume_pos.coord_.y_,
                 &global_param_cfg::energy_resume_radius,
                 &global_param_cfg::energy_resume_per,
                 &global_param_cfg::energy_resume_interval) != 6)
      return -1;

    // lvl gift
    {
      global_param_cfg::lvl_gift_list.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["lvl_gift"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        pair_t<int> pair;
        if (::sscanf(token, "%d:%d", &pair.first_, &pair.second_) != 2)
          return -1;
        global_param_cfg::lvl_gift_list.push_back(pair);
      }
    }
    { //
      global_param_cfg::diamond_not_cs_type_set.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["diamond_not_cs_type"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
        global_param_cfg::diamond_not_cs_type_set.push_back(::atoi(token));
    }
    { //
      global_param_cfg::first_recharge_gift.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["first_charge_gift"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        item_amount_bind_t iab;
        int bind_type = 0;
        if (::sscanf(token, "%d,%d,%d", &iab.cid_, &iab.amount_, &bind_type) != 3)
          return -1;
        iab.bind_ = bind_type;
        global_param_cfg::first_recharge_gift.push_back(iab);
      }
    }

    {
      global_param_cfg::robbed_items.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["rob_items"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        global_param_cfg::robbed_items.push_back(::atoi(token));
      }
    }
    {
      global_param_cfg::rob_prob_list.clear(); // for reload
      char bf[512] = {0};
      ::strncpy(bf, root["rob_coe"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        rob_probability rp;
        if (::sscanf(token, "%hd~%hd:%hd",
                     &rp.min_per_, &rp.max_per_, &rp.prob_) != 3)
          return -1;
        global_param_cfg::rob_prob_list.push_back(rp);
      }
    }
    {
      global_param_cfg::baowu_mall_refresh_time.clear();
      char bf[32] = {0};
      ::strncpy(bf, root["baowu_mall_refresh_time"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ",", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ",", &tok_p))
      {
        itime f_it;
        if (::sscanf(token, "%d:%d", &f_it.hour_, &f_it.min_) != 2)
          return -1;
        global_param_cfg::baowu_mall_refresh_time.push_back(f_it);
      }
    }

    if (global_param_cfg::lvl_limit < 1
        || global_param_cfg::lvl_limit > 200
        || global_param_cfg::xiangke_param < 1
        || global_param_cfg::xiangke_param >= 1000
        || global_param_cfg::hurt_base_val < 1
        || global_param_cfg::hurt_base_val >= 10000
        || global_param_cfg::hurt_base_area < 1
        || global_param_cfg::hurt_base_area >= 10000
        || global_param_cfg::mst_shanbi_per < 0
        || global_param_cfg::mst_shanbi_per >= 10000
        || global_param_cfg::mst_baoji_per < 0
        || global_param_cfg::equip_price_coe <= 0
        || global_param_cfg::hate_val_coe <= 0
        || global_param_cfg::sin_val_decrease_time <= 0
        || global_param_cfg::relive_energy_p <= 0
        || global_param_cfg::tili_resume_interval <= 0
        || global_param_cfg::guild_chairman_turn_days <= 0
        || global_param_cfg::guild_auto_dismiss_days <= 0
        || global_param_cfg::guild_one_page_amt <= 1
        || global_param_cfg::guild_jb_init_cost < 0
        || global_param_cfg::guild_jb_cost_add < 0
        || global_param_cfg::guild_juan_xian_contrib_rate < 0
        || global_param_cfg::guild_juan_xian_resource_rate < 0
        || global_param_cfg::guild_summon_boss_wday < 0
        || global_param_cfg::guild_summon_boss_wday > 7
        || global_param_cfg::market_one_page_amt >= 30
        || global_param_cfg::market_item_sale_time <= 0
        || global_param_cfg::mian_bao_fu <= 0
        || global_param_cfg::vip_buy_add_exp_min_vip_lvl <= 1
        || global_param_cfg::vip_auto_be_at_lvl <= 1
        || global_param_cfg::energy_resume_interval < 500
        || global_param_cfg::energy_resume_radius < 5
        || global_param_cfg::energy_resume_per < 1
        || global_param_cfg::mst_baoji_per >= 10000
        || global_param_cfg::jing_ji_prm[0] <= 0
        || global_param_cfg::jing_ji_prm[1] <= global_param_cfg::jing_ji_prm[0]
        || global_param_cfg::jing_ji_prm[2] <= global_param_cfg::jing_ji_prm[1]
        || (global_param_cfg::jing_ji_prm[0] + global_param_cfg::jing_ji_prm[1]
            + global_param_cfg::jing_ji_prm[2]) != 10
        || global_param_cfg::ghz_prepare_time < 60
        || global_param_cfg::ghz_lv_average < 1
        || global_param_cfg::ghz_lv_average > 20
        || global_param_cfg::rank_amount < 5
        || global_param_cfg::diamond_not_cs_type_set.size() < 3
        || global_param_cfg::wang_zuo_cid == 0
        || (global_param_cfg::lucky_turn_award_diamond.first_
            > global_param_cfg::lucky_turn_award_diamond.second_)
        || global_param_cfg::guangai_lvl_limit < 1
        || global_param_cfg::first_recharge_gift.empty()
        || global_param_cfg::min_robbed_percent < 0
        || global_param_cfg::min_robbed_percent > global_param_cfg::max_robbed_percent
        || global_param_cfg::fabao_recovery_interval <= 0
        || global_param_cfg::lucky_goods_turn_cnt > 100
        || global_param_cfg::daily_luck_draw_award_cnt > 100
        || global_param_cfg::free_relive_cnt > 50
        || global_param_cfg::mushroom_coin_coe <= 0
        )
        return -1;

    return 0;
  }
};
global_param_cfg::global_param_cfg() :
  impl_(new global_param_cfg_impl())
{ }
int global_param_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int global_param_cfg::reload_config(const char *cfg_root)
{ return this->load_config(cfg_root); }
