#ifndef LOG_WINDOW_H_
#define LOG_WINDOW_H_

#include <deque>
#include <string>
#include "window.h"
class log_window : public window
{
public:
  log_window(int start_y, int start_x, int height, int width):
    window(start_y, start_x, height, width)
   { }
  void add_log(const char *log);
private:
  void show_all_log();

private:
  std::deque<std::string> log_queue_;
};
#endif
