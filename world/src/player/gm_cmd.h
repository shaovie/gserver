// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-10-23 10:00
 */
//========================================================================

#ifndef GM_CMD_H_
#define GM_CMD_H_

class player_obj;

/**
 * @class gm_cmd
 *
 * @brief
 */
class gm_cmd
{
public:
  //= from input
  static void add_exp(player_obj *player, const char *params[], const int number); 
  static void add_skill(player_obj *player, const char *params[], const int param_cnt);
  static void add_monster(player_obj *player, const char *params[], const int param_cnt);
  static void add_item(player_obj *player, const char *params[], const int param_cnt);
  static void add_money(player_obj *player, const char *params[], const int param_cnt);
  static void accept_task(player_obj *player, const char *params[], const int param_cnt);
  static void submit_task(player_obj *player, const char *params[], const int param_cnt);
  static void move(player_obj *player, const char *params[], const int param_cnt);
  static void switch_pk(player_obj *player, const char *params[], const int param_cnt);
  static void add_sin(player_obj *player, const char *params[], const int param_cnt);
  static void add_title(player_obj *player, const char *params[], const int param_cnt);
  static void copy_self(player_obj *player, const char *params[], const int param_cnt);
  static void clear_scp(player_obj *player, const char *params[], const int param_cnt);
  static void enter_scp(player_obj *player, const char *params[], const int param_cnt);
  static void gm(player_obj *player, const char *params[], const int param_cnt);
  static void mpfull(player_obj *player, const char *params[], const int param_cnt);
};

#endif // GM_CMD_H_
