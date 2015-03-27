#include "gm_window.h"

void gm_window::append(const char c)
{
  this->gm_cmd_.append(1, c);
  this->show_gmd();
}

const std::string& gm_window::get_gm_cmd()
{
  return this->gm_cmd_;
}

void gm_window::back_one_char()
{
  if (this->gm_cmd_.empty())
    return ;
  this->gm_cmd_.erase(this->gm_cmd_.end() -1);
  this->show_gmd();
}

void gm_window::show_gmd()
{
  std::string gm = "gm_cmd : ";

  gm += this->gm_cmd_;
  this->clear();
  this->show(1, 1, gm.c_str());
}
