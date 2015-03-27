// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-28 19:55
 */
//========================================================================

#ifndef ERROR_H_
#define ERROR_H_

// = 全局错误码
#define ERR_UNKNOWN                              -1000  // 未知错误
#define ERR_CLIENT_OPERATE_ILLEGAL               -1001  // 客户端数据异常
#define ERR_CONFIG_NOT_EXIST                     -1002  // 配置文件出错
#define ERR_SAVE_DATA_FAILED                     -1003  // 服务器保存数据失败
#define ERR_GET_PLAYER_DATA_FAILED               -1004  // 获取玩家数据失败
#define ERR_SERVER_INTERNAL_COMMUNICATION_FAILED -1005  // 服务器内部通信失败
#define ERR_OPERATION_TOO_FAST                   -1006  // 您的操作太频繁，请稍后再试
#define ERR_INPUT_IS_ILLEGAL                     -1007  // 您输入的内容包含非法字符
#define ERR_PLAYER_IS_OFFLINE                    -1008  // 玩家不在线
#define ERR_ITEM_NOT_EXIST                       -1009  // 道具不存在
#define ERR_CLIENT_NETWORK_UNSTABLE              -1010  // 您的网络不稳定，请重新登录
#define ERR_CLIENT_STATE_ERROR                   -1011  // 客户端状态错误
#define ERR_LVL_NOT_ENOUGH                       -1012  // 等级不足

// ===================================== world ======================================
#define ERR_LOGIN_AC_IS_ILLEGAL                  -2001  // 帐户非法
#define ERR_LOGIN_AC_AUTH_FAILED                 -2002  // 登录认证失败
#define ERR_LOGIN_REPEADED                       -2003  // 帐号在其他地方登录
#define ERR_CHAR_NAME_EXIST                      -2004  // 角色名已存在
#define ERR_CAN_NOT_CREATE_CHAR                  -2005  // 抱歉，本服角色已满，不能再创建
#define ERR_ASSIGN_SVC_FAILED                    -2006  // 分配游戏服务器失败
#define ERR_OVER_MAX_PAYLOAD                     -2007  // 该服务器人数已满
#define ERR_CHAR_IS_DEAD                         -2008  // 角色已经死亡
#define ERR_COORD_IS_ILLEGAL                     -2009  // 坐标非法
#define ERR_CAN_NOT_MOVE                         -2010  // 不能移动
#define ERR_CAN_NOT_BE_MOVED                     -2011  // 目标不能被移动
#define ERR_GM_PARAM_INVALID                     -2012  // GM命令输入参数错误
#define ERR_COORD_IS_CAN_NOT_MOVE                -2013  // 坐标不可移动
#define ERR_CAN_NOT_TRANSFER                     -2014  // 不能传送
#define ERR_CAN_NOT_TRANSFER_IN_FIGHTING         -2015  // 您现在处于战斗状态，不能传送
#define ERR_LOGIN_TIMEOUT                        -2016  // 登录游戏超时，请重新登录
#define ERR_FORBID_LOGIN                         -2017  // 您已被禁止登录游戏

// social
#define ERR_OVER_MAX_SOCIAL_F_SIZE               -2020  // 您的好友数量已到上限
#define ERR_DST_OVER_MAX_SOCIAL_F_SIZE           -2021  // 对方好友数量已到上限
#define ERR_HAS_BEEN_FRIEND                      -2022  // 对方已经是您的好友
#define ERR_HAS_BEEN_BLACK                       -2023  // 已经在黑名单
#define ERR_OVER_MAX_SOCIAL_B_SIZE               -2024  // 您的黑名单数量已到上限
#define ERR_NOT_HAVE_CHAR                        -2025  // 没有该玩家
#define ERR_OVER_MAX_BE_BLESSED_COUNT            -2026  // 该玩家本次接受祝福已满
#define ERR_OVER_MAX_BLESSED_COUNT               -2027  // 今日祝福次数已满
#define ERR_IN_THE_OTHER_BALCK                   -2028  // 你在对方黑名单中

// team
#define ERR_SELF_HAVE_TEAM                       -2040  // 您已经有队伍
#define ERR_SELF_NOT_HAVE_TEAM                   -2041  // 您还没有队伍
#define ERR_OPP_HAVE_TEAM                        -2042  // 对方已经有队伍
#define ERR_OPP_NOT_HAVE_TEAM                    -2043  // 对方已没有队伍
#define ERR_NOT_LEADER                           -2044  // 您不是队长
#define ERR_TEAM_FULL                            -2045  // 队伍已满员

// task
#define ERR_TASK_CAN_NOT_ACCEPT                  -2050  // 任务不可接受
#define ERR_TASK_DIST_TOO_FAR                    -2051  // 距离任务NPC过远
#define ERR_TASK_ADD_2_MAP_FAIL                  -2052  // 添加至任务列表失败
#define ERR_TASK_NOT_EXIST                       -2053  // 任务不存在
#define ERR_TASK_CONFIG_ERR                      -2054  // 任务配置文件错误
#define ERR_TASK_EXEC_ACCEPT_FAIL                -2055  // 执行任务接受命令失败
#define ERR_TASK_NOT_ACCEPTED                    -2056  // 任务尚未接受
#define ERR_TASK_CAN_NOT_SUBMIT                  -2057  // 任务尚未完成
#define ERR_TASK_EXEC_COMPLETE_FAIL              -2058  // 执行任务提交命令失败
#define ERR_TASK_HAD_ACCEPTED                    -2059  // 任务已经接受

#define ERR_CAN_NOT_USE_ITEM                     -2070  // 不能使用此道具
#define ERR_ITEM_CD_LIMIT                        -2071  // 道具冷却中
#define ERR_PACKAGE_SPACE_NOT_ENOUGH             -2072  // 背包空间不足
#define ERR_ITEM_AMOUNT_NOT_ENOUGH               -2073  // 物品数量不足
#define ERR_NOT_FOUND_IN_CONFIG                  -2074  // 配置文件中不存在
#define ERR_CAPACITY_OUT_OF_LIMIT                -2075  // 容量已达上限
#define ERR_HP_IS_FULL                           -2076  // 生命已满
#define ERR_MP_IS_FULL                           -2077  // 法力已满
#define ERR_CAN_NOT_TAKEUP_THIS_EQUIP            -2078  // 您不能穿戴此装备
#define ERR_SIN_VAL_IS_EMPTY                     -2079  // 罪恶值已经为零
#define ERR_DROPPED_ITEM_NOT_EXIST               -2080  // @掉落的道具不存在
#define ERR_DROPPED_ITEM_NOT_YOURS               -2081  // 掉落的道具不属于您

#define ERR_CAN_NOT_USE_SKILL                    -2089  // 不能使用此技能
#define ERR_MP_NOT_ENOUGH                        -2090  // 您的法力不足
#define ERR_SKILL_OUT_OF_DISTANCE                -2091  // 技能施放距离不满足条件
#define ERR_SKILL_CD_LIMIT                       -2092  // 技能冷却时间未到
#define ERR_SKILL_NOT_EXIST                      -2093  // 技能不存在
#define ERR_SKILL_TARGET_ILLEGAL                 -2094  // 技能目标非法
#define ERR_SKILL_TARGET_NOT_EXIST               -2095  // 技能目标不存在
#define ERR_CAN_NOT_ATTACK_TARGET                -2096  // 目标不可攻击
#define ERR_BUFF_EXIST                           -2097  // 状态已存在
#define ERR_BUFF_CAN_NOT_DJ                      -2098  // 状态不可叠加
#define ERR_CAN_NOT_FIGHT_IN_PEACE_MODE          -2099  // 和平模式无法造成伤害
#define ERR_CAN_NOT_FIGHT_IN_PEACE_SCENE         -2100  // 此地图不可以PK
#define ERR_TEAM_MEMBER_CAN_NOT_ATTACK           -2101  // 不能攻击您的队友
#define ERR_GUILD_MEMBER_CAN_NOT_ATTACK          -2102  // 同一公会成员不能PK
#define ERR_FRIENDS_CAN_NOT_ATTACK               -2103  // 友方不可攻击
#define ERR_CAN_NOT_ATTACK_TARGET_2              -2104  // @目标不可攻击

#define ERR_DIAMOND_NOT_ENOUGH                   -2110  // 您的钻石不足
#define ERR_B_DIAMOND_NOT_ENOUGH                 -2111  // 您的绑定钻石不足
#define ERR_COIN_NOT_ENOUGH                      -2112  // 您的金币不足
#define ERR_MONEY_UPPER_LIMIT                    -2113  // 超过金钱上限
#define ERR_MALL_BUY_OUT_OF_LIMIT                -2114  // 今日已经达到购买上限，明日再来哦
#define ERR_ZHAN_XING_V_NOT_ENOUGH               -2115  // 占星点不足
#define ERR_CAN_NOT_BUY_ITEM                     -2116  // 您不能购买此道具
#define ERR_CHENG_JIU_NOT_ENOUGH                 -2117  // 成就没有达到
#define ERR_HUO_YUE_DU_NOT_ENOUGH                -2118  // 活跃度没有达到
#define ERR_XSZC_HONOR_NOT_ENOUGH                -2119  // 战场荣誉值不足

#define ERR_PASSIVE_UPGRADE_FAILED               -2131  // 技能升级失败
#define ERR_CAN_NOT_CHANGE_PK_MODE               -2132  // 不能切换PK模式
#define ERR_ITEM_CAN_NOT_SELL                    -2133  // 此道具不可出售
#define ERR_TILI_IS_FULL                         -2134  // 体力值已满

// IU
#define ERR_CAI_LIAO_NOT_ENOUGH                  -2140  // 材料不足
#define ERR_EQUIP_STRENGTHEN_FAILED              -2141  // @装备强化失败

// market
#define ERR_MARKET_NOT_HAVE_ITEM                 -2150  // 拍卖行中没有该物品
#define ERR_MARKET_FULL                          -2151  // 拍卖行货物已满，不能再寄售
#define ERR_MARKET_CLOSED                        -2152  // 拍卖行暂时不可用，请谅解

// lue duo
#define ERR_TARGET_DONT_HAVE_MATERIAL            -2160  // 对方没有这种材料了
#define ERR_JING_LI_NOT_ENOUGH                   -2161  // 精力不足

// mail 2200 - 2299
#define ERR_MAIL_RECEIVER_IS_SELF                -2201  // 收件人不能为自己
#define ERR_DAILY_MAIL_TOO_MUCH                  -2202  // 您今天的邮件发的过多
#define ERR_MAIL_ATTACHMENT                      -2203  // 附件异常
#define ERR_MAIL_CLOSED                          -2204  // 邮件系统暂时不可用，请谅解

// trade 2300 - 2399
#define ERR_TRADE_OPP_STATE_IN                   -2301  // 对方交易中
#define ERR_TRADE_SELF_STATE_IN                  -2302  // 您在交易中
#define ERR_TRADE_ITEM_NEED_IN_PACKAGE           -2303  // 只有背包中的道具才能交易
#define ERR_TRADE_OPP_NO_LOCK                    -2304  // 对方尚未锁定
#define ERR_BIND_ITEM_CAN_NOT_SELL               -2305  // 此道具不能出售

// guild 2400 - 2500
#define ERR_GUILD_CHAR_HAD                       -2401  // 您已有公会
#define ERR_GUILD_CHAR_NO_HAD                    -2402  // 您没有公会
#define ERR_GUILD_HAD_APPLY                      -2403  // 您已申请过
#define ERR_GUILD_CHAR_NO_EXIST                  -2404  // 查无此人
#define ERR_GUILD_NO_HAD_APPLY                   -2405  // 您尚未申请
#define ERR_GUILD_NAME_EXIST                     -2406  // 公会名已存在
#define ERR_GUILD_NO_EXIST                       -2407  // 公会不存在
#define ERR_GUILD_MEM_FULL                       -2408  // 成员已满
#define ERR_GUILD_RIGHT_NO_HAD                   -2409  // 您没有权限
#define ERR_GUILD_NO_ENOUGH_ZHAN_LI              -2410  // 战力不足
#define ERR_GUILD_RESOURCE_LACK                  -2411  // 公会资金不足
#define ERR_GUILD_DA_TING_LVL_LACK               -2412  // 请升级公会大厅
#define ERR_GUILD_JU_BAO_CNT_LACK                -2413  // 聚宝次数用完
#define ERR_GUILD_ALREADY_IN_ZHU_DI              -2414  // 您已在驻地中
#define ERR_GUILD_SUMMON_WDAY                    -2415  // 未到召唤时间
#define ERR_GUILD_TODAY_HAD_SUMMON               -2416  // 今日已召唤
#define ERR_GUILD_ZHU_DI_NOT_OPEN                -2417  // 公会驻地未开启
#define ERR_GUILD_CONTIRB_LACK                   -2418  // 贡献不足
#define ERR_GUILD_ZHNA_QI_LVL_LACK               -2419  // 请升级战旗
#define ERR_GUILD_DT_LVL_IS_MAX                  -2420  // 大厅等级已最高
#define ERR_CALL_BOSS_IN_ZHU_DI                  -2421  // 只有在公会驻地才可以使用
#define ERR_MAP_CANT_GUILD_SUMMON_MEM            -2422  // 此地图不可召唤
#define ERR_GUILD_SCP_OVER                       -2423  // 副本已结束
#define ERR_GUILD_SCP_TODAY_HAD_ENTER            -2424  // 今日已参加过

// mis_award 2501 - 2520
#define ERR_AWARD_NO_REACH_TIME                  -2501  // 时间未到
#define ERR_AWARD_HAD_GET                        -2502  // 今日已领取
#define ERR_AWARD_OVER_GET                       -2503  // 已领取完

// vip 2521 - 2530
#define ERR_NOT_VIP                              -2521  // 您不是VIP
#define ERR_VIP_LVL_LACK                         -2522  // VIP等级不足，充值可提升等级

// bao_shi 2531 - 2540
#define ERR_BAO_SHI_ACTIVATE_HAD                 -2531  // 您已激活
#define ERR_BAO_SHI_ACTIVATE_LAST_ROW            -2532  // 请先激活前一列所有宝石
#define ERR_BAO_SHI_ACTIVATE_NO                  -2533  // 尚未激活
#define ERR_BAO_SHI_UPGRADE_LAST_ROW             -2534  // 等级不能超过前一列宝石的最低等级
#define ERR_BAO_SHI_REACH_MAX_LVL                -2535  // 已达到最大等级

// scp
#define ERR_CAN_NOT_ENTER_SCP                    -2551  // 您暂时不能进入副本，请稍候重试
#define ERR_SCP_NOT_OPENED                       -2552  // 副本未开启
#define ERR_SCP_IS_INVALID                       -2553  // 副本暂未开放
#define ERR_SCP_NOT_LEADER                       -2554  // 只有队长才能创建副本
#define ERR_SCP_CREATE_FAILED                    -2555  // 创建副本失败
#define ERR_SCP_ENTER_CNT_OUT_LIMIT              -2556  // 您今日的副本次数已用完

// di xia cheng
#define ERR_DXC_NORMAL_ENTER_CNT_OUT_LIMIT       -2565  // 您今日挑战次数已用完
#define ERR_DXC_HARD_ENTER_CNT_OUT_LIMIT         -2566  // 您今日挑战次数已用完
#define ERR_DXC_SCP_CREATE_FAILED                -2567  // 创建地下城副本失败
#define ERR_ZHANLI_NOT_ENOUGH                    -2568  // 战力不足

// tui tu
#define ERR_TUI_TU_SCP_CREATE_FAILED             -2571  // 创建关卡副本失败
#define ERR_TI_LI_NOT_ENOUGH                     -2572  // 您的体力值不足
#define ERR_TUI_TU_TURN_OVER                     -2573  // 转盘结束
#define ERR_TUI_TU_NOT_EXIST                     -2574  // 您没有进入过此关卡
#define ERR_TUI_TU_TURN_FREE_CNT_NOT_ENOUGH      -2575  // 您的免费次数已经用完

// jing ji
#define ERR_JING_JI_NO_COMPETITOR                -2591  // 系统没有为您匹配到合适的对手
#define ERR_COMPETITOR_RANK_CHANGE               -2592  // 对手排行改变，请选择其他对手或重新匹配
#define ERR_COMPETITOR_ON_JING_JI                -2593  // 对手正在竞技中
#define ERR_OVER_JING_JI_COUNT                   -2594  // 挑战竞技场次数已用完
#define ERR_JING_JI_IN_CD                        -2595  // 挑战冷却时间未到
#define ERR_NOW_SEND_RANK_AWARD                  -2596  // 奖励发放中不能挑战
#define ERR_CAN_NOT_JING_JI_IN_THIS_SCENE        -2597  // 在此场景不能参与竞技
#define ERR_JING_JI_SCORE_NOT_ENOUGH             -2598  // 竞技积分不足
#define ERR_JING_JI_CLOSED                       -2599  // 竞技场暂时不可用，请谅解

// activity
#define ERR_ACTIVITY_NOT_OPENED                  -2601  // 活动暂未开放
#define ERR_GHZ_PREPARING                        -2602  // 公会战正在准备阶段，您还不能进入！
#define ERR_HAD_GOT_AWARD                        -2603  // 您已领取过奖励
#define ERR_SHOU_WEI_IS_ACTIVITED                -2604  // 通灵塔目前处于被激活状态，不能购买！
#define ERR_XSZC_IS_END                          -2605  // 该雄狮战场已结束，请选择其他战场进入
#define ERR_OVER_PLAYER_COUNT                    -2606  // 人数已满
#define ERR_5_MIN_CANNOT_ENTER                   -2607  // 退出后5分钟内不能进入

// recharge
#define ERR_TIME_NOT_TO_GET                      -2610  // 还没到领取返利的时间
#define ERR_DONT_HAVE_MC                         -2611  // 您还没有购买月卡或已过期
#define ERR_HAVE_GOT_TODAYS                      -2612  // 您已经领取了今日的返利

#define ERR_EXCHANGE_KEY_ERROR                   -2615  // 您的兑换码无效

// chat
#define ERR_CHAT_LEVEL_NOT_ENOUGH                -3101  // 您的等级不够聊天
#define ERR_CHAT_NO_TEAM                         -3102  // 没有队伍
#define ERR_CHAT_NO_GUILD                        -3103  // 您没有公会
#define ERR_CHAT_TARGET_OFFLINE                  -3104  // 对方不在线
#define ERR_CHAT_TOO_FAST                        -3105  // 您的发言太快
#define ERR_CHAT_CAN_NOT_CHAT                    -3106  // 您不能发言
#define ERR_CHAT_FORBID_TALK                     -3107  // 您已被禁止发言

#endif // ERROR_H_

