#ifndef PLAYER_H_
#define PLAYER_H_

#include "ilist.h"
#include "client.h"
#include "skill_info.h"
#include "global_macros.h"
#include "scene_unit_obj.h"

#include <map>

class ai_struct;
class time_value;
class in_stream;
class social_relation;
class team_info;
class item_obj;
class scene_unit_obj;

class player : public client 
               , public scene_player
{
  typedef client supper;
public:
  player(const char *ac, const char *name);
  ~player();
  virtual int open(void *);
  virtual int handle_timeout(const time_value & /*now*/);
  
  int do_login();
  int handle_login_result(const char *msg, const int len, const int res);
  int do_create_char();
  int handle_create_char_result(const char *msg, const int len, const int res);
  int do_start_game();
  int handle_start_game_result(const char *msg, const int len, const int res);
  int do_enter_game();
  int handle_resp_heart_beat(const char *msg, const int len, const int res);
  int handle_scene_snap_update(const char *msg, const int len, const int res);
  int handle_other_char_move(const char *msg, const int len, const int );
  int handle_char_move(const char *msg, const int len, const int );
  int handle_unit_life_status(const char *msg, const int len, const int );
  int handle_char_lvl_up(const char *msg, const int len, const int );
  int handle_skill_upgrapde_or_learn(const char *msg, const int len, const int );
  int handle_update_money(const char *msg, const int len, const int );
  int handle_update_exp(const char *msg, const int len, const int );
  int handle_update_attr(const char *msg, const int len, const int );
  int handle_update_sheng_ming_fa_li(const char *msg, const int len, const int );
  int handle_broadcast_be_hurt_effect(const char *msg, const int len, const int );
  int handle_push_item_list(const char *msg, const int len, const int );
  int handle_add_item(const char *msg, const int len, const int );
  int handle_del_item(const char *msg, const int len, const int );
  int handle_update_item(const char *msg, const int len, const int );
  int handle_use_skill(const char *msg, const int len, const int res);
  int handle_update_task_list(const char *msg, const int len, const int res);

  int handle_push_skill_list(const char *msg, const int len, const int );

  int lua_schedule_timer();
  scene_unit_obj *get_random_scene_unit(const int t, const int no_include_st, const int area_radius);
  scene_unit_obj *get_pos_scene_unit(const int type, const short x, const short y);
  scene_unit_obj *find_scene_unit(const int id);
  skill_info *find_skill(const int skill_cid);
  item_obj *find_item(const int item_cid);

  int do_use_skill();
  skill_info *do_select_skill(const time_value &now,
                              int &att_target_id,
                              short &x,
                              short &y);
  void do_calc_patrl_path(const time_value &now);
protected:
  virtual int dispatch_msg(const int id, 
                           const int res,
                           const char *msg, 
                           const int len);
private:
  int print_operate_err(const int msg_id, const int res);
  void on_scene_unit_exit(const int id);
public:
  short lvl_;
  int scene_cid_;
  int last_heart_beat_time_;
  int att_target_id_;
  int last_use_skill_cid_;
  int search_att_target_area_radius_;
  int diamond_;
  int b_diamond_;
  int coin_;
  int64_t exp_;

	char account_[MAX_ACCOUNT_LEN + 1];
	time_value begin_time_;
	time_value heart_time_;
  time_value reach_pos_time_;
  time_value last_use_skill_time_;
  time_value next_patrol_time_;
  ai_struct *ai_;

  ilist<skill_info *> skill_list_;
  ilist<item_obj *> item_list_;
  ilist<coord_t> patrol_list_;

  typedef std::map<int, scene_unit_obj* > scene_unit_map_t;
  typedef std::map<int, scene_unit_obj* >::iterator scene_unit_map_itor;
  scene_unit_map_t scene_unit_map_;

  social_relation *social_relation_;
  team_info *team_info_;
};
#endif
