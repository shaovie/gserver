// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-17 17:39
 */
//========================================================================

#ifndef VIP_CONFIG_H_
#define VIP_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define VIP_CFG_PATH                        "vip_fun.json"
#define VIP_PURCHASE_CFG_PATH               "vip_purchase.json"

// Forward declarations
class vip_config_impl;
class vip_purchase_cfg_impl;

enum vip_effect_id // id must < 127
{
  VIP_EFF_XXX                   = 0,
  VIP_EFF_ZHAN_XING_CNT         = 1,  // 赠送占星次数
  VIP_EFF_TI_LI_UPPER_LIMIT     = 2,  // 体力上限增加
  VIP_EFF_TUI_TU_PASS_EXP       = 3,  // 关卡通关经验增加
  VIP_EFF_EQUIP_QIANG_HUA_RATE  = 4,  // 装备强化成功率增加
  VIP_EFF_PASSIVE_SKILL_UP_RATE = 5,  // 被动技能提升成功率增加
  VIP_EFF_GUILD_JU_BAO_CNT      = 6,  // 公会聚宝
  VIP_EFF_TUI_TU_FREE_TURN_CNT  = 7,  // 推图转盘免费次数
  VIP_EFF_DAY_JING_JI_CNT       = 8,  // 增加每日竞技次数
  VIP_EFF_BAO_SHI_EXP_PER       = 9,  // 宝石升级经验额外加成百分比
  VIP_EFF_FRIEND_CNT            = 10, // 额外增加好友上限
  VIP_EFF_WORSHIP_CNT           = 11, // 额外增加每日膜拜次数
  VIP_EFF_TIANFU_SKILL_BAOJI_TIMES = 12, // 天赋技能升级时暴击倍数
  VIP_EFF_BUY_TI_LI             = 13, // 购买体力值次数
  VIP_EFF_EXCHANGE_COIN         = 14, // 兑换金币次数
  VIP_EFF_DXC_ENTER_ADD         = 15, // 地下城进入次数
  VIP_EFF_JING_LI_UPPER_LIMIT   = 16, // 精力上线增加数值
  VIP_EFF_FREE_RELIVE           = 17, // 精力上线增加数值
  VIP_EFF_END
};

/**
 * @class vip_config
 *
 * @brief
 */
class vip_config : public singleton<vip_config>
{
  friend class singleton<vip_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  int  lvl_up_exp(const char lvl);
  int  trade_npc_cid(const char lvl);
  bool find_trade_npc(const int npc_cid);
  char max_vip_lvl();
  int  effect_val1(const char lvl, const char effect_id);
  int  effect_val2(const char lvl, const char effect_id);
  ilist<item_amount_bind_t> *lvl_award_list(const char lvl);
  item_amount_bind_t *login_award(const char lvl, const char day);
  int  award_title(const char lvl);
private:
  vip_config();
  vip_config_impl *impl_;
};

/**
 * @class vip_purchase_cfg
 * 
 * @brief
 */
class vip_purchase_cfg : public singleton<vip_purchase_cfg>
{
public:
  friend class singleton<vip_purchase_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int ti_li(const int cnt, int &cost, int &amount);
  int coin(const int cnt, int &cost, int &amount);
private:
  vip_purchase_cfg();
  vip_purchase_cfg_impl *impl_;
};
#endif // VIP_CONFIG_H_
