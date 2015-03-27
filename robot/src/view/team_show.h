#ifndef TEAM_SHOW_H_
#define TEAM_SHOW_H_

#include "window.h"
#include <ncurses.h>
#include <map>
#include <string>

using std::map;
using std::string;
class team_show : public window
{
public:
  team_show(int start_y, int start_x, int height, int width);
  virtual ~team_show();

  int add_teamer(const int player_id, const char *name);
  int delet_teamer(const int player_id);

private:
  void clear_show();
  void show_all_teamer();
  void show_1_teamer(const char *name);
private:
  typedef std::map<int, string>  team_map;
  typedef map<int, string>::iterator  team_map_itor;
 
  int next_print_y_;
  team_map team_map_;
};

#endif
