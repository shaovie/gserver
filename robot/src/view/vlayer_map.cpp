#include "vlayer_map.h"
#include "scene_unit_obj.h"
#include "player.h"
#include "director.h"
#include "scene_config.h"
#include "sys_log.h"
#include "time_value.h"
#include "macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("view");
static ilog_obj *e_log = err_log::instance()->get_ilog("view");

enum
{
  LEADER_VS_NULL,
  LEADER_VS_USE_SKILL,
};

static int s_leader_v_state = LEADER_VS_NULL;
//static int s_skill_cur_pos = 0;
static int s_skill_show_cnt = 0;

vlayer_map::vlayer_map()
{ }
vlayer_map::~vlayer_map()
{ }
int vlayer_map::draw(const int )
{
  // clean
  static WINDOW *win = panel_window(this->panel_);
  for(int i = 1; i <= this->ncols_; ++i)
  {
    for(int j = 1; j <= this->nlines_; ++j)
    {
      const int color = this->to_get_view_color(i, j, NULL);
      wattron(win, COLOR_PAIR(color));
      mvwprintw(win, j, i, " ");
      wattroff(win, COLOR_PAIR(color));
    }
  }

  // other
  int su_vx = 0;
  int su_vy = 0;
  player *leader = director::instance()->leader();
  std::map<int, scene_unit_obj* >::iterator itor = leader->scene_unit_map_.begin();
  for (; itor != leader->scene_unit_map_.end(); ++itor)
  {
    scene_unit_obj *su = itor->second;
    this->to_fetch_su_vpos(su, su_vx, su_vy);
    if (su->id_ == leader->id_) continue;

    int color = this->to_get_view_color(su_vx, su_vy, su);
    if (BIT_ENABLED(su->status_, OBJ_DEAD)) color = COLOR_PLY_DEAD;
    wattron(win, COLOR_PAIR(color));
    mvwprintw(win, su_vy, su_vx, "%s", su->name());
    wattroff(win, COLOR_PAIR(color));
  }

  // leader
  wattron(win, COLOR_PAIR(COLOR_PLY_CM));
  mvwprintw(win, this->leader_vy(), this->leader_vx(), "%s", leader->name());
  this->do_draw_leader_dir();
  wattroff(win, COLOR_PAIR(COLOR_PLY_CM));

  // skill draw
  this->do_draw_skill();
  return 0;
}
int vlayer_map::to_fetch_su_vpos(scene_unit_obj *su, int &su_vx, int &su_vy)
{
  player *leader = director::instance()->leader();
  su_vx = this->leader_vx() + (su->x_ - leader->x_) * VGRID_W;
  su_vy = this->leader_vy() + (su->y_ - leader->y_) * VGRID_H;
  return 0;
}
int vlayer_map::to_get_view_color(const int vx, const int vy, scene_unit_obj *su)
{
  player *leader = director::instance()->leader();
  if (su != NULL && su->id_ == leader->att_target_id_) return COLOR_LEADER_TARGET; 

  const int x = leader->x_ + (vx / VGRID_W - this->leader_vx() / VGRID_W);
  const int y = leader->y_ + (vy / VGRID_H - this->leader_vy() / VGRID_H);
  const bool can_move = scene_config::instance()->can_move(leader->scene_cid_, x, y);
  if (can_move && (su == NULL || su->unit_type() == scene_unit_obj::PLAYER))
    return COLOR_PLY_CM;
  if (!can_move && (su == NULL || su->unit_type() == scene_unit_obj::PLAYER))
    return COLOR_PLY_NM;
  if (can_move && (su->unit_type() == scene_unit_obj::MONSTER))
    return COLOR_MST_CM;
  if (!can_move && (su->unit_type() == scene_unit_obj::MONSTER))
    return COLOR_MST_NM;
  if (su->unit_type() == scene_unit_obj::DROPPED_ITEM)
    return COLOR_MST_CM;
  return 0;
}
int vlayer_map::do_use_skill()
{
  // begin
  s_leader_v_state = LEADER_VS_USE_SKILL;
  return 0;
}
int vlayer_map::do_draw_skill()
{
  if (s_leader_v_state != LEADER_VS_USE_SKILL)
    return 0;

#if 0
  const int cd = 1;
  if (director::instance()->frame_num() % cd == 0)
  {
    if (s_skill_cur_pos == 0)
      s_skill_cur_pos = this->leader_vy() - 1;
    else
      --s_skill_cur_pos;
  }

  static WINDOW *win = panel_window(this->panel_);
  wattron(win, COLOR_PAIR(COLOR_LEAER_SKILL));
  mvwprintw(win, s_skill_cur_pos, this->leader_vx(), "    ");
  wattroff(win, COLOR_PAIR(COLOR_LEAER_SKILL));

  // area
  const int skill_area = 4 * VGRID_W;
  if (this->leader_vy() - s_skill_cur_pos > skill_area)
  {
    s_skill_cur_pos = 0;
    s_leader_v_state = LEADER_VS_NULL;
  }
#elseif 0
  int start_x = 0, end_x = 0;
  int start_y = 0, end_y = 0;
  const int skill_range = 4;
  switch(director::instance()->leader()->dir_)
  {
  case DIR_UP:
    start_x = end_x = this->leader_vx(); 
    start_y = this->leader_vy() - skill_range, end_y = this->leader_vy();
    break;
  case DIR_RIGHT:
    start_x = this->leader_vx(), end_x = this->leader_vx() + skill_range * VWIDTH_RADIO;
    start_y = end_y = this->leader_vy(); 
    break;
  case DIR_DOWN:
    start_x = end_x = this->leader_vx(); 
    start_y = this->leader_vy(), end_y = this->leader_vy() + skill_range;
    break;
  case DIR_LEFT:
    start_x = this->leader_vx() - skill_range * VWIDTH_RADIO, end_x = this->leader_vx();
    start_y = end_y = this->leader_vy(); 
    break;
  }
  static WINDOW *win = panel_window(this->panel_);
  wattron(win, COLOR_PAIR(COLOR_LEAER_SKILL));
  for (int i = start_x; i <= end_x; ++i)
    for (int j = start_y; j <= end_y; ++j)
      mvwprintw(win, j, i, "    ");
  wattroff(win, COLOR_PAIR(COLOR_LEAER_SKILL));
  if (++s_skill_show_cnt == 4)
  {
    s_skill_show_cnt = 0;
    s_leader_v_state = LEADER_VS_NULL;
  }
#else
  static WINDOW *win = panel_window(this->panel_);
  wattron(win, COLOR_PAIR(COLOR_LEAER_SKILL));
  mvwprintw(win, this->leader_vy() - s_skill_show_cnt, this->leader_vx(), "    "); // up
  mvwprintw(win, this->leader_vy() + s_skill_show_cnt, this->leader_vx(), "    "); // down 
  mvwprintw(win, this->leader_vy(), this->leader_vx() - s_skill_show_cnt, " "); // left 
  mvwprintw(win, this->leader_vy(), this->leader_vx() + VGRID_W + s_skill_show_cnt, " "); // right 
  wattroff(win, COLOR_PAIR(COLOR_LEAER_SKILL));
  if (++s_skill_show_cnt == 4)
  {
    s_skill_show_cnt = 0;
    s_leader_v_state = LEADER_VS_NULL;
  }
#endif
  return 0;
}
int vlayer_map::do_draw_leader_dir()
{
  static WINDOW *win = panel_window(this->panel_);
  switch(director::instance()->leader()->dir_)
  {
  case DIR_UP:
    mvwprintw(win, this->leader_vy() - VGRID_H, this->leader_vx(), "^");
    break;
  case DIR_RIGHT:
    mvwprintw(win, this->leader_vy(), this->leader_vx() + VGRID_W, ">");
    break;
  case DIR_DOWN:
    mvwprintw(win, this->leader_vy() + VGRID_H, this->leader_vx(), "V");
    break;
  case DIR_LEFT:
    mvwprintw(win, this->leader_vy(), this->leader_vx() - 1 , "<");
    break;
  }
  return 0;
}
int vlayer_map::update(const int event_id, mblock *)
{
  switch(event_id)
  {
  case VEVENT_USE_SKILL:
    return this->do_use_skill();
  }
  return 0;
}
int vlayer_map::leader_vx()
{ return VSNAP_W / 2; }
int vlayer_map::leader_vy()
{ return VSNAP_H / 2; }
