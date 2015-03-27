// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-22 10:19
 */
//========================================================================

#ifndef LUCKY_GOODS_TURN_CFG_H_
#define LUCKY_GOODS_TURN_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class lucky_goods_turn_cfg_impl;

#define LUCKY_GOODS_TURN_CFG_PATH             "lucky_goods_turntable.json"

class lucky_goods_turn_cfg_obj
{
public:
  char notify_;
  int  rate_;
  item_amount_bind_t award_item_;
};
/**
 * @class lucky_goods_turn_cfg
 *
 * @brief
 */
class lucky_goods_turn_cfg : public singleton<lucky_goods_turn_cfg>
{
  friend class singleton<lucky_goods_turn_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<lucky_goods_turn_cfg_obj *> * get_cfg_list();
private:
  lucky_goods_turn_cfg();

  lucky_goods_turn_cfg_impl *impl_;
};

#endif // LUCKY_GOODS_TURN_CFG_H_

