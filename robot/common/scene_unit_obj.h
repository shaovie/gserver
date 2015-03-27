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
#include "global_macros.h"

// Forward declarations
class mblock;
class out_stream;

const char T_END          = 0;
const char T_CID_ID       = 1;
const char T_BASE_INFO    = 2;
const char T_POS_INFO     = 3;
const char T_STATUS       = 4;
const char T_SHENG_MING_FA_LI = 5;
const char T_EQUIP_INFO   = 6;
const char T_X_Y_INFO    = 7;
const char T_MST_EXTRA_INFO  = 8;

enum SNAP_STATUS
{
  SNAP_LIFE               = 1,   // value 0:alive 1:dead 2:exit
};
/**
 * @class scene_unit_obj
 * 
 * @brief 表示要在场景中显示的个体
 */
class scene_unit_obj
{
public:
  enum
  {
    PLAYER           = 1L << 1,   //
    MONSTER          = 1L << 2,   //
    DROPPED_ITEM     = 1L << 3,   //
    EFFECT           = 1L << 4,   //
    UNIT_END
  };
  scene_unit_obj()
    : dir_(DIR_RIGHT_DOWN),
    x_(0),
    y_(0),
    speed_(100),
    id_(0),
    cid_(0),
    total_sheng_ming_(0),
    sheng_ming_(0),
    total_fa_li_(0),
    fa_li_(0),
    status_(0),
    block_radius_(0)
  { }

  virtual ~scene_unit_obj() { }

  //= inline method
  virtual int unit_type()   const = 0;
  virtual bool can_block()  const = 0;

  virtual const char *name() = 0;
  virtual void on_sheng_ming_fa_li_update(const int total_sheng_ming,
                                          const int sheng_ming,
                                          const int total_fa_li,
                                          const int fa_li);
public:
  char  dir_;
  short x_;
  short y_;
  short speed_;

  int  id_;
  int  cid_;

  int total_sheng_ming_;
  int sheng_ming_;
  int total_fa_li_;
  int fa_li_;
  int status_;
  int block_radius_;
};

class scene_player : public scene_unit_obj
{
public:
  scene_player(const int cid, const int id) :
    career_(0)
  {
    cid_ = cid;
    id_ = id;
    ::memset(name_, 0, sizeof(name_));
  }
  virtual int unit_type() const { return scene_unit_obj::PLAYER; }
  virtual bool can_block() const { return true; }
  virtual const char *name() { return this->name_; }

  char career_;
  char name_[MAX_NAME_LEN + 1];
};
class scene_monster : public scene_unit_obj
{
public:
  scene_monster(const int cid, const int id)
  {
    cid_ = cid;
    id_ = id;
  }
  virtual int unit_type() const { return scene_unit_obj::MONSTER; }
  virtual bool can_block() const { return true; }
  virtual const char *name()
  {
    static char name[32] = {0};
    ::snprintf(name, sizeof(name), "%d:%d", this->cid_, this->id_ % 1000);
    return name;
  }
};
class scene_item : public scene_unit_obj
{
public:
  scene_item(const int cid, const int id)
  {
    cid_ = cid;
    id_ = id;
  }
  virtual int unit_type() const { return scene_unit_obj::DROPPED_ITEM; }
  virtual bool can_block() const { return false; }
  virtual const char *name()
  {
    static char name[32] = {0};
    ::snprintf(name, sizeof(name), "%d:%d", this->cid_, this->id_ % 1000);
    return name;
  }
};
class effect_obj : public scene_item
{
public:
  effect_obj(const int cid, const int id)
    : scene_item(cid, id)
  { }
  virtual int unit_type() const { return scene_unit_obj::EFFECT; }
};
#endif // SCENE_UNIT_H_

