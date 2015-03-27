// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-30 20:11
 */
//========================================================================

#ifndef PRELOAD_DB_H_
#define PRELOAD_DB_H_

// Forward declarations
class mysql_db;

/**
 * @class preload_db
 *
 * @brief
 */
class preload_db
{
public:
  static int do_preload_db();
private:
  static int preload_service_info(mysql_db *);
  static int preload_account_info(mysql_db *);
  static int preload_recharge_log(mysql_db *);
  static int preload_consume_log(mysql_db *);
  static int preload_char_info(mysql_db *);
  static int preload_char_extra_info(mysql_db *);
  static int preload_char_attr(mysql_db *);
  static int preload_social_info(mysql_db *);
  static int preload_guild_info(mysql_db *);
  static int preload_guild_member_info(mysql_db *db);
  static int preload_guild_apply_info(mysql_db *db);
  static int preload_max_market_id(mysql_db *db);
  static int preload_market_info(mysql_db *db);
  static int preload_tui_tu_log(mysql_db *db);
  static int preload_forbid_opt(mysql_db *db);
  static int preload_vip_info(mysql_db *db);
  static int preload_notice_info(mysql_db *db);
  static int preload_jing_ji_rank(mysql_db *db);
  static int preload_ltime_act(mysql_db *db);
  static int preload_lucky_turn(mysql_db *db);
  static int preload_lucky_turn_score(mysql_db *db);
  static int preload_activation_code(mysql_db *db);
  static int preload_worship_info(mysql_db *db);
  static int preload_water_tree(mysql_db *db);
  static int preload_tianfu_skill(mysql_db *db);
  static int preload_lueduo_item(mysql_db *db);
};

#endif // PRELOAD_DB_H_

