// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-14 14:26
 */
//========================================================================

#ifndef BEHAVIOR_ID_H_
#define BEHAVIOR_ID_H_

#define MAX_BEHAVIOR_LOG_ID                     999
//
#define BL_CHAR_LOGIN                           110
#define BL_CHAR_LOGOUT                          111

#define BL_ITEM_GOT                             112
# define ITEM_GOT_PICKUP                    1001      // 拾取
# define ITEM_GOT_TASK                      1002      // 任务奖励
# define ITEM_GOT_MAIL                      1003      // 邮件提取
# define ITEM_GOT_TRADE                     1004      // 角色交易
# define ITEM_GOT_NPC_TRADE                 1005      // NPC购买
# define ITEM_GOT_GIFT                      1006      // 开礼包
# define ITEM_GOT_EQUIP_FEN_JIE             1007      // 装备分解
# define ITEM_GOT_EQUIP_RONG_HE             1008      // 装备融合
# define ITEM_GOT_MARKET_BUY                1009      // 拍卖行购买
# define ITEM_GOT_MARKET_CANCEL             1010      // 拍卖行取消出售
# define ITEM_GOT_ONLINE_AWARD              1011      // 在线奖励
# define ITEM_GOT_LOGIN_AWARD               1012      // 登陆奖励
# define ITEM_GOT_GUILD_JU_BAO              1013      // 公会聚宝
# define ITEM_GOT_TUI_TU_FIRST_AWARD        1014      // 推图首次奖励
# define ITEM_GOT_TUI_TU_TURN               1015      // 推图转盘
# define ITEM_GOT_ZHAN_XING                 1016      // 占星
# define ITEM_GOT_VIP_LVL_AWARD             1017      // vip等级奖励
# define ITEM_GOT_CHENG_JIU                 1018      // 成就
# define ITEM_GOT_LVL_AWARD                 1019      // 等级奖励
# define ITEM_GOT_LUCKY_TURN                1020      // 幸运转盘
# define ITEM_GOT_PRODUCE                   1021      // 制造生产
# define ITEM_GOT_ACTIVATION_CODE           1022      // 激活码兑换
# define ITEM_GOT_SEVEN_DAY_LOGIN           1023      // 七天登陆
# define ITEM_GOT_WATER_TREE                1024      // 浇树
# define ITEM_GOT_HUO_YUE_DU                1025      // 活跃度
# define ITEM_GOT_FIRST_RECHARGE            1026      // 首充奖励
# define ITEM_GOT_WEI_XIN_SHARED            1027      // 微信分享
# define ITEM_GOT_LUEDUO                    1028      // 掠夺
# define ITEM_GOT_LUEDUO_FANPAI             1029      // 掠夺后翻牌
# define ITEM_GOT_FA_BAO_HE_CHENG           1030      // 法宝合成
# define ITEM_GOT_WATER_TREE_RANDOM_AWARD   1031      // 灌溉随机奖励
# define ITEM_GOT_GOODS_LUCKY_TURN          1032      // 实物奖励幸运转盘
# define ITEM_GOT_GOODS_LUCKY_DRAW          1033      // 实物奖励日常幸运抽奖

#define BL_ITEM_LOSE                            113
# define ITEM_LOSE_USE                      1001      // 道具使用
# define ITEM_LOSE_DESTROY                  1002      // 销毁
# define ITEM_LOSE_TASK                     1003      // 任务扣除
# define ITEM_LOSE_MAIL                     1004      // 发送邮件
# define ITEM_LOSE_TRADE                    1005      // 角色交易
# define ITEM_LOSE_NPC_TRADE                1006      // NPC交易
# define ITEM_LOSE_UPGRADE_PASSIVE_SKILL    1007      // 被动技能升级
# define ITEM_LOSE_DROP_TO_SCENE            1008      // 场景掉落
# define ITEM_LOSE_EQUIP_STRENGTHEN         1009      // 装备强化
# define ITEM_LOSE_EQUIP_FEN_JIE            1010      // 装备分解
# define ITEM_LOSE_EQUIP_RONG_HE            1011      // 装备融合
# define ITEM_LOSE_MARKET_SALE              1012      // 拍卖行出售
# define ITEM_LOSE_EQUIP_XI_LIAN            1013      // 装备洗练
# define ITEM_LOSE_BAO_SHI_ACTIVATE         1014      // 宝石激活
# define ITEM_LOSE_BAO_SHI_UPGRADE          1015      // 宝石升级
# define ITEM_LOSE_PRODUCE                  1016      // 制造其他物品
# define ITEM_LOSE_WATER_TREE               1017      // 浇树
# define ITEM_LOSE_LUEDUO                   1018      // 掠夺
# define ITEM_LOSE_FA_BAO_TAKEUP            1019      // 法宝穿戴
# define ITEM_LOSE_FA_BAO_HE_CHENG          1020      // 法宝合成
# define ITEM_LOSE_EQUIP_JIN_JIE            1021      // 装备进阶

#define BL_MONEY_GOT                            114
# define MONEY_GOT_GM                       1001      // GM
# define MONEY_GOT_USE_ITEM                 1002      // 使用道具
# define MONEY_GOT_RECHARGE                 1003      // 充值
# define MONEY_GOT_MAIL                     1004      // 邮件提取
# define MONEY_GOT_TRADE                    1005      // 玩家交易
# define MONEY_GOT_NPC_TRADE                1006      // NPC出售
# define MONEY_GOT_TASK                     1007      // 任务奖励
# define MONEY_GOT_PICK_UP                  1008      // 掉落拾取
# define MONEY_GOT_MARKET_BUY               1009      // 市场购买
# define MONEY_GOT_MARKET_SAIL              1010      // 市场寄售
# define MONEY_GOT_CANCEL_MARKET_SAIL       1011      // 取消市场寄售
# define MONEY_GOT_TUI_TU_FIRST_AWARD       1012      // 推图首次奖励
# define MONEY_GOT_TUI_TU_TURN              1013      // 推图转盘
# define MONEY_GOT_ZHAN_XING                1014      // 占星
# define MONEY_GOT_FIRST_JING_JI_RANK       1015      // 首次达到竞技场排名
# define MONEY_GOT_DIAMOND_TO_COIN          1016      // 钻石换金币
# define MONEY_GOT_MC_REBATE                1017      // 月卡返利
# define MONEY_GOT_LUCKY_TURN               1018      // 幸运转盘
# define MONEY_GOT_RECHARGE_REBATE          1019      // 充值返利
# define MONEY_GOT_ACTIVATION_CODE          1020      // 激活码兑换
# define MONEY_GOT_WROSHIP                  1021      // 膜拜王族
# define MONEY_GOT_BE_WROSHIPED             1022      // 被膜拜
# define MONEY_GOT_WATER_TREE               1023      // 浇树
# define MONEY_GOT_VIP_EXCHANGE_COIN        1024      // VIP金币兑换
# define MONEY_GOT_LUEDUO                   1025      // 掠夺奖励
# define MONEY_GOT_LUEDUO_FANPAI            1026      // 掠夺成功后翻牌奖励
# define MONEY_GOT_SCP                      1027      // 副本获取

#define BL_MONEY_LOSE                           115
# define MONEY_LOSE_NPC_TRADE               1001      // NPC交易
# define MONEY_LOSE_ENLARGE_CAPACITY        1002      // 扩展背包/仓库
# define MONEY_LOSE_MAIL                    1003      // 邮费
# define MONEY_LOSE_MAIL_ATTACH             1004      // 邮寄金钱
# define MONEY_LOSE_TRADE                   1005      // 玩家间交易
# define MONEY_LOSE_MARKET_SALE_COST        1006      // 拍卖行寄售手续费
# define MONEY_LOSE_MARKET_SALE             1007      // 拍卖行寄售金钱
# define MONEY_LOSE_UPGRADE_SKILL           1008      // 升级主动技能
# define MONEY_LOSE_UPGRADE_PASSIVE_SKILL   1009      // 升级被动技能
# define MONEY_LOSE_DROP_TO_SCENE           1010      // 死亡掉落
# define MONEY_LOSE_RELIVE                  1011      // 角色复活
# define MONEY_LOSE_EQUIP_STRENGTHEN        1012      // 装备强化
# define MONEY_LOSE_EQUIP_FEN_JIE           1013      // 装备分解
# define MONEY_LOSE_EQUIP_RONG_HE           1014      // 装备融合
# define MONEY_LOSE_GUILD_CREATE            1015      // 创建公会
# define MONEY_LOSE_MARKET_BUY              1016      // 拍卖行购买
# define MONEY_LOSE_GUILD_JUAN_XIAN         1017      // 公会捐献
# define MONEY_LOSE_GUILD_JU_BAO            1018      // 公会聚宝
# define MONEY_LOSE_EQUIP_XI_LIAN           1019      // 装备洗炼
# define MONEY_LOSE_TUI_TU_TURN_ALL         1020      // 推图转盘获得所有道具
# define MONEY_LOSE_BAO_SHI_ACTIVATE        1021      // 宝石激活
# define MONEY_LOSE_BAO_SHI_UPGRADE         1022      // 宝石升级
# define MONEY_LOSE_JING_JI_CLEAN_TIME      1023      // 竞技场重置时间
# define MONEY_LOSE_ACTIVATE_SHOU_WEI       1024      // 公会战激活守卫
# define MONEY_LOSE_DIAMOND_TO_COIN         1025      // 钻石换金币
# define MONEY_LOSE_LUCKY_TURN              1026      // 幸运转盘
# define MONEY_LOSE_PRODUCE                 1027      // 生产制造
# define MONEY_LOSE_BACK_TOWN               1028      // 快速回城
# define MONEY_LOSE_UPGRADE_TIANFU_SKILL    1029      // 升级天赋技能
# define MONEY_LOSE_BUY_TI_LI               1030      // 购买体力
# define MONEY_LOSE_GUILD_SUMMON_MEM        1031      // 公会召唤成员
# define MONEY_LOSE_VIP_EXCHANGE_COIN       1032      // VIP金币兑换
# define MONEY_LOSE_ZHAN_XING               1033      // 占星
# define MONEY_LOSE_FA_BAO_HE_CHENG         1034      // 法宝合成
# define MONEY_LOSE_REFRESH_BAOWU_MALL      1035      // 刷新宝物商城
# define MONEY_LOSE_GM                      1036      // GM删除
# define MONEY_LOSE_EQUIP_JIN_JIE           1037      // 装备进阶

#define BL_TASK_ACCEPT                          118
#define BL_TASK_SUBMIT                          119

#define BL_RECHARGE                             121

#define BL_ENTER_SCP                            190
#define BL_JOIN_ACTVITY                         191

#define BL_ZHAN_LI                              901
# define ZHAN_LI_EQUIP_TAKEON               1001      // 穿戴装备
# define ZHAN_LI_EQUIP_TAKEDOWN             1002      // 卸下装备
# define ZHAN_LI_SKILL                      1003      // 主动技能
# define ZHAN_LI_PSKILL                     1004      // 被动技能
# define ZHAN_LI_GSKILL                     1005      // 公会技能
# define ZHAN_LI_LVLUP                      1006      // 角色升级
# define ZHAN_LI_TITLE                      1007      // 称号
# define ZHAN_LI_EQUIP_BAO_SHI              1008      // 宝石
# define ZHAN_LI_EQUIP_STRENGTHEN           1009      // 装备强化
# define ZHAN_LI_EQUIP_RONG_HE              1010      // 装备融合
# define ZHAN_LI_EQUIP_XI_LIAN              1011      // 装备洗炼
# define ZHAN_LI_EQUIP_XI_LIAN_REPLACE      1012      // 装备洗炼替换
# define ZHAN_LI_TAKEUP_FA_BAO              1013      // 穿戴法宝
# define ZHAN_LI_FA_BAO_JIN_JIE             1014      // 法宝进阶
# define ZHAN_LI_EQUIP_JIN_JIE              1015      // 装备进阶

#define BL_ONLINE                               999

#endif // BEHAVIOR_ID_H_
