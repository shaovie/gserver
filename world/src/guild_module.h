// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-23 21:08
 */
//========================================================================

#ifndef GUILD_MODULE_H_
#define GUILD_MODULE_H_

#include "ilist.h"

// Forward declarations
class guild_info;
class guild_member_info;
class guild_apply_info;
class player_obj;
class in_stream;
class out_stream;
class char_brief_info;
class guild_skill_info;
class mblock;

/**
 * @class guild_module
 *
 * @brief
 */
class guild_module
{
  friend class preload_db;
public:
  static void destroy(player_obj *player);
  static int  handle_db_get_guild_skill_list_result(player_obj *player, in_stream &is);
  static int  dispatch_msg(player_obj *, const int , const char *, const int );
  static char *get_guild_name(const int guild_id);
  static int  get_guild_zhan_li(const int guild_id);
  static int  get_guild_id(const int char_id);
  static int  get_ghz_winner();
  static int  get_ghz_winner_chairman();
  static int  get_chairman(const int guild_id);
  static char get_char_guild_pos(const int guild_id, const int char_id);
  static void get_guild_list_before_rank(ilist<int> &id_list, const int n);
  static void get_member_list(const int guild_id, ilist<int> &);
  static void on_new_day();
  static void on_char_login(player_obj *);
  static void on_enter_game(player_obj *);
  static void on_join_guild(const int char_id, const int guild_id, const bool create_guild);
  static void on_exit_guild(const int char_id, const int guild_id);
  static void on_ghz_over(const int win_guild_id);
  static void on_guild_demise(const int chairman_id);
  static void on_close_scp(const int guild_id);

  static void broadcast_to_guild(const int guild_id, const int msg_id, const int, mblock *mb);
public:
  static void do_insert_guild(guild_info *info);
  static void do_insert_guild_member(guild_member_info *info);
  static void do_insert_guild_apply(guild_apply_info *info);
  static void do_fetch_char_guild_info(const int guild_id, out_stream &os);
  static void do_got_contrib(const int guild_id, const int char_id, const int v);
  static void do_notify_contrib_to_clt(player_obj *player, const guild_member_info *info);
  static void do_got_resource(const int guild_id, const int v);
  static void do_notify_resource_to_clt(player_obj *player, const guild_info *info);
  static void do_broadcast_apply_to_clt(const int guild_id);
  static void do_notify_apply_to_clt(player_obj *player, const int amt);
  static void do_notify_guild_scp_open(const int guild_id, const char difficulty);
  static void do_notify_guild_scp_over(const int guild_id, const char result);
private:
  static int  do_create_guild(const int char_id, const char *name);
  static void do_destory_guild(const int guild_id);
  static void do_create_guild_member(const int guild_id, const int char_id);
  static void do_destory_guild_member(const int guild_id, const int char_id);
  static void do_create_guild_apply(const int guild_id, const int char_id);
  static void do_destory_guild_apply(const int guild_id, const int char_id);
  static void do_destory_guild_apply_by_guild(const int guild_id);
  static void do_sort_guild();
  static void do_fetch_char_info(char_brief_info *info, out_stream &os);
  static void do_fetch_chairman_info(char_brief_info *info, out_stream &os);
  static void do_check_chairman_turn();
  static void do_check_no_mem_login_dissmis_guild();
  static int  do_demise_chairman(player_obj *player, const int tar_id);
  static void do_guild_up_building(player_obj *player, const char buliding_type);
  static void do_update_guild_2_db(const guild_info *info);
  static void do_update_guild_member_2_db(const guild_member_info *info);
  static void do_build_guild_skill_list(player_obj *player, out_stream &os);
  static void do_promote_guild_skill(player_obj *player, const int skill_cid, const short skill_lvl);
  static guild_skill_info *to_find_guild_skill(player_obj *player, const int skill_cid);
  static void do_broadcast_mem_join_or_exit_to_guild(const int guild_id,
                                                     const int char_id,
                                                     const char *name,
                                                     const char join_or_exit);
  static char to_get_scp_doing(const int guild_id, const int char_id);
  static void do_notify_guild_pos_to_clt(const int char_id, const char pos);
private:
  static void on_new_guild(const int guild_id);
  static void on_delete_guild(const int guild_id);
  static void on_got_contrib(player_obj *, const int cur_value);
private:
  static int clt_guild_create(player_obj *, const char *, const int);
  static int clt_guild_list(player_obj *, const char *, const int);
  static int clt_guild_apply(player_obj *, const char *, const int);
  static int clt_guild_apply_list(player_obj *, const char *, const int);
  static int clt_guild_reply(player_obj *, const char *, const int);
  static int clt_guild_reply_all(player_obj *, const char *, const int);
  static int clt_guild_mem_exit(player_obj *, const char *, const int);
  static int clt_guild_member_list(player_obj *, const char *, const int);
  static int clt_guild_apply_limit_set(player_obj *, const char *, const int);
  static int clt_guild_mem_pos_set(player_obj *, const char *, const int);
  static int clt_guild_expel_mem(player_obj *, const char *, const int);
  static int clt_guild_info(player_obj *, const char *, const int);
  static int clt_guild_purpose_set(player_obj *, const char *, const int);
  static int clt_guild_up_building(player_obj *, const char *, const int);
  static int clt_guild_juan_xian(player_obj *, const char *, const int);
  static int clt_guild_summon_boss(player_obj *, const char *, const int);
  static int clt_guild_ju_bao(player_obj *, const char *, const int);
  static int clt_guild_enter_zhu_di(player_obj *, const char *, const int);
  static int clt_guild_skill_list(player_obj *, const char *, const int);
  static int clt_guild_promote_skill(player_obj *, const char *, const int);
  static int clt_enter_ghz(player_obj *, const char *, const int);
  static int clt_ghz_activate_shou_wei(player_obj *, const char *, const int);
  static int clt_ghz_obtain_fighting_info(player_obj *, const char *, const int);
  static int clt_start_guild_scp(player_obj *player, const char *, const int );
  static int clt_enter_guild_scp(player_obj *player, const char *, const int );
  static int clt_summon_guild_mem(player_obj *player, const char *, const int );
  static int clt_transfer_by_guild_summon(player_obj *player, const char *, const int );
};
#endif // GUILD_MODULE_H_
