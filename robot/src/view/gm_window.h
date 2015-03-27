#ifndef GM_WINDOW_H_
#define GM_WINDOW_H_

#include <string>

#include "window.h"

class gm_window : public window
{
public:
  gm_window(int start_y, int start_x, int height, int width):
    window(start_y, start_x, height, width)
  {
    this->show(1, 1, "gm_cmd :");
  }
  void append(const char c);
  const std::string& get_gm_cmd();
  void back_one_char();
private:
  void show_gmd();
private:
  std::string gm_cmd_;
};

#endif
