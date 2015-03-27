#ifndef AI_FUNC_FUNC_
#define AI_FUNC_FUNC_

#include "time_value.h"

class player;

#define MAX_GM_STR          100

class ai_struct
{
public:
  ai_struct()
    : param_(0),
    done_cnt_(0)
  { }

  int param_;
  int done_cnt_;
  time_value do_time_;
};

class ai_func
{
public:
  static int do_heart_beat(player *, const time_value &now, ai_struct &ai);
  static int do_move(player *, const time_value &now, ai_struct &ai);
  static int do_move_to(player *, const time_value &now, ai_struct &ai);
  static int do_add_remove_socialer(player *, const time_value &now, ai_struct &ai);
  static int do_team(player *, const time_value &now, ai_struct &ai);
  static int do_add_item(player *, const time_value &now, ai_struct &ai);
  static int do_look_other(player *, const time_value &now, ai_struct &ai);
  static int do_market(player *, const time_value &now, ai_struct &ai);
  static int do_sort_package(player *, const time_value &now, ai_struct &ai);
  static int do_meridians_xiulian(player *, const time_value &now, ai_struct &ai);
  static int do_modify_hp_mp(player *, const time_value &now, ai_struct &ai);
  static int do_use_item(player *, const time_value &now, ai_struct &ai);
  static int do_let_pet_out(player *, const time_value &now, ai_struct &ai);
  static int do_add_pet(player *, const time_value &now, ai_struct &ai);
  static int do_enter_scp(player *, const time_value &now, ai_struct &ai);
  static int do_select_career(player *, const time_value &now, ai_struct &ai);
  static int do_add_skill(player *, const time_value &now, ai_struct &ai);
  static int do_use_skill(player *, const time_value &now, ai_struct &ai);
  static int do_relive(player *, const time_value &now, ai_struct &ai);
  static int do_add_monster(player *, const time_value &now, ai_struct &ai);
  static int do_add_money(player *, const time_value &now, ai_struct &ai);
  static int do_get_ol_gift(player *, const time_value &, ai_struct &ai);
  static int do_get_scp_zhaohui_exp(player *, const time_value &, ai_struct &ai);
  static int do_add_exp(player *, const time_value &, ai_struct &ai);
  static int do_lvl_up(player *, const time_value &, ai_struct &ai);
  static int do_chat(player *, const time_value &, ai_struct &);
  static int do_mpfull(player *, const time_value &, ai_struct &);
};

#endif  // AI_FUNC_FUNC_
