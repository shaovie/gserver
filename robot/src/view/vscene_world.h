#ifndef VSCENE_WORLD_H_
#define VSCENE_WORLD_H_

// Lib header
#include "vcom.h"
#include "vlayer_map.h"

/**
 * @class vscene_world
 *
 * @brief game main scene
 */
class vscene_world : public vscene
{
public:
  vscene_world();
  ~vscene_world();
public:
  virtual int draw(const int now);
  virtual int update(const int event_id, mblock *mb = NULL);
private:
  vlayer_map *map_;
  PANEL *panel_;
  WINDOW *win_;
};
#endif // VSCENE_WORLD_H_
