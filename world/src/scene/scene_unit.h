// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-09-05 00:00
 */
//========================================================================

#ifndef SCENE_UNIT_H_
#define SCENE_UNIT_H_

#include "def.h"
#include "ilist.h"
#include "istream.h"

// Forward declarations
class mblock;
class out_stream;

const char T_END                = 0;
const char T_CID_ID             = 1;
const char T_BASE_INFO          = 2;
const char T_POS_INFO           = 3;
const char T_STATUS             = 4;
const char T_HP_MP              = 5;
const char T_EQUIP_INFO         = 6;
const char T_X_Y_INFO           = 7;
const char T_MST_EXTRA_INFO     = 8;

enum SNAP_STATUS
{
  SNAP_LIFE               = 1,   // value 0:alive 1:dead 2:exit
  LIFE_V_ALIVE            = 0,   // value 0:alive 1:dead 2:exit
  LIFE_V_DEAD             = 1,   // value 0:alive 1:dead 2:exit
  LIFE_V_EXIT             = 2,   // value 0:alive 1:dead 2:exit
};
/**
 * @class scene_unit
 * 
 * @brief 表示要在场景中显示的个体
 */
class scene_unit
{
public:
  enum
  {
    PLAYER           = 1L << 1,   //
    MONSTER          = 1L << 2,   //
    DROPPED_ITEM     = 1L << 3,   //
    EFFECT           = 1L << 4,   //
    UNIT_CNT,
  };
  scene_unit()
    : dir_(DIR_RIGHT_DOWN),
    coord_x_(0),
    coord_y_(0),
    id_(0),
    cid_(0),
    scene_id_(0),
    scene_cid_(0),
    block_radius_(0),
    unit_status_(0)
  { }

  virtual ~scene_unit() { }

  //= inline method
  virtual int unit_type()   const = 0;
  virtual bool can_block()  const = 0;

  inline int  unit_status() const        { return this->unit_status_;  }
  inline void add_status(const int val)  { this->unit_status_ |= val;  }
  inline void del_status(const int val)  { this->unit_status_ &= ~val; }

  inline int id()          const { return this->id_;            }
  inline int cid()         const { return this->cid_;           }

  inline char  dir()       const { return this->dir_;           }
  inline short coord_x()   const { return this->coord_x_;       }
  inline short coord_y()   const { return this->coord_y_;       }

  inline int scene_id()    const { return this->scene_id_;      }
  inline int scene_cid()   const { return this->scene_cid_;     }

  inline int block_radius()const { return this->block_radius_;  } 

  virtual int do_enter_scene();
  virtual int do_exit_scene();

  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_list,
                                  ilist<pair_t<int> > *new_snap_list) = 0;

  virtual void do_delivery(mblock *) { return ; }

  bool is_new_one(ilist<pair_t<int> > *old_snap_list);
protected:
  char  dir_;
  short coord_x_;
  short coord_y_;

  int  id_;
  int  cid_;

  int scene_id_;
  int scene_cid_;
  int block_radius_;
  int unit_status_;
};
#endif // SCENE_UNIT_H_

