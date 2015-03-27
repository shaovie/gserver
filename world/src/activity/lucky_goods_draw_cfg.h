// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-22 15:56
 */
//========================================================================

#ifndef LUCKY_GOODS_DRAW_CFG_H_
#define LUCKY_GOODS_DRAW_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define LUCKY_GOODS_DRAW_CFG_PATH               "luck_goods_draw.json"

// Forward declarations
class lucky_goods_draw_cfg_impl;

class lucky_goods_draw_cfg_obj 
{
public:
  char bind_type_;
  short rate_;
  int  min_cnt_;
  int  max_cnt_;
  int  item_cid_;
};
/**
 * @class lucky_goods_draw_cfg
 * 
 * @brief
 */
class lucky_goods_draw_cfg : public singleton<lucky_goods_draw_cfg>
{
  friend class singleton<lucky_goods_draw_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<lucky_goods_draw_cfg_obj *> *get_cfg_obj(const int cnt);
private:
  lucky_goods_draw_cfg();

  lucky_goods_draw_cfg_impl *impl_;
};

#endif // LUCKY_GOODS_DRAW_CFG_H_

