#include "scene_unit.h"
#include "istream.h"
#include "scene_mgr.h"

// Lib header

int scene_unit::do_enter_scene()
{
  if (scene_mgr::instance()->enter_scene(this,
                                         this->scene_id_,
                                         this->coord_x_,
                                         this->coord_y_) != 0)
    return -1;
  return 0;
}
int scene_unit::do_exit_scene()
{
  scene_mgr::instance()->exit_scene(this,
                                    this->scene_id_,
                                    this->coord_x_,
                                    this->coord_y_);
  return 0;
}
bool scene_unit::is_new_one(ilist<pair_t<int> > *old_scene_unit_list)
{
  bool new_one = true;
  int size = old_scene_unit_list->size();
  for (int i = 0; i < size; ++i)
  {
    pair_t<int> v = old_scene_unit_list->pop_front();
    if (v.first_ == this->id_)
    {
      new_one = false;
      break;
    }else
      old_scene_unit_list->push_back(v);
  }
  return new_one;
}
