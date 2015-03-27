// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-05 14:44
 */
//========================================================================

#ifndef PASSIVE_SKILL_MODULE_H_
#define PASSIVE_SKILL_MODULE_H_

// Forward declarations
class in_stream;
class out_stream;
class player_obj;
class passive_skill_info;
class passive_skill_cfg_obj;

/**
 * @class passive_skill_module
 *
 * @brief
 */
class passive_skill_module
{
public:
  static void destroy(player_obj *);
  static int handle_db_get_passive_skill_list_result(player_obj *player, in_stream &);
  static int db_update_skill_info(player_obj *player, passive_skill_info *si);

  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *player);

  static void do_learn_skill(player_obj *player, const int skill_cid);

  static passive_skill_info *get_skill(player_obj *, const int skill_cid);
  static int get_pskill_all_lvl(player_obj *);
private:
  static int  clt_upgrade_skill(player_obj *, const char *, const int );
  static int  clt_one_key_upgrade_skill(player_obj *player, const char *msg, const int len);
  static int  do_one_key_upgrade_skill(player_obj *player, const int skill_cid, const char auto_buy);
  static int  do_upgrade_skill(player_obj *player,
                               const passive_skill_cfg_obj *psco,
                               const char auto_buy,
                               const int skill_cid,
                               int &cost_item_cid,
                               int &cost_coin_amt,
                               int &total_cost_cai_liao_amt,
                               int &cost_cai_liao_amt,
                               int &auto_buy_cai_liao_amt,
                               int &cur_up_lvl,
                               int &new_up_lvl,
                               char &result);
  static void do_build_skill_info(passive_skill_info *si, out_stream &os);
  static void do_notify_skills_can_learn_or_upgrade(player_obj *);
  static void on_upgrade_over(player_obj *, passive_skill_info *psi);
};

#endif // PASSIVE_SKILL_MODULE_H_

