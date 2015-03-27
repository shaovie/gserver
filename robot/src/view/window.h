#ifndef WINDOW_H_
#define WINDOW_H_

#include <ncurses.h>
#include <panel.h>

class window
{
public:
  window(int start_y, int start_x, int height, int width);
  virtual ~window();
  
  int show(const int y, const int x, const char *msg);
  void clear();
  void destroy();
  WINDOW *create_window();

protected:
  void refresh_frame();
protected:
  int y_;
  int x_;
  int height_;
  int width_;
  WINDOW *win_;
  PANEL *panel_;
};
#endif
