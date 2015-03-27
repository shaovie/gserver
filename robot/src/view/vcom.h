#ifndef VIEW_COM_H_
#define VIEW_COM_H_

#include "def.h"
#include "mblock.h"
#include "global_macros.h"
#include "scene_unit_obj.h"

#include <ncurses.h>
#include <panel.h>

//============================= readme =============================
/* : world_scene
 * |||||||||||||||||||border|||||||||||||||||||
 * |******************snap********************|
 * |******************************************|
 * |******************************************|
 * |******************************************|
 * |******************************************|
 * |******************************************|
 * |******************************************|
 * |******************************************|
 * |******************snap********************|
 * |||||||||||||||||||border|||||||||||||||||||
 */
/* : class 
 * vdirector 
 * vscene [vworld_scene]
 * vlayer [vlayer_map,vlayer_zoon]
 * vsprite[vsprite_player,vlayer_monster]
 */
//============================ constant ==============================
const int VWIDTH_RADIO  = 4;

const int VBORDER_W = 1; 
const int VBORDER_H = 1; 

const int VGRID_H = 1; 
const int VGRID_W = VWIDTH_RADIO; 
const int VGRID_LINES = (CLT_VIEW_HEIGHT_RADIUS + 5) * 2; 
const int VGRID_COLS  = (CLT_VIEW_WIDTH_RADIUS + 5) * 2; 

const int VSNAP_W = VGRID_COLS * VGRID_W; 
const int VSNAP_H = VGRID_LINES * VGRID_H; 

const int VWORLD_W = VBORDER_W * 2 + VSNAP_W; 
const int VWORLD_H = VBORDER_H * 2 + VSNAP_H; 

const int VLAYER_W = VSNAP_W; 
const int VLAYER_H = VSNAP_H; 

const int VPLAYER_SPRITE_W  = 4; 
const int VPLAYER_SPRITE_H  = 1; 
const int VMONSTER_SPRITE_W = 4; 
const int VMONSTER_SPRITE_H = 1; 

//============================ struct ==============================
//= pair
typedef pair_t<char> vol_t;
typedef pair_t<short> pos_t; 
enum
{
  SPRITE_TYPE_LEA = 1,
  SPRITE_TYPE_PLY = 2,
  SPRITE_TYPE_MST = 3
};
enum
{
  COLOR_MST_CM, // mst && this coord can move
  COLOR_MST_NM, // mst && this coord can't move
  COLOR_PLY_CM, // player can move
  COLOR_PLY_NM, // player can't move
  COLOR_LED_CM, // leader can move
  COLOR_LED_NM, // leader can't move
  COLOR_WORLD_BOX, // world box color
  COLOR_LEAER_SKILL, // skill color 
  COLOR_NOTE_TITLE, // 
  COLOR_LEADER_TARGET, // 
  COLOR_PLY_DEAD,
};
enum TOUCH_ID
{
  ROLE_STEP_LEFT,
  ROLE_STEP_RIGHT,
  ROLE_STEP_UP,
  ROLE_STEP_DOWN,
  ROLE_USE_SKILL,
  ROLE_CHANGE_DIR,
};
enum EVENT_ID
{
  VEVENT_USE_SKILL,
};
//========================================= function ===================================
/**
 * @class vcom
 *
 * @brief view common 
 */
class vcom
{
public:
  static pos_t gen_start_coord(const pos_t &pos, const vol_t &vol)
  { return pos_t(pos.first_ - vol.first_ / 2, pos.second_ - vol.second_ / 2); }
  static bool is_snap_coord(const short x, const short y)
  {
    return x > VBORDER_W 
      && x <= VSNAP_W + VBORDER_W
      && y > VBORDER_H
      && y <= VSNAP_H + VBORDER_H;
  }
  static bool is_snap_coord(const pos_t &pos)
  { return is_snap_coord(pos.first_, pos.second_); }
};
//========================================= class ===================================
/**
 * @class vnode
 *
 * @brief basic node 
 */
class vnode
{
public:
  vnode()
    : begin_x_(0),
    begin_y_(0),
    nlines_(0),
    ncols_(0)
  { }
  virtual ~vnode() { }
public:
  virtual int draw(const int now) = 0;
  virtual int type() { return 0; }
public:
  int begin_x_; // at the screen pos
  int begin_y_; // at the screen pos
  int nlines_;  // own area 
  int ncols_;   // own area 
};
/**
 * @class vscene 
 *
 * @brief basic scene 
 */
class vscene : public vnode
{
public:
  vscene() { }
  virtual ~vscene() { }
public:
  virtual int update(const int event_id, mblock *mb = NULL) = 0; 
};
/**
 * @class vlayer 
 *
 * @brief basic layer
 */
class vlayer : public vnode
{
public:
  vlayer()
    : panel_(NULL)
  { }
  virtual ~vlayer() { }
  virtual int update(const int event_id, mblock *mb = NULL) = 0; 
public:
  PANEL *panel_;
};
#endif
