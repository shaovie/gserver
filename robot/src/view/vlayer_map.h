#ifndef VLAYER_MAP_H_
#define VLAYER_MAP_H_

// Lib header
#include "vcom.h"
class scene_unit_obj;

/**
 * @class vlayer_map
 *
 * @brief map && mst && player
 */
class vlayer_map : public vlayer
{
public:
  vlayer_map();
  virtual ~vlayer_map();
public:
  virtual int draw(const int now);
  virtual int update(const int event_id, mblock *mb = NULL);
  int to_fetch_su_vpos(scene_unit_obj *su, int &su_vx, int &su_vy);
  int to_get_view_color(const int vx, const int vy, scene_unit_obj *su);
private:
  int do_use_skill();
  int do_draw_skill();
  int do_draw_leader_dir();
  int do_run_state(const int now);
  int leader_vx();
  int leader_vy();
};
#endif // VSCENE_MAP_H_
