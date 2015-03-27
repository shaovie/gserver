#ifndef CHAT_SHOW_H_
#define CHAT_SHOW_H_

#include "window.h"
#include <ncurses.h>

class chat_show : public window
{
public:
  chat_show(int start_y, int start_x, int height, int width);
  virtual ~chat_show();

  int chat(const char *name, const char *msg);
private:
  void clear();

private:

  int last_chat_len_;
};
#endif
