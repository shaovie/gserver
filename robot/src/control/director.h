#ifndef DIRECTOR_H_
#define DIRECTOR_H_

// Lib header
#include "singleton.h"
#include "vcom.h"
#include "team_show.h"
#include "chat_show.h"
#include "char_base_show.h"
#include "gm_window.h" 
#include "log_window.h"

class player;
class scene_unit_obj;
/**
 * @class director
 *
 * @brief the view main
 */
class director : public singleton<director>
{
  friend class singleton<director>;
public:
  int init(player *leader);
  int exit();
  int draw(const int now);
  int touch();
  int update(const int event_id, mblock *mb = NULL);
  player *leader() { return this->leader_; };
  void add_teamer(const int char_id, const char *name);
  void delet_teamer(const int char_id);
  void chat(const char *name, const char *msg);
  void show_char_info(const char *name, const short, const char);
  int frame_num() { return this->frame_num_; };
  void log(const char *log_message);
private:
  int do_note_show(const int now);
  int do_touch(const int touch_id);
  int do_leader_move(const short offset_x, const short offset_y);
  int do_use_skill();
  scene_unit_obj *to_get_leader_dir_first_obj();
  int do_change_dir();
  int to_get_next_dir(const char dir);
  int do_create_gm_window();
  int handle_gm_input(const int input_char);
private:
  director();

  int frame_num_;
  vscene *cur_scene_;
  player *leader_;

  chat_show *chat_window_;
  team_show *team_window_;
  char_base_show *char_base_window_;
  gm_window  *gm_window_;
  log_window *log_window_;
};
#endif // DIRECTOR_H_
