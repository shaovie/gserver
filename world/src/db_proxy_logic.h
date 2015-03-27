// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-06 11:16
 */
//========================================================================

#ifndef DB_PROXY_LOGIC_H_
#define DB_PROXY_LOGIC_H_

#include "db_proxy.h"

// Forward declarations

/**
 * @class db_proxy_logic
 *
 * @brief
 */
class db_proxy_logic : public db_handler
{
public:
  virtual int dispatch_msg(const int , const int , const char *, const int );
private:
  int handle_get_account_info_result(const char *msg, const int len, const int res);
  int handle_insert_account_info_err_result(const char *msg, const int len, const int res);
  int handle_get_sys_settings_result(const char *msg, const int len, const int res);
  int handle_get_new_char_info_result(const char *msg, const int len, const int res);
  int handle_get_char_info_result(const char *msg, const int len, const int res);
  int handle_char_info_err_result(const char *msg, const int len, const int res);

  int handle_get_char_extra_info_result(const char *msg, const int len, const int res);
  int handle_char_extra_info_err_result(const char *msg, const int len, const int res);

  int handle_get_daily_clean_info_result(const char *msg, const int len, const int res);
  int handle_daily_clean_info_err_result(const char *msg, const int len, const int res);

  int handle_get_social_list_result(const char *msg, const int len, const int res);
  int handle_get_search_list_result(const char *msg, const int len, const int res);
  int handle_social_err_result(const char *msg, const int len, const int res);

  int handle_get_char_msg_list_result(const char *msg, const int len, const int res);
  int handle_char_msg_queue_err_result(const char *msg, const int len, const int res);

  int handle_get_task_list_result(const char *msg, const int len, const int res);
  int handle_task_info_err_result(const char *msg, const int len, const int res);

  int handle_get_task_bit_arrary(const char *msg, const int len, const int res);
  int handle_task_bit_array_err_result(const char *msg, const int len, const int res);

  int handle_get_daily_task_list_result(const char *msg, const int len, const int res);
  int handle_daily_task_err_result(const char *msg, const int len, const int res);

  int handle_get_skill_list_result(const char *msg, const int len, const int res);
  int handle_get_other_skill_list_result(const char *msg, const int len, const int res);
  int handle_skill_err_result(const char *msg, const int len, const int res);

  int handle_get_passive_skill_list_result(const char *msg, const int len, const int res);
  int handle_passive_skill_err_result(const char *msg, const int len, const int res);

  int handle_get_tianfu_skill_list_result(const char *msg, const int len, const int res);
  int handle_tianfu_skill_err_result(const char *msg, const int len, const int res);

  int handle_get_item_single_pkg_result(const char *msg, const int len, const int res);
  int handle_cross_get_item_single_pkg_result(const char *msg, const int len, const int res);
  int handle_get_castellan_item_single_pkg(const char *msg, const int len, const int res);
  int handle_get_item_list_result(const char *msg, const int len, const int res);
  int handle_item_err_result(const char *msg, const int len, const int res);

  int handle_get_buff_list_result(const char *msg, const int len, const int res);
  int handle_buff_err_result(const char *msg, const int len, const int res);

  int handle_get_tui_tu_log_result(const char *msg, const int len, const int res);
  int handle_tui_tu_err_result(const char *msg, const int len, const int res);

  int handle_get_scp_log_result(const char *msg, const int len, const int res);
  int handle_scp_err_result(const char *msg, const int len, const int res);

  int handle_mail_err_result(const char *msg, const int len, const int res);
  int handle_get_mail_list(const char *msg, const int len, const int res);
  int handle_get_mail_info(const char *msg, const int len, const int res);
  int handle_check_new_mail(const char *msg, const int len, const int res);
  int handle_get_mail_attach(const char *msg, const int len, const int res);
  int handle_get_system_mail_list(const char *msg, const int len, const int res);

  int handle_get_mall_buy_log(const char *msg, const int len, const int res);
  int handle_get_baowu_mall_info(const char *msg, const int len, const int res);

  int handle_get_guild_skill_list_result(const char *msg, const int len, const int res);
  int handle_guild_err_result(const char *msg, const int len, const int res);

  int handle_get_vip_result(const char *msg, const int len, const int res);
  int handle_vip_err_result(const char *msg, const int len, const int res);

  int handle_get_cheng_jiu_result(const char *msg, const int len, const int res);
  int handle_cheng_jiu_err_result(const char *msg, const int len, const int res);

  int handle_get_title_result(const char *msg, const int len, const int res);
  int handle_title_err_result(const char *msg, const int len, const int res);

  int handle_get_bao_shi_result(const char *msg, const int len, const int res);
  int handle_bao_shi_err_result(const char *msg, const int len, const int res);

  int handle_get_huo_yue_du_result(const char *msg, const int len, const int res);
  int handle_huo_yue_du_err_result(const char *msg, const int len, const int res);

  int handle_get_jing_ji_log_result(const char *msg, const int len, const int res);
  int handle_jing_ji_log_err_result(const char *msg, const int len, const int res);

  int handle_get_char_recharge_result(const char *msg, const int len, const int res);
  int handle_char_recharge_err_result(const char *msg, const int len, const int res);

  int handle_get_lueduo_log_result(const char *msg, const int len, const int res);
  int handle_lueduo_err_result(const char *msg, const int len, const int res);

  int handle_get_dxc_info_result(const char *msg, const int len, const int res);
  int handle_dxc_err_result(const char *msg, const int len, const int res);

  int handle_get_lucky_turn_big_award_list_result(const char *msg, const int len, const int res);

  int handle_get_ltime_recharge_award_result(const char *msg, const int len, const int res);
  int handle_ltime_recharge_award_err_result(const char *msg, const int len, const int res);

  int handle_get_kai_fu_act_log_result(const char *msg, const int len, const int res);
  int handle_kai_fu_act_log_err_result(const char *msg, const int len, const int res);

  int handle_get_db_proxy_payload_result(const char *msg, const int len, const int res);
};

#endif // DB_PROXY_LOGIC_H_

