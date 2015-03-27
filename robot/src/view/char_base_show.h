#ifndef CHAR_BASE_SHOW_H_
#define CHAR_BASE_SHOW_H_

#include "window.h"
#include <string>

using std::string;

class char_base_show : public window
{
public:
  char_base_show(int start_y, int start_x, int height, int width);
  int show_info(const char *name, const short lvl, const char career);
private:
  string name_;
};

#endif
