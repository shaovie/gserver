#include "director.h"
#include "vscene_world.h"
#include "player.h"
#include "scene_config.h"
#include "scene_unit_obj.h"
#include "istream.h"
#include "message.h"
#include "util.h"

// Lib header
#include <unistd.h>
#include <fcntl.h>

const int max_frame_num = 999999999;

director::director()
: frame_num_(0),
  cur_scene_(NULL),
  leader_(NULL),
  chat_window_(NULL),
  team_window_(NULL),
  char_base_window_(NULL),
  gm_window_(NULL),
  log_window_(NULL)
{ }
int director::init(player *leader)
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  if(has_colors() == FALSE)
  {
    printf("You terminal does not support color\n");
    return this->exit();
  }

  start_color();
  init_pair(COLOR_MST_CM, COLOR_WHITE, COLOR_BLACK);
  init_pair(COLOR_MST_NM, COLOR_WHITE, COLOR_RED);

  init_pair(COLOR_PLY_CM, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_PLY_NM, COLOR_GREEN, COLOR_RED);
  init_pair(COLOR_PLY_DEAD, COLOR_GREEN, COLOR_RED);

  init_pair(COLOR_LED_CM, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_LED_NM, COLOR_YELLOW, COLOR_RED);

  init_pair(COLOR_WORLD_BOX, COLOR_WHITE, COLOR_YELLOW);
  init_pair(COLOR_LEAER_SKILL, COLOR_WHITE, COLOR_CYAN);
  init_pair(COLOR_NOTE_TITLE, COLOR_YELLOW, COLOR_BLACK);

  init_pair(COLOR_LEADER_TARGET, COLOR_RED, COLOR_MAGENTA);

  // first_scene
  this->cur_scene_ = new vscene_world();

  const int chat_height = 6;
  const int chat_width = 30;
  const int chat_start_x = COLS/2 - chat_width/2;
  const int chat_start_y = LINES / 2 + VWORLD_H / 2 - chat_height;
  this->chat_window_ = new chat_show(chat_start_y,
                                     chat_start_x,
                                     chat_height,
                                     chat_width);


  const int base_height = 6;
  const int base_width = 10;
  const int char_base_start_x = COLS/2 - VWORLD_W/2 + 1;
  const int char_base_start_y = LINES/2 - VWORLD_H/2 + 1;
  this->char_base_window_  = new char_base_show(char_base_start_y,
                                                char_base_start_x,
                                                base_height,
                                                base_width);

  const int team_height = 10;
  const int team_width = 10;
  const int team_start_x = char_base_start_x;
  const int team_start_y = char_base_start_y + base_height + 2;
  this->team_window_ = new team_show(team_start_y,
                                     team_start_x,
                                     team_height,
                                     team_width);


  const int log_height = VWORLD_H;
  const int log_width = 36;
  const int log_start_x = COLS/2 + VWORLD_W/2 + 1;
  const int log_start_y =  LINES/2 - VWORLD_H/2;
  this->log_window_ = new log_window(log_start_y,
                                     log_start_x,
                                     log_height,
                                     log_width);

  this->leader_ = leader;

  // show char base info
  this->char_base_window_->show_info(leader_->name_,
                                     leader_->lvl_,
                                     leader_->career_);

  //

  fcntl(0, F_SETFL, O_NONBLOCK);
  return 0;
}
int director::exit()
{
  if (this->cur_scene_ != NULL)
  {
    delete this->cur_scene_;
    this->cur_scene_ = NULL;
    endwin();
  }
  this->leader_ = NULL;
  return 0;
}
int director::draw(const int now)
{
  this->frame_num_ = this->frame_num_ % max_frame_num + 1;

  if (this->cur_scene_ == NULL)
    return 0;

  if (this->touch() != 0)
    return -1;

  this->do_note_show(now);
  this->cur_scene_->draw(now);

  refresh();
  update_panels();
  doupdate();
  return 0;
}
int director::do_note_show(const int now)
{
  for (int i = 0; i < 40; ++i)
    for (int j = 14; j < 60; ++j)
      mvprintw(j, i, " ");

  attron(COLOR_PAIR(COLOR_NOTE_TITLE));
  int i = 14;
  mvprintw(i++, 0, "Time \t\t%d", now);
  mvprintw(i++, 0, "Frame \t\t%d", this->frame_num_);
  i++;
  mvprintw(i++, 0, "Exit \t\tF10");
  mvprintw(i++, 0, "DIR \t\tO");
  mvprintw(i++, 0, "Skill \t\tH J K L");
  mvprintw(i++, 0, "Move \t\tW S A D");
  mvprintw(i++, 0, "GM \t\tTab");
  i++;
  if (this->leader_ != NULL)
  {
    mvprintw(i++, 0, "ID \t\t%d", this->leader_->id_);
    mvprintw(i++, 0, "MY_POS \t\t%d %d.%d", this->leader_->scene_cid_, this->leader_->x_, this->leader_->y_);
    mvprintw(i++, 0, "MY_HP \t\t%d:%d", this->leader_->total_sheng_ming_, this->leader_->sheng_ming_);
    mvprintw(i++, 0, "MY_MP \t\t%d:%d", this->leader_->total_fa_li_, this->leader_->fa_li_);
    mvprintw(i++, 0, "MY_EXP \t\t%d", this->leader_->exp_);
    mvprintw(i++, 0, "UNITS \t\t%d", this->leader_->scene_unit_map_.size());
    i++;
    scene_unit_obj *su = this->leader_->find_scene_unit(this->leader_->att_target_id_);
    if (su != NULL)
    {
      mvprintw(i++, 0, "TARGET \t\t%s", su->name());
      mvprintw(i++, 0, "HP \t\t%d/%d", su->sheng_ming_, su->total_sheng_ming_);
    }
  }
  attroff(COLOR_PAIR(COLOR_NOTE_TITLE));

  this->cur_scene_->draw(now);
  refresh();
  update_panels();
  doupdate();
  return 0;
}
int director::touch() // notice touch
{
  // only world
  const int once_acc = 5;
  for (int i = 0; i < once_acc; ++i)
  {
    const int key = getch();
    if (key == -1)
      break;
    if (this->gm_window_ != NULL)
    {
      this->handle_gm_input(key);
      return 0;
    }

    // control
    switch(key)
    {
    case 'a':
    case 'A':
    case KEY_LEFT:
      this->do_touch(ROLE_STEP_LEFT);
      break;
    case 'd':
    case 'D':
    case KEY_RIGHT:
      this->do_touch(ROLE_STEP_RIGHT);
      break;
    case 'w':
    case 'W':
    case KEY_UP:
      this->do_touch(ROLE_STEP_UP);
      break;
    case 's':
    case 'S':
    case KEY_DOWN:
      this->do_touch(ROLE_STEP_DOWN);
      break;
    case 'h':
    case 'H':
    case 'j':
    case 'J':
    case 'k':
    case 'K':
    case 'l':
    case 'L':
      this->do_touch(ROLE_USE_SKILL);
      break;
    case 'o':
    case 'O':
      this->do_touch(ROLE_CHANGE_DIR);
      break;
    case '\t':
      this->do_create_gm_window();
      break;
    case KEY_F(10):
      this->exit();
      return -1;
    }
  }
  return 0;
}
int director::do_touch(const int touch_id)
{
  switch(touch_id)
  {
  case ROLE_STEP_LEFT:
    this->do_leader_move(-1, 0);
    break;
  case ROLE_STEP_RIGHT:
    this->do_leader_move(1, 0);
    break;
  case ROLE_STEP_UP:
    this->do_leader_move(0, -1);
    break;
  case ROLE_STEP_DOWN:
    this->do_leader_move(0, 1);
    break;
  case ROLE_USE_SKILL:
    this->do_use_skill();
    break;
  case ROLE_CHANGE_DIR:
    this->do_change_dir();
    break;
  }
  return 0;
}
int director::do_leader_move(const short offset_x, const short offset_y)
{
  if (this->leader_ == NULL) return 0;

  time_value now = time_value::gettimeofday();
  if (now < this->leader_->reach_pos_time_) return 0;

  const short x = this->leader_->x_ + offset_x;
  const short y = this->leader_->y_ + offset_y;
  const int type = scene_unit_obj::PLAYER|scene_unit_obj::MONSTER;
  if (!scene_config::instance()->can_move(this->leader_->scene_cid_, x, y)
      || this->leader_->get_pos_scene_unit(type, x, y) != NULL)
    return 0;

  char dir = util::calc_next_dir(this->leader_->x_, this->leader_->y_, x, y);
  long int move_time = util::calc_move_time(dir, this->leader_->speed_);
  this->leader_->reach_pos_time_ = now + time_value(0, move_time * 1000);

  out_stream os(client::send_buf, client::send_buf_len);
  os << dir << x << y;
  this->leader_->send_request(REQ_CHAR_MOVE, &os);

  this->leader_->dir_ = dir;
  this->leader_->x_ = x;
  this->leader_->y_ = y;
  return 0;
}
int director::do_use_skill()
{
  if (this->leader_ == NULL) return 0;

  this->cur_scene_->update(VEVENT_USE_SKILL);

#if 0
  scene_unit_obj *su = this->to_get_leader_dir_first_obj();
  if (su == NULL) return 0;
  const int skill_cid = 31110001;
  out_stream os(client::send_buf, client::send_buf_len);
  os << skill_cid << su->id_ << su->x_ << su->y_;
  return this->leader_->send_request(REQ_USE_SKILL, &os);
#else
  return this->leader_->do_use_skill();
#endif
}
scene_unit_obj *director::to_get_leader_dir_first_obj()
{
  int start_x = 0, end_x = 0;
  int start_y = 0, end_y = 0;
  const int skill_range = 4;
  switch(this->leader_->dir_)
  {
  case DIR_UP:
    start_x = end_x = this->leader_->x_; 
    start_y = this->leader_->y_ - skill_range, end_y = this->leader_->y_;
    break;
  case DIR_RIGHT:
    start_x = this->leader_->x_, end_x = this->leader_->x_ + skill_range;
    start_y = end_y = this->leader_->y_; 
    break;
  case DIR_DOWN:
    start_x = end_x = this->leader_->x_; 
    start_y = this->leader_->y_, end_y = this->leader_->y_ + skill_range;
    break;
  case DIR_LEFT:
    start_x = this->leader_->x_ - skill_range, end_x = this->leader_->x_;
    start_y = end_y = this->leader_->y_; 
    break;
  }
  const int type = scene_unit_obj::PLAYER|scene_unit_obj::MONSTER;
  for (int i = start_x; i <= end_x; ++i)
  {
    for (int j = start_y; j <= end_y; ++j)
    {
      scene_unit_obj *su = this->leader_->get_pos_scene_unit(type, i, j);
      if (su != NULL && su->id_ != this->leader_->id_) return su;
    }
  }
  return NULL;
}
int director::do_change_dir()
{
  if (this->leader_ == NULL) return 0;

  int next_dir = this->to_get_next_dir(this->leader_->dir_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->leader_->dir_ << this->leader_->x_ << this->leader_->y_;
  this->leader_->send_request(REQ_CHAR_MOVE, &os);

  this->leader_->dir_ = next_dir;
  return 0;
}
int director::to_get_next_dir(const char dir)
{
  switch(dir)
  {
  case DIR_UP:
    return DIR_RIGHT;
  case DIR_RIGHT:
    return DIR_DOWN;
  case DIR_DOWN:
    return DIR_LEFT;
  case DIR_LEFT:
  case DIR_RIGHT_UP:
  case DIR_RIGHT_DOWN:
  case DIR_LEFT_DOWN:
  case DIR_LEFT_UP:
    return DIR_UP;
  }
  return DIR_XX;
}
int director::update(const int event, mblock *mb) // on event
{ return this->cur_scene_->update(event, mb); }


void director::show_char_info(const char *name,
                              const short lvl,
                              const char career)
{
  this->char_base_window_->show_info(name, lvl, career);
}
void director::add_teamer(const int char_id, const char *name)
{
  this->team_window_->add_teamer(char_id, name);
}
void director::delet_teamer(const int char_id)
{
  this->team_window_->delet_teamer(char_id);
}
void director::chat(const char *name, const char *msg)
{
  this->chat_window_->chat(name, msg);
}
int director::do_create_gm_window()
{
  if (this->gm_window_ != NULL)
    return 0;

  const int gm_height = 3;
  const int gm_width = VWORLD_W;
  const int gm_start_x = COLS/2 - VWORLD_W/2 + 1;
  const int gm_start_y = LINES/2 + VWORLD_H/2 - 1;
  this->gm_window_ = new gm_window(gm_start_y,
                                   gm_start_x,
                                   gm_height,
                                   gm_width);
  this->gm_window_->show(1, 1, "gm_cmd : ");
  return 0;
}

int director::handle_gm_input(const int input_key)
{
  if ('\n' == input_key)
  {
    const string gm = this->gm_window_->get_gm_cmd();
    if (!gm.empty())
    {
      string gm_str = gm;
      out_stream os(client::send_buf, client::send_buf_len);
      if (gm[0] == '@')
      {
        gm_str = gm.substr(1);
        stream_ostr gm_so(gm_str.c_str(), gm_str.length());
        os << gm_so;
        this->leader_->send_request(REQ_GM_CMD, &os);
      }else
      {
        stream_ostr so(gm.c_str(), gm.length());
        os << so;
        this->leader_->send_request(REQ_CHAT_WORLD, &os);
      }
    }
    this->gm_window_->destroy();
    this->gm_window_ = NULL;
  }else
  {
    if (input_key == 263) //backspace
      this->gm_window_->back_one_char();
    else
      this->gm_window_->append(static_cast<char>(input_key));
  }
  /*
     string a("sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss");
     string b(a.c_str(), rand()%15) ;
     this->log_window_->add_log(b.c_str());
     */
  return 0;
}


void director::log(const char *log)
{
  this->log_window_->add_log(log);
}
