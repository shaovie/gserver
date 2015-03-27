// -*- C++ -*-

//========================================================================
/**
 * Author   : alvinhu
 * Date     : 2012-12-27 16:07
 */
//========================================================================

#ifndef MAIL_CONFIG_H_
#define MAIL_CONFIG_H_

#include "singleton.h"
#include "global_macros.h"

#define MAIL_CFG_PATH                            "mail.json"

// 替换符
#define MAIL_P_ITEM                              "#item"
#define MAIL_P_NUMBER                            "#number"
#define MAIL_P_MONEY_TYPE                        "#money_type"
#define MAIL_P_DATE                              "#date"
#define MAIL_P_ACTIVITY                          "#activity"
#define MAIL_P_TITLE                             "#title"
#define MAIL_P_DATE_TIME                         "#datetime"
#define MAIL_P_CHAR                              "#char"

// Forward declarations
class mail_config_impl;

/**
 * @class mail_obj
 *
 * @brief
 */
class mail_obj
{
public:
  mail_obj();
public:
  char sender_name_[MAX_NAME_LEN + 1];      // 发件人名字
  char title_[MAX_MAIL_TITLE_LEN + 1];      // 邮件主题
  char content_[MAX_MAIL_CONTENT_LEN + 1];  // 邮件内容
};
/**
 * @class mail_config
 *
 * @brief
 */
class mail_config : public singleton<mail_config>
{
  friend class singleton<mail_config>;
public:
  enum mail_cfg_id
  {
    RECHARGE_INFORM                 = 1,
    MARKET_TIME_OVER_MAIL           = 2,
    MARKET_SALE_MAIL                = 3,
    TUI_TU_MST_DROP                 = 4,
    PACKAGE_IS_FULL                 = 5,
    RECHARGE_REBATE                 = 6,
    MAIL_JJ_RANK_AWARD              = 7,
    MAIL_JJ_FRIST_RANK_AWARD        = 8,
    GHZ_OVER_AWARD                  = 9,
    TUI_TU_TURN_AWARD               = 10,
    CHENG_JIU_LUCKY_EGG_AWARD       = 11,
    LTIME_ACC_RECHARGE              = 12,
    LTIME_DAILY_RECHARGE            = 13,
    LTIME_ACC_CONSUME               = 14,
    LTIME_DAILY_CONSUME             = 15,
    LUCKY_TURN_RANK_AWARD           = 16,
    GUILD_EXPEL_MEM                 = 17,
    LTIME_ACC_RECHARGE_OPENED       = 18,
    LTIME_DAILY_RECHARGE_OPENED     = 19,
    LTIME_ACC_CONSUME_OPENED        = 20,
    LTIME_DAILY_CONSUME_OPENED      = 21,
    LTIME_TURN_OPENED               = 22,
    LTIME_TIMES_EXP_OPENED          = 23,
    LTIME_TIMES_DROP_OPENED         = 24,
    RECHARGE_MC_INFORM              = 25,
    MC_DISABLED                     = 26,
    WATER_TREE_LEVEL_UP_ALL         = 27,
    WATER_TREE_LEVEL_UP_ONE         = 28,
    GHZ_WINNER_AWARD                = 29,
    XSZC_WIN                        = 30,
    XSZC_FAIL                       = 31,
    XSZC_DRAW                       = 32,
    LUEDUO_PLUNDER                  = 33,
    KAI_FU_ACT_ZHAN_LI_RANK         = 34,
    KAI_FU_ACT_LVL_RANK             = 35,
    KAI_FU_ACT_JING_JI_RANK         = 36,
    KAI_FU_ACT_TUI_TU_RANK          = 37,
    KAI_FU_ACT_GUILD_RANK           = 38,
    KAI_FU_ACT_GUILD_RANK_FOR_LEADER= 39,
    KAI_FU_ACT_ALL_QH_LVL           = 40,
    KAI_FU_ACT_ALL_PSKILL_LVL       = 41,
    KAI_FU_ACT_ALL_BAO_SHI_LVL      = 42,
    KAI_FU_ACT_ALL_EQUIP_DJ         = 43,
    KAI_FU_ACT_ZHAN_LI              = 44,
    KAI_FU_ACT_LVL_UP               = 45,
    KAI_FU_ACT_TUI_TU_CHAPTER       = 46,
    KAI_FU_ACT_ALL_TIAN_FU_LVL      = 47,
    KAI_FU_ACT_ALL_FA_BAO_LVL       = 48,

    MAIL_IDX_END
  };
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const mail_obj *get_mail_obj(const int mail_cfg_id);
private:
  mail_config();

  mail_config_impl *impl_;
};
#endif // MAIL_CONFIG_H_

