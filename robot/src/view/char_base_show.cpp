#include "char_base_show.h"

char_base_show::char_base_show(int start_y, int start_x, int height, int width):
  window(start_y, start_x, height, width)
{ }

int char_base_show::show_info(const char *name,
                              const short lvl,
                              const char career)
{
  if (this->show(1, 1, name) < 0)
    return -1;
  char bf[16] = {0};
  ::snprintf(bf, sizeof(bf), "l:%02d c:%d", lvl, career);
  if (this->show(2, 1, bf) < 0)
    return -1;
  this->name_ = name;
  return 0;
}
