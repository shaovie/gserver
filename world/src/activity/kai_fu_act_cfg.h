// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-10-31 11:57
 */
//========================================================================

#ifndef KAI_FU_ACT_CFG_H_
#define KAI_FU_ACT_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class kai_fu_act_cfg_impl;

#define KAI_FU_ACT_CFG_PATH                   "open_server.json"

class kai_fu_act_cfg_obj 
{
public:
  bool open_;
  int param_;
  ilist<item_amount_bind_t> award_list_;
  ilist<item_amount_bind_t> award_list_2_;
};
/**
 * @class kai_fu_act_cfg
 * 
 * @brief
 */
class kai_fu_act_cfg : public singleton<kai_fu_act_cfg>
{
  friend class singleton<kai_fu_act_cfg>;
public:
  enum
  {
    RANK_ZHAN_LI         = 1001,
    RANK_LVL             = 1002,
    RANK_JING_JI         = 1003,
    RANK_TUI_TU          = 1004,
    RANK_GUILD           = 1005,

    EQUIP_ALL_QH         = 2001,  // 装备全身强化
    EQUIP_JIN_JIE        = 2002,  // 所有装备进阶
    ALL_BAO_SHI_LVL      = 2003,  // 所有宝石等级
    ALL_PSKILL_LVL       = 2004,  // 所有被动技能等级
    ALL_TIAN_FU_LVL      = 2005,  // 所有天赋技能等级
    FA_BAO_DJ            = 2006,  // 法宝等阶

    ZHAN_LI              = 3001,  // 战力提升
    LVL_UP               = 3002,  // 角色等级
    TUI_TU_CHAPTER       = 3003,  // 关卡章节
  };
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<kai_fu_act_cfg_obj *> *get_cfg(const int id);
private:
  kai_fu_act_cfg_impl *impl_;

  kai_fu_act_cfg();
};

#endif // KAI_FU_ACT_CFG_H_

