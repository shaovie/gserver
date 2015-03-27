#include "team_show.h"

team_show::team_show(int start_y, int start_x, int height, int width):
  window(start_y, start_x, height, width),
  next_print_y_(1)
{ }

team_show::~team_show()
{ }

int team_show::add_teamer(const int player_id, const char *name)
{
  if (this->team_map_.find(player_id) != this->team_map_.end())
    return -1;
  this->team_map_.insert(std::make_pair(player_id, name));
  show_1_teamer(name);
  return 0;
}

int team_show::delet_teamer(const int player_id)
{
  team_map_itor iter = this->team_map_.find(player_id);
  if (iter == this->team_map_.end())
    return -1;

  this->clear_show();
  this->team_map_.erase(iter);
  this->show_all_teamer();
  return 0;
}

void team_show::clear_show()
{
  static string empty(this->width_-2, ' ');
  int teamer_num = this->team_map_.size();

  //why i begin from 1 : the first y is show in window is 1
  for (int i = 1; i < teamer_num + 1; ++i)
  {
    mvwprintw(this->win_, i, 1,  empty.c_str());
    wrefresh(this->win_);
    refresh();
  }
  this->next_print_y_ = 1;
}

void team_show::show_all_teamer()
{
  team_map_itor itor = this->team_map_.begin();
  for (; itor != this->team_map_.end(); ++itor)
    this->show_1_teamer(itor->second.c_str());
}
void team_show::show_1_teamer(const char *name)
{
  mvwprintw(this->win_, this->next_print_y_++, 1, name);
  wrefresh(this->win_);
}
