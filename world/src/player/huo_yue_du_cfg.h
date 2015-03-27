// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-05-04 15:27
 */
//========================================================================

#ifndef HUO_YUE_DU_CFG_H_
#define HUO_YUE_DU_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define HUO_YUE_DU_CFG_PATH                "huoyue.json"

class huo_yue_du_cfg_impl;

enum
{
  HYD_LOGIN_AWARD             = 1,
  HYD_EQUIP_QH                = 2,
  HYD_EQUIP_FEN_JIE           = 3,
  HYD_EQUIP_XI_LIAN           = 4,
  HYD_UPGRADE_BAO_SHI         = 5,
  HYD_UPGRADE_PASSIVE_SKILL   = 6,

  HYD_QH_SHI_SCP              = 7,
  HYD_SKILL_FU_SCP            = 8,
  HYD_XI_LIAN_SHI_SCP         = 9,
  HYD_BAO_SHI_JING_HUA_SCP    = 10,
  HYD_COIN_SCP                = 11,
  HYD_EXP_SCP                 = 12,

  HYD_JING_JI                 = 13,
  HYD_GUILD_JU_BAO            = 14,
  HYD_MO_BAI                  = 15,
  HYD_WATER_TREE              = 16,

  HYD_END
};

/**
 * @class huo_yue_du_cfg
 *
 * @brief
 */
class huo_yue_du_cfg : public singleton<huo_yue_du_cfg>
{
  friend class singleton<huo_yue_du_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int get_param(const int huo_yue_du_cid);
  int get_total_score(const int huo_yue_du_cid);
  int get_per_score(const int huo_yue_du_cid);
  ilist<item_amount_bind_t> *get_award(const int huo_yue_du_cid);
public:
  huo_yue_du_cfg();

  huo_yue_du_cfg_impl *impl_;
};

#endif  // HUO_YUE_DU_CFG_H_
