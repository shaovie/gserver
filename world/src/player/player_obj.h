// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-11-20 23:03
 */
//========================================================================

#ifndef PLAYER_OBJ_H_
#define PLAYER_OBJ_H_

#include "ilist.h"
#include "client.h"
#include "char_info.h"
#include "char_extra_info.h"
#include "account_info.h"
#include "char_obj.h"
#include "global_macros.h"
#include "daily_clean_info.h"
#include "char_recharge.h"

// Lib header
#include <map>

typedef std::map<int/*skill_cid*/, skill_info *> skill_map_t;
typedef std::map<int/*skill_cid*/, skill_info *>::iterator skill_map_itor;

// Forward declarations
class task_data;
class pack_info;
class item_obj;
class item_cfg_obj;
class char_brief_info;
class account_brief_info;
class social_relation;
class trade_info;
class passive_skill_info;
class guild_skill_info;
class mall_buy_log;
class scp_log;
class tui_tu_log;
class tui_tu_cfg_obj;
class vip_info;
class dropped_item;
class cheng_jiu_info;
class bao_shi_info;
class cheng_jiu_data;
class tui_tu_turn_item_info;
class sys_settings;
class huo_yue_du_data;
class char_recharge;
class ltime_recharge_award;
class lueduo_log;
class dxc_info;
class baowu_mall_info;
class kai_fu_act_log;

/**
 * @class player_obj
 *
 * @brief
 */
class player_obj : public client
                   , public char_obj
{
  friend class social_module;
  friend class task_module;
  friend class daily_task_module;
  friend class mail_module;
  friend class trade_module;
  friend class package_module;
  friend class buff_module;
  friend class skill_module;
  friend class passive_skill_module;
  friend class gm_cmd;
  friend class web_service;
  friend class mall_module;
  friend class scp_module;
  friend class tui_tu_module;
  friend class guild_module;
  friend class mis_award_module;
  friend class vip_module;
  friend class cheng_jiu_module;
  friend class title_module;
  friend class bao_shi_module;
  friend class huo_yue_du_module;
  friend class jing_ji_module;
  friend class char_msg_queue_module;
  friend class ghz_module;
  friend class ltime_act_module;
  friend class mobai_module;
  friend class dxc_module;
  friend class attr_module;
  friend class rank_module;
  friend class lue_duo_module;
  friend class kai_fu_act_obj;
public:
  enum
  {
    CLT_NULL                  = 0,
    CLT_TO_GET_ACCOUNT        = 1L << 1,
    CLT_TO_CREATE_ACCOUNT     = 1L << 2,
    CLT_GET_ACCOUNT_OK        = 1L << 3,
    CLT_TO_CREATE_CHAR        = 1L << 4,
    CLT_GET_CHAR_OK           = 1L << 5,
    CLT_TO_LOAD_ALL_DB        = 1L << 6,
    CLT_LOAD_ALL_DB_OK        = 1L << 7,
    CLT_LOGIN_OK              = 1L << 8,
    CLT_ENTER_GAME_OK         = 1L << 9,
  };
  player_obj();

  virtual ~player_obj();

  //= virtual method
  virtual int open(void *arg);

  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_unit_list,
                                  ilist<pair_t<int> > *new_snap_unit_list);
  virtual void do_delivery(mblock *mb);
  void do_delivery(const int msg_id, mblock *mb);
  void do_lose_money(const int value,
                     const int money_type,
                     const int behavior_sub_type,
                     const int src_id_1,
                     const int src_id_2,
                     const int src_id_2_cnt);
  void do_got_money(const int value,
                    const int money_type,
                    const int behavior_sub_type,
                    const int src_id);
  void do_got_item_money(const int cid,
                         const int amount,
                         const int behavior_sub_type,
                         const int src);
  static void do_exchange_item_money(const int cid,
                                     const int amount,
                                     int &coin,
                                     int &diamond,
                                     int &b_diamond);
  void do_calc_attr_affected_by_basic();
  void do_calc_attr_affected_by_equip();
  void do_calc_attr_affected_by_passive_skill();
  void do_calc_attr_affected_by_guild_skill();
  void do_calc_attr_affected_by_bao_shi();
  void do_calc_attr_affected_by_title();
  void do_calc_attr_affected_by_fa_bao();
  int  do_check_item_enough_by_auto_buy(const int item_cid,
                                        const int need_amount,
                                        const char auto_buy);
  int  do_auto_buy_item(const int item_cid, const int amt);
  int  do_transfer_to(const int target_scene_cid,
                      const int target_scene_id,
                      const short target_x,
                      const short target_y);

  void do_notify_attr_to_clt();
  void do_notify_money_to_clt();
  void do_notify_pk_mode_to_clt();
  void do_notify_zhan_xing_v_to_clt();
  void do_notify_goods_lucky_turn_info_to_clt();
  void do_notify_jing_ji_score_to_clt();
  void do_notify_xszc_honor_to_clt();
  void do_notify_fa_bao_info_to_clt();
  void do_add_zhan_xing_value(const int v);
  void do_timing_add_ti_li_value(const int now, const bool when_login);
  void do_add_ti_li(const short value, const bool can_overflow, const bool notify_to_clt);
  void do_reduce_ti_li(const short value);
  void do_exchange_activation_code(const char *content,
                                   const int coin,
                                   const int b_diamond,
                                   ilist<item_amount_bind_t> &item_list);
  void do_timing_add_jing_li_value(const int now, const bool when_login);
  void do_save_fa_bao_info(const int *);

  static void do_build_look_detail_info(player_obj*player,
                                        const int target_id,
                                        ilist<item_obj*> *list,
                                        out_stream &os);
  int  do_enter_scp(const int scene_id, const int scene_cid);
  int  do_timeout(const time_value &now);
public:
  //= inline property method
  virtual int   unit_type()     const { return scene_unit::PLAYER; }
  virtual char  career()        const { return this->char_info_->career_; }
  virtual int   team_id()       const { return this->team_id_; }
  virtual int   master_id()     const { return this->id_; }
  virtual short lvl()           const { return this->char_info_->lvl_; }
  virtual short move_speed()    const { return this->move_speed_; }
  virtual short prev_move_speed() const { return this->prev_move_speed_; }
  virtual void  move_speed(const short v);
  virtual int   attack_target() const { return scene_unit::PLAYER|scene_unit::MONSTER; }
  virtual int   c_time()        const { return this->char_info_->c_time_; }
  void team_id(const int team) { this->team_id_ = team; }
  virtual char  pk_mode() const { return this->char_info_->pk_mode_; }

  int   clt_state()const{ return this->clt_state_; }
  bool  is_reenter_game() { return this->is_reenter_game_; }
  char *account() { return this->account_; }
  char *name()    const { return this->char_info_->name_; }
  int   db_sid()  const { return this->db_sid_; }
  int   sin_val() const { return this->char_info_->sin_val_; }
  int   zhan_li() const { return this->char_info_->zhan_li_; }
  int   out_time()const { return this->char_info_->out_time_; }

  short cur_title() const { return this->char_info_->cur_title_; }
  void  cur_title(const short title_cid) { this->char_info_->cur_title_ = title_cid; }

  int   guild_id() const { return this->guild_id_; }
  void  guild_id(const int v) { this->guild_id_ = v; };
  char  guild_free_jb_cnt() const { return this->daily_clean_info_->guild_free_jb_cnt_; }
  void  guild_free_jb_cnt(const short v) { this->daily_clean_info_->guild_free_jb_cnt_ = v; }
  char  guild_cost_jb_cnt() const { return this->daily_clean_info_->guild_cost_jb_cnt_; }
  void  guild_cost_jb_cnt(const short v) { this->daily_clean_info_->guild_cost_jb_cnt_ = v; }
  short guild_promote_skill_cnt() const { return this->daily_clean_info_->guild_promote_skill_cnt_; }
  void  guild_promote_skill_cnt(const short v) { this->daily_clean_info_->guild_promote_skill_cnt_ = v; }
  char  jing_ji_cnt() const { return this->daily_clean_info_->jing_ji_cnt_; }
  void  jing_ji_cnt(const char v) { this->daily_clean_info_->jing_ji_cnt_ = v; }
  int   jing_ji_score() const { return this->char_extra_info_->jing_ji_score_; }
  int   if_mc_rebate() const { return this->daily_clean_info_->if_mc_rebate_; }
  void  if_mc_rebate(const int v) { this->daily_clean_info_->if_mc_rebate_ = v; }
  int   ti_li() const { return this->char_extra_info_->ti_li_; }
  int   worship_time() const { return this->char_extra_info_->worship_time_; }
  int   worship_cnt() const { return this->daily_clean_info_->worship_cnt_; }
  void  worship_time(const int v) { this->char_extra_info_->worship_time_ = v; }
  void  worship_cnt(const short v) { this->daily_clean_info_->worship_cnt_ = v; }
  int   water_tree_time() const { return this->char_extra_info_->water_tree_time_; }
  void  water_tree_time(const int v) { this->char_extra_info_->water_tree_time_ = v; }

  int  got_ti_li_award_time() const { return this->char_extra_info_->got_ti_li_award_time_; }
  void got_ti_li_award_time(const int v) { this->char_extra_info_->got_ti_li_award_time_ = v; }
  int  check_sys_mail_time() const { return this->char_extra_info_->check_sys_mail_time_; }
  void check_sys_mail_time(const int v) { this->char_extra_info_->check_sys_mail_time_ = v; }
  int  last_buy_mc_time();
  void last_buy_mc_time(const int v);
  int  increase_recharge_cnt(const int rc_type);
  int  yes_or_no() const { return this->char_extra_info_->yes_or_no_; }
  void yes_or_no(const int v) { this->char_extra_info_->yes_or_no_ = v; }
  char dxc_enter_cnt() const { return this->daily_clean_info_->dxc_enter_cnt_; }
  void dxc_enter_cnt(const char v) { this->daily_clean_info_->dxc_enter_cnt_ = v; }
  int  xszc_honor() const { return this->char_extra_info_->xszc_honor_; }
  void xszc_honor(const int v) { this->char_extra_info_->xszc_honor_ = v; }
  virtual char group() const { return this->group_; }
  virtual void group(const char v) { this->group_ = v; }
  short jing_li() const { return this->char_extra_info_->jing_li_; }
  void jing_li(const short v) { this->char_extra_info_->jing_li_ = v; }
  char fa_bao_dj() const { return this->char_extra_info_->fa_bao_dj_; }
  void fa_bao_dj(const char v) { this->char_extra_info_->fa_bao_dj_ = v; }
  char *fa_bao() const { return this->char_extra_info_->fa_bao_; }
  char wt_goods_cnt() const { return this->char_extra_info_->wt_goods_cnt_; }
  void wt_goods_cnt(const char v) { this->char_extra_info_->wt_goods_cnt_ = v; }
  char lucky_turn_goods_cnt() const { return this->char_extra_info_->lucky_turn_goods_cnt_; }
  void lucky_turn_goods_cnt(const char v) { this->char_extra_info_->lucky_turn_goods_cnt_ = v; }
  char daily_goods_lucky_draw_cnt() const { return this->char_extra_info_->daily_goods_lucky_draw_cnt_; }
  void daily_goods_lucky_draw_cnt(const char v) { this->char_extra_info_->daily_goods_lucky_draw_cnt_ = v; }
  char if_got_goods_lucky_draw() const { return this->daily_clean_info_->if_got_goods_lucky_draw_; }
  void if_got_goods_lucky_draw(const char v) { this->daily_clean_info_->if_got_goods_lucky_draw_ = v; }
  char free_relive_cnt() const { return this->daily_clean_info_->free_relive_cnt_; }
  void free_relive_cnt(const char v) { this->daily_clean_info_->free_relive_cnt_ = v; }
  void all_qh_lvl(const short v) { this->all_qh_lvl_ = v; }
  short all_qh_lvl() const { return this->all_qh_lvl_; }
  void sum_bao_shi_lvl(const short v) { this->sum_bao_shi_lvl_ = v; }
  short sum_bao_shi_lvl() const { return this->sum_bao_shi_lvl_; }
  int  total_mstar() const { return this->char_extra_info_->total_mstar_; }
public:
  virtual bool can_ke_zhi(char_obj *defender);
  virtual bool can_be_ke_zhi(char_obj *defender);
  virtual int  can_attack(char_obj * /*target*/);
  int can_transfer_to(const int to_scene_cid);
  bool is_in_my_view(const int target_id);
  int  is_money_enough(const int money_type, const int v);
  bool is_money_upper_limit(const int money_type, const int value);
  int coin() const { return this->char_info_->coin_; }
  int diamond() const { return this->ac_info_->diamond_; }
  int b_diamond() const { return this->char_info_->b_diamond_; }
  int notify_recharge_info();
public:
  //= db message
  void handle_db_get_account_info_result(account_info *);
  void handle_db_get_new_char_info_result(char_info *);
  void handle_db_get_char_info_result(char_info *);
  void handle_db_get_char_extra_info_result(char_extra_info *);
  void handle_db_get_daily_clean_info_result(daily_clean_info *);
  void handle_db_get_skill_list_result(in_stream &is);
  void handle_db_get_passive_skill_list_result(in_stream &is);
  void handle_db_get_mall_buy_log_result(in_stream &is);
  void handle_db_get_baowu_mall_info_result(baowu_mall_info *);
  void handle_db_get_item_single_pkg_result(in_stream &is);
  void handle_db_get_sys_settings_result(sys_settings *ss);
  void handle_db_get_char_recharge_result(char_recharge *);
public:
  //= db
  int db_save_char_info();
  int db_save_char_extra_info();
  int db_save_daily_clean_info();
  int db_save_account_info();
  int db_save_char_recharge();
  int db_save_buff();
protected:
  //
  virtual int  dispatch_msg(const int id,
                            const int res,
                            const char *msg,
                            const int len);

  virtual int  check_package_seq(const int seq);
  virtual void dispatch_aev(const int aev_id, mblock *);
  virtual void dispatch_sev(const int sev_id, mblock *);
private:
  int to_create_account(const char *, const char *);
  int to_create_char(const char *, const char );
  int to_start_game(const int );
  int to_load_all_db_info(const int );
  int to_set_coord_ok();
private:
  int  do_init_char_info(char_info *);
  int  do_init_char_extra_info(char_info *);
  int  do_init_daily_clean_info(char_info *);
  void do_init_birth_equip(char_info *);
  void do_attach_sev();
  void do_deattach_sev();
  void do_build_char_login_show_info(out_stream &os, char_brief_info *cbi);
  void do_release_msg_buff();
  void do_check_fight_status(const int now);
  void do_check_online_award(const int now);
  int  do_login_module_msg(const int msg_id, const char *msg, const int len);
  int  do_basic_module_msg(const int msg_id, const char *msg, const int len);
  int  do_logout();
  int  do_send_client_char_list();
  int  do_ntf_clt_hot_name();
  int  do_kick_out();
  int  do_something_every_1_sec(const int );
  int  do_something_every_3_sec(const int );
  int  do_send_msg_buff_chunk();
  int  do_init_char_info();
  int  do_calc_all_attr();
  int  do_check_clt_heart_beat();
  int  do_got_exp(const int );
  int  do_lvl_up();
  int  do_dead(const int, const int);
  int  do_relive(const char);
  int  do_use_item(item_obj *item, const item_cfg_obj *ico, const int use_amount);

  int  do_scan_snap(const time_value &now);
  int  do_send_snap_slice_list();
  void do_build_snap_base_info(out_stream &);
  void do_build_snap_pos_info(out_stream &os);
  void do_build_snap_status_info(out_stream &os);
  void do_build_snap_equip_info(out_stream &os);
  void do_clear_snap_units();
  void do_broadcast(mblock *mb, const bool to_self);
  void do_post_aev_in_view_area(const int aev_id,
                                mblock *ev_mb,
                                const int include_unit_type);
  int  do_buy_item(const int npc_cid, const int item_cid, const int amt, const int param = 0);
  int  do_sell_item(const int pos, const int amt);
  int  do_buy_and_use_item(const item_cfg_obj* ico,
                           const int npc_cid,
                           const int cost,
                           const int price_type,
                           const int group_id,
                           const int total_amt,
                           const int item_cid,
                           const int amt,
                           const char bind);
  void do_decrease_sin(const int );

  void do_exit_scp();
  void do_scp_end();
  void do_give_scp_award();
  void do_clear_scp_award();
  void do_exit_tui_tu_scp();
  void do_notify_scp_award_to_clt();
  int  do_enter_tui_tu_scp(const int scene_cid, const tui_tu_cfg_obj *ttco);
  void do_resume_last_position();
  void do_reset_position(const int cid, const short x, const short y);
  void do_resume_energy(const time_value &now);
  void do_record_lose_diamond(const int, const int);
private:
  void on_transfer_scene(const short old_x,
                         const short old_y,
                         const int old_scene_id,
                         const int old_scene_cid);
  void on_get_account_info_ok(account_brief_info *abi);
  void on_load_all_db_info_ok();
  void on_char_login();
  void on_new_day(const bool, const int);
  void on_after_login();
  void on_enter_game();
  void on_char_logout();
  void on_got_money(const int /*money_type*/) { };
  void on_use_skill_ok(skill_info *, const skill_detail *, const time_value &);
  void on_item_used_ok(item_obj *, item_cfg_obj *, const int );
  void on_kill_somebody(const int char_id);
  void on_buy_item_ok(const int npc_cid, const int item_cid, const int amt, const int param = 0);
public:
  virtual void on_attr_update(const int , const int);
  virtual void on_attack_somebody(const int /*target_id*/,
                                  const int obj_type,
                                  const time_value &/*now*/,
                                  const int /*skill_cid*/,
                                  const int /*real_hurt*/);
  virtual void on_be_attacked(char_obj * /*attacker*/);
  virtual void on_add_buff(buff_obj *bo);
  virtual void on_remove_buff(buff_obj *bo);
  virtual void on_modify_buff(buff_obj *bo);

  void on_exit_tui_tu_scp();
  void on_clt_kill_mst(const int mst_cid, const short x, const short y, const int msg_id, bool &);
private:
  //= client message
  int  clt_hello_world(const char *msg, const int len);
  int  clt_create_char(const char *msg, const int len);
  int  clt_start_game(const char *msg, const int len);
  int  clt_enter_game(const char *msg, const int len);
  int  clt_reenter_game(const char *msg, const int len);
  int  clt_heart_beat(const char *msg, const int len);
  int  clt_gm_cmd(const char *msg, const int len);
  int  clt_move(const char *msg, const int len);
  int  clt_use_skill(const char *msg, const int len);
  int  clt_use_item(const char *msg, const int len);
  int  clt_relive(const char *msg, const int len);
  int  clt_look_other_detail_info(const char *msg, const int len);
  int  clt_exit_transfer(const char *msg, const int len);
  int  clt_takeon_equip(const char *msg, const int len);
  int  clt_takedown_equip(const char *msg, const int len);
  int  clt_buy_item(const char *msg, const int len);
  int  clt_buy_and_use_item(const char *msg, const int len);
  int  clt_diamond_to_coin(const char *msg, const int len);
  int  clt_sell_item(const char *msg, const int len);
  int  clt_pick_up_item(const char *msg, const int len);
  int  clt_wild_boss_info(const char *msg, const int len);
  int  clt_change_pk_mode(const char *msg, const int len);
  int  clt_obtain_mall_buy_log(const char *msg, const int len);
  int  clt_obtain_baowu_mall_info(const char *msg, const int len);
  int  clt_refresh_baowu_mall(const char *msg, const int len);
  int  clt_zhan_xing(const char *msg, const int len);
  int  clt_zhan_xing_ten(const char *msg, const int len);
  int  clt_get_title_list(const char *msg, const int len);
  int  clt_use_title(const char *msg, const int len);
  int  clt_cancel_cur_title(const char *msg, const int len);
  int  clt_modify_sys_settings(const char *msg, const int len);
  int  clt_get_huo_yue_du_info(const char *msg, const int len);
  int  clt_get_huo_yue_du_award(const char *msg, const int len);
  int  clt_sync_hp_mp(const char *msg, const int len);
  int  clt_get_hot_name(const char *msg, const int len);
  int  clt_obtain_svc_time(const char *msg, const int len);
  int  clt_obtain_wild_boss_info(const char *msg, const int len);
  int  clt_direct_transfer(const char *msg, const int len);
  int  clt_get_trade_no(const char *msg, const int len);
  int  clt_produce_item(const char *msg, const int len);
  int  clt_enter_scene_ok(const char *msg, const int len);
  int  clt_back_town(const char *msg, const int len);
  int  clt_buy_ti_li(const char *msg, const int len);
  int  clt_get_first_recharge_award(const char *msg, const int len);
  int  clt_goods_award_wei_xin_shared_ok(const char *msg, const int len);
  int  clt_lucky_goods_turn(const char *msg, const int len);

  int  clt_test_echo(const char *msg, const int len);
private:
  int  can_use_skill_i(const skill_detail* sd,
                       const skill_info *si,
                       const int target_id,
                       const time_value &now,
                       const coord_t &);
  int can_use_item(item_obj *item, item_cfg_obj *ico, const int use_amount);
  int can_buy_item(const int , const int , const int , const int param = 0);
public:
  void broadcast_lvl_up();
  void broadcast_equip_fino();
  void broadcast_sin_val();
  void broadcast_guild_info();
  void broadcast_vip_info();
  void broadcast_cur_title();
  void broadcast_water_tree_info();
  void broadcast_fa_bao_info();
  void broadcast_all_qh_lvl();
  void broadcast_sum_bao_shi_lvl();
  virtual void broadcast_position();
  virtual void broadcast_speed();
  virtual void broadcast_hp_mp();
  virtual void broadcast_use_skill(const int skill_cid,
                                   const short skill_lvl,
                                   const int target_id,
                                   const short x,
                                   const short y);
  virtual void broadcast_be_hurt_effect(const int skill_cid,
                                        const int hurt,
                                        const int tip,
                                        const int attacker_id);
  virtual void broadcast_unit_life_status(const char st);
  virtual void broadcast_add_buff(buff_obj *bo);
  virtual void broadcast_del_buff(buff_obj *bo);
  virtual void broadcast_tianfu_skill(const int skill_cid);
private:
  // async event
  void aev_on_new_day(mblock *);
  void aev_got_exp(mblock *);
  void aev_be_killed(mblock *);
  void aev_got_money(mblock *);
  void aev_lose_money(mblock *);
  void aev_kill_monster(mblock *);
  void aev_kill_somebody(mblock *);
  void aev_zhuzai_fen_shen(mblock *);
  void aev_zhuzai_fen_shen_destroy(mblock *);
  void aev_kick_from_scp(mblock *);
  void aev_notify_new_mail(mblock *);
  void aev_transfer_to(mblock *);
  void aev_ghz_over_award(mblock *);
  void aev_direct_send_msg(mblock *);
private:
  // sync event
  void sev_shutdown(mblock *);
private:
  bool  is_reenter_game_;
  bool  enter_scene_ok_;
  bool  notify_jing_ji_;
  bool  has_resource_; // for client
  char  group_;
  short move_speed_;
  short prev_move_speed_;
  short all_qh_lvl_;
  short sum_bao_shi_lvl_;
  int  db_sid_;
  int  clt_state_;
  int  team_id_;
  int  guild_id_;

  int  last_check_1_sec_time_;
  int  last_check_3_sec_time_;
  int  last_clt_heart_beat_time_;
  int  last_fight_time_;
  int  scp_award_exp_;
  int  enter_scp_time_;

  account_info *ac_info_;
  char_info *char_info_;
  char_extra_info *char_extra_info_;
  daily_clean_info *daily_clean_info_;
  char_recharge *char_recharge_;
  baowu_mall_info *baowu_mall_info_;

  mblock *msg_buff_chunk_;
  ilist<mblock *> msg_buff_chunk_list_;

  time_value next_scan_snap_time_;
  time_value next_resume_energy_time_;

  ilist<mblock *> snap_slice_list_;
  ilist<pair_t<int> > *old_snap_unit_list_;  // first: id, second:cid
  ilist<pair_t<int> > *new_snap_unit_list_;

  social_relation *social_relation_;
  pack_info *pack_info_;
  task_data *task_data_;
  trade_info *trade_info_;
  vip_info *vip_info_;
  ilist<bao_shi_info *> bao_shi_list_;
  cheng_jiu_data *cheng_jiu_data_;
  huo_yue_du_data *huo_yue_du_data_;
  ilist<int/*mail_id*/> get_system_mail_list_;
  skill_map_t skill_map_;

  ilist<passive_skill_info *> passive_skill_list_;
  ilist<guild_skill_info *> guild_skill_list_;
  typedef std::map<int/*effect id*/, time_value> item_used_time_map_t;
  typedef std::map<int/*effect id*/, time_value>::iterator item_used_time_map_itor;
  item_used_time_map_t item_used_time_map_;

  ilist<mall_buy_log *> mall_buy_list_;
  ilist<int> zhuzai_fen_shen_;

  ilist<scp_log *> scp_log_;
  ilist<item_amount_bind_t> scp_award_items_;

  ilist<tui_tu_log *> tui_tu_log_;
  ilist<tui_tu_turn_item_info *> tui_tu_turn_item_list_;
  ilist<dropped_item *> tui_tu_dropped_item_list_;

  ilist<lueduo_log *> lueduo_log_list_;

  ilist<dxc_info *> dxc_info_list_;

  ilist<ltime_recharge_award *> ltime_recharge_award_list_;
  ilist<kai_fu_act_log *> kai_fu_act_log_;

  ilist<short/*title_cid*/>  title_list_;

  char account_[MAX_ACCOUNT_LEN + 1];
};
#include "player_obj.inl"
#endif // PLAYER_OBJ_H_

