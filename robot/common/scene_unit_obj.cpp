#include "scene_unit_obj.h"
#ifdef FOR_ONEROBOT
#include "robot_ncurses.h"
#include "director.h"
#endif

void scene_unit_obj::on_sheng_ming_fa_li_update(const int total_sheng_ming,
                                                const int sheng_ming,
                                                const int total_fa_li,
                                                const int fa_li)
{
  if (this->total_sheng_ming_ > 0)
  {
    char bf[128] = {0};
    int diff = sheng_ming - this->sheng_ming_;
    ::snprintf(bf, sizeof(bf), "%s hp %d:%d:%d",
               this->name(),
               total_sheng_ming,
               sheng_ming,
               diff);
#ifdef FOR_ONEROBOT
    director::instance()->log(bf);
#endif
  }
  this->total_sheng_ming_ = total_sheng_ming;
  this->sheng_ming_ = sheng_ming;
  this->total_fa_li_ = total_fa_li;
  this->fa_li_ = fa_li;
}
