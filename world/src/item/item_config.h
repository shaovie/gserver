// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-09-27 17:45
 */
//========================================================================

#ifndef ITEM_CONFIG_H_
#define ITEM_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"
#include "global_macros.h"

#define ITEM_CFG_PATH                      "item.json"
#define EQUIP_CFG_PATH                     "equip.json"
#define FA_BAO_CFG_PATH                    "fabao.json"

// Forward declarations
class item_config_impl;

/**
 * @class item_cfg_obj
 *
 * @brief
 */
class item_cfg_obj
{
public:
  bool can_sell_;
  bool can_use_;
  char color_;
  short use_lvl_;
  int buff_id_;
  int cd_;
  int price_;
  int dj_upper_limit_;
  int effect_;                     // 效果ID
  int value_;                      // 效果值
  int market_classify_;
  char name_[MAX_ITEM_NAME_LEN + 1];
};
/**
 * @class equip_cfg_obj 
 *
 * @brief
 */
class equip_cfg_obj
{
public:
  char career_;
  char color_;
  char part_;
  char attr_float_range_;
  short use_lvl_;
  short strength_limit_;
  int wash_id_;
  int suit_id_;
  int attr_val_[ATTR_T_ITEM_CNT];
  char name_[MAX_ITEM_NAME_LEN + 1];
};
/**
 * @class fa_bao_cfg_obj 
 *
 * @brief
 */
class fa_bao_cfg_obj
{
public:
  char part_;
  char color_;
  short use_lvl_;
  int price_;
  int dj_upper_limit_;
  int hp_recover_;
  int mp_recover_;
  int add_extra_hurt_;
  int reduce_hurt_;
  int attr_rate_[ATTR_T_ITEM_CNT];
  int market_classify_;
  char name_[MAX_ITEM_NAME_LEN + 1];
};
/**
 * @class item_config
 *
 * @brief
 */
class item_config : public singleton<item_config>
{
  friend class singleton<item_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  bool find(const int item_cid);
  item_cfg_obj  *get_item(const int item_cid);
  equip_cfg_obj *get_equip(const int item_cid);
  fa_bao_cfg_obj *get_fa_bao(const int item_cid);
  int dj_upper_limit(const int );
  int market_classify(const int item_cid);

  static bool item_is_money(const int cid);
private:
  item_config();
  item_config(const item_config &);
  item_config& operator= (const item_config &);

  item_config_impl *impl_;
};

#endif  // ITEM_CONFIG_H_
