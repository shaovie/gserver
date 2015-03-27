#include "log_window.h"

void log_window::add_log(const char* log)
{
  this->log_queue_.push_front(std::string(log));
  if (this->log_queue_.size() > 80)
    this->log_queue_.pop_back();

  this->show_all_log();
}
void log_window::show_all_log()
{
  this->clear();
  std::deque<std::string>::iterator iter = this->log_queue_.begin();
  int begin_y = 1;
  int x = 1;
  for (; iter != this->log_queue_.end(); ++iter)
  {
    this->show(begin_y, x, (*iter).c_str());
    begin_y += (*iter).length()/(this->width_) + 1 + 0; //second 2 is to kong hang 
    if (begin_y + 1 >= this->height_)
      break;
  }
}
