#include "vscene_world.h"

vscene_world::vscene_world()
{
  const int x = COLS / 2 - VWORLD_W / 2;
  const int y = LINES / 2 - VWORLD_H / 2;
  this->win_ = newwin(VWORLD_H, VWORLD_W, y, x);
  this->panel_ = new_panel(this->win_);

  this->map_ = new vlayer_map();
  this->map_->panel_ = this->panel_;
  this->map_->begin_x_ = x, this->map_->begin_y_ = y;
  this->map_->ncols_ = VLAYER_W;
  this->map_->nlines_ = VLAYER_H;
}
vscene_world::~vscene_world()
{
  del_panel(this->panel_);
  delwin(this->win_);
  delete this->map_;
}
int vscene_world::draw(const int now)
{
  wattron(this->win_, COLOR_PAIR(COLOR_WORLD_BOX));
  wborder(this->win_, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wattroff(this->win_, COLOR_PAIR(COLOR_WORLD_BOX));
  this->map_->draw(now);
  return 0;
}
int vscene_world::update(const int event_id, mblock *mb)
{ return this->map_->update(event_id, mb); }
