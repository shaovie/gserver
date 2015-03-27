#include "window.h"
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

using std::string;

window::window(int y, int x, int height, int width):
  y_(y),
  x_(x),
  height_(height),
  width_(width),
  win_(NULL),
  panel_(NULL)
{
  this->create_window();
  this->panel_ = new_panel(this->win_); 
  update_panels();
  doupdate();
}

window::~window()
{ }

int window::show(const int y, const int x, const char *msg)
{
  if (y <= 0 || y >= this->height_
     || x <= 0 || x >= this->width_)
    return -1;

  if (this->win_ == NULL)
    this->create_window();

  mvwprintw(this->win_, y, x, msg);
  wrefresh(this->win_);

  return 0;
}

void window::clear()
{
  string empty(this->width_ -2, ' ');
  for (int i = 1; i < (this->height_ - 1); ++i)
  {
    mvwprintw(this->win_, i, 1,  empty.c_str());
    wrefresh(this->win_);
    refresh();
  }
}

void window::destroy()
{
  this->clear();
  wborder(this->win_, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); 
  wrefresh(this->win_);
  delwin(this->win_);
  this->win_ = NULL;

  del_panel(this->panel_);
  this->panel_ = NULL;
}

WINDOW *window::create_window()
{
  this->win_ = newwin(this->height_, this->width_, this->y_, this->x_);
  box(this->win_, 0, 0);
  wborder(this->win_, '|', '|', '_','-', '_', '_' , '-', '-');
  wrefresh(this->win_);

  return  this->win_;
}

void window::refresh_frame()
{
  wborder(this->win_, '|', '|', '_','-', '_', '_' , '-', '-');
  wrefresh(this->win_);
}
