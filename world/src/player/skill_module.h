// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-30 20:23:00
 */
//========================================================================

#ifndef SKILL_MODULE_H_
#define SKILL_MODULE_H_

// Forward declarations
class player_obj;
class in_stream;
class out_stream;
class skill_info;

/**
 * @class skill_module
 * 
 * @brief
 */
class skill_module
{
public:
  static int handle_db_get_skill_list_result(player_obj *player, in_stream &);
  static int handle_db_get_other_skill_list_result(player_obj *player, in_stream &);
  static int db_insert_skill_info(player_obj *player, skill_info *si);
  static int db_update_skill_info(player_obj *player, skill_info *si);

  static int *get_common_skills(const int career);

  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void on_enter_game(player_obj *player);
  static void on_char_lvl_up(player_obj *); 

  static void do_learn_skill(player_obj *player, const int skill_cid);
private:
  static int  clt_learn_skill(player_obj *, const char *, const int );
  static int  clt_upgrade_skill(player_obj *, const char *, const int );
  static void do_build_skill_info(skill_info *si, out_stream &os);
  static void do_notify_skills_can_upgrade(player_obj *);
  static void do_learn_skills(player_obj *player);
};

#endif // TEAM_MODULE_H_

