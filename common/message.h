// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-28 19:55
 */
//========================================================================

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MAJOR_ID(ID)                         ((ID) / 1000)

/*************************************************/
//= player msg list, range in 100001 ~ 199999
/*************************************************/
//* client ~ world server
#define MJ_LOGIN_MODULE                      100
#define REQ_HELLO_WORLD                      100001
#define RES_HELLO_WORLD                      100002

#define REQ_CREATE_CHAR                      100003
#define RES_CREATE_CHAR                      100004

#define REQ_START_GAME                       100005
#define RES_START_GAME                       100006

#define REQ_ENTER_GAME                       100007
#define NTF_OPERATE_RESULT                   100009

#define REQ_REENTER_GAME                     100011

#define NTF_PLAYER_LOGOUT                    100013

#define REQ_GET_HOT_NAME                     100015
#define NTF_CLT_HOT_NAME                     100017

#define MJ_BASIC_MODULE                      101
#define NTF_PUSH_SNAP_INFO                   101001

#define REQ_CLT_HEART_BEAT                   101003
#define RES_CLT_HEART_BEAT                   101004

#define REQ_GM_CMD                           101005
#define RES_GM_CMD                           101006

#define NTF_CHAR_ATTR_UPDATE                 101007

#define REQ_CHAR_MOVE                        101009
#define RES_CHAR_MOVE                        101010

#define NTF_BROADCAST_OTHER_CHAR_MOVE        101011

#define REQ_USE_SKILL                        101013
#define RES_USE_SKILL                        101014

#define NTF_BROADCAST_OTHER_USE_SKILL        101015
#define NTF_BROADCAST_HP_MP                  101017
#define NTF_BROADCAST_BE_HURT_EFFECT         101019

#define REQ_RELIVE                           101021
#define RES_RELIVE                           101022

#define NTF_BROADCAST_UNIT_LIFE_STATUS       101023

#define REQ_EXIT_TRANSFER                    101025
#define RES_EXIT_TRANSFER                    101026

#define NTF_UPDATE_EXP                       101027
#define NTF_BROADCAST_LVL_UP                 101029

#define NTF_PUSH_BUFF_LIST                   101031
#define NTF_CHAR_ADD_BUFF                    101033
#define NTF_CHAR_DEL_BUFF                    101035
#define NTF_BROADCAST_ADD_BUFF               101037
#define NTF_BROADCAST_DEL_BUFF               101039

#define NTF_BROADCAST_OTHER_CHAR_SPEED       101041

#define REQ_USE_ITEM                         101043
#define RES_USE_ITEM                         101044

#define REQ_LOOK_OTHER_DETAIL_INFO           101045
#define RES_LOOK_OTHER_DETAIL_INFO           101046

#define NTF_TRANSFER_SCENE                   101047

#define NTF_UPDATE_MONEY                     101049

#define REQ_REPLACE_EQUIP                    101051
#define REQ_TAKEDOWN_EQUIP                   101053
#define NTF_BROADCAST_EQUIP_UPDATE           101055

#define NTF_BROADCAST_SIN_VAL_UPDATE         101057

#define REQ_PICK_UP_ITEM                     101059
#define RES_PICK_UP_ITEM                     101060

#define NTF_BROADCAST_NOTICE                 101061

#define REQ_WILD_BOSS_INFO                   101063
#define RES_WILD_BOSS_INFO                   101064

#define REQ_CHANGE_PK_MODE                   101065
#define NTF_CHANGE_PK_MODE                   101067

#define REQ_BUY_ITEM                         101069
#define REQ_SELL_ITEM                        101071

#define NTF_UPDATE_TI_LI                     101073

#define REQ_OBTAIN_MALL_BUY_LOG              101075
#define RES_OBTAIN_MALL_BUY_LOG              101076

#define NTF_MALL_BUY_LOG                     101077

#define NTF_BROADCAST_FLASH_MOVE             101079

#define REQ_ZHAN_XING                        101081
#define RES_ZHAN_XING                        101082
#define NTF_ZHAN_XING_VALUE                  101083

#define REQ_MODIFY_SYS_SETTINGS              101085
#define RES_MODIFY_SYS_SETTINGS              101086
#define NTF_PUSH_SYS_SETTINGS                101087

#define NTF_BROADCAST_JI_TUI                 101089

#define REQ_GET_TITLE_LIST                   101091
#define RES_GET_TITLE_LIST                   101092

#define NTF_ADD_NEW_TITLE                    101093

#define REQ_USE_TITLE                        101095
#define RES_USE_TITLE                        101096

#define REQ_CANCEL_TITLE                     101097
#define RES_CANCEL_TITLE                     101098

#define NTF_BROADCAST_CUR_TITLE              101099

#define REQ_GET_HUO_YUE_DU_LIST              101101
#define RES_GET_HUO_YUE_DU_LIST              101102

#define NTF_CHAR_BE_KILLED                   101103

#define REQ_ZHAN_XING_TEN                    101105
#define RES_ZHAN_XING_TEN                    101106

#define REQ_BUY_AND_USE_ITEM                 101107
#define REQ_DIAMOND_TO_COIN                  101109
#define RES_DIAMOND_TO_COIN                  101110
#define REQ_CLT_SYNC_HP_MP                   101111

#define REQ_OBTAIN_SVC_TIME                  101113
#define RES_OBTAIN_SVC_TIME                  101114

#define REQ_OBTAIN_WILD_BOSS_INFO            101115
#define RES_OBTAIN_WILD_BOSS_INFO            101116

#define NTF_BROADCAST_MST_ENTER_SCENE        101117
#define REQ_DIRECT_TRANSFER                  101119

#define NTF_RECHARGE_INFO                    101121

#define NTF_BROADCAST_GM_NOTICE              101123

#define REQ_GET_TRADE_NO                     101125
#define RES_GET_TRADE_NO                     101126

#define REQ_PRODUCE_ITEM                     101127
#define RES_PRODUCE_ITEM                     101128

#define REQ_ENTER_SCENE_OK                   101129

#define NTF_EXCHANGE_ACTIVATION_CODE_AWARD   101131

#define REQ_BACK_TOWN                        101133

#define NTF_BROADCAST_BOSS_APPEAR            101135

#define REQ_GET_HUO_YUE_DU_AWARD             101137
#define RES_GET_HUO_YUE_DU_AWARD             101138

#define NTF_HUO_YUE_DU_COMPLETE              101139

#define REQ_BUY_TI_LI                        101141

#define REQ_GET_FIRST_RECHARGE_AWARD         101143
#define RES_GET_FIRST_RECHARGE_AWARD         101144

#define NTF_BROADCAST_DROPPED_ITEM_OWNER_ID  101145

#define REQ_GOODS_AWARD_WEI_XIN_SHARED_OK    101147
#define NTF_GOODS_AWARD_WEI_XIN_SHARED_STATE 101149

#define REQ_GOODS_AWARD_LUCKY_TURN           101151
#define RES_GOODS_AWARD_LUCKY_TURN           101152

#define NTF_UPDATE_JING_LI                   101153

#define NTF_BROADCAST_BATTLE_NOTICE          101155

#define NTF_SUCCESS_TRADE_NO                 101157

#define NTF_XSZC_HONOR_VALUE                 101159

#define NTF_OPEN_ACTIVITY_BOX                101161

#define REQ_OBTAIN_BAOWU_MALL_INFO           101163
#define RES_OBTAIN_BAOWU_MALL_INFO           101164

#define REQ_REFRESH_BAOWU_MALL               101165
#define RES_REFRESH_BAOWU_MALL               101166

#define NTF_BUY_BAOWU_MALL_ITEM              101167

#define NTF_BROADCAST_FA_BAO_INFO_UPDATE     101169

#define NTF_BROADCAST_ALL_QH_LVL             101171
#define NTF_BROADCAST_SUM_BAO_SHI_LVL        101173

#define REQ_TEST_ECHO                        101997
#define RES_TEST_ECHO                        101998

//= package module
#define MJ_PACKAGE_MODULE                    102
#define NTF_PUSH_ITEM_LIST                   102001

#define NTF_ADD_ITEM                         102003
#define NTF_UPD_ITEM                         102005
#define NTF_DEL_ITEM                         102007

#define REQ_MOVE_ITEM                        102009

#define REQ_OBTAIN_ITEM_LIST                 102011
#define RES_OBTAIN_ITEM_LIST                 102012

#define REQ_DESTROY_ITEM                     102013

#define REQ_ENLARGE_CAPACITY                 102015
#define NTF_UPDATE_PACK_CAPACITY             102017

//= skill module
#define MJ_SKILL_MODULE                      103
#define NTF_PUSH_CHAR_SKILL_LIST             103001
#define NTF_CHAR_SKILL_CAN_UPGRADE           103003
#define REQ_UPGRADE_SKILL                    103005
#define NTF_CHAR_SKILL_UPDATE                103007
#define REQ_CHAR_LEARN_SKILL                 103009

//= social module
#define MJ_SOCIAL_MODULE                     104
#define NTF_SOCIAL_LIST                      104001

#define REQ_ADD_FRIEND                       104003
#define RES_ADD_FRIEND                       104004
#define NTF_ADD_FRIEND                       104005

#define REQ_REMOVE_FRIEND                    104007

#define REQ_ADD_BLACK                        104009

#define REQ_REMOVE_BLACK                     104011

#define NTF_ADD_SOCIALER                     104013

#define NTF_REMOVE_SOCIALER                  104015

#define NTF_SOCIAL_OTHER_CHAR_STATE          104017

#define REQ_SEARCH_SOCIAL_BY_NAME            104019
#define RES_SEARCH_SOCIAL_BY_NAME            104020

#define NTF_SOCIAL_OTHER_CHAR_LVL_UP         104021

#define NTF_SOCIAL_OTHER_CHAR_ZHAN_LI_UPDATE 104023

//= task module
#define MJ_TASK_MODULE                       105
#define NTF_TASK_UPDATE                      105001
#define REQ_ACCEPT_TASK                      105003
#define REQ_SUBMIT_TASK                      105005
#define REQ_ABANDON_TASK                     105007
#define REQ_DO_GUIDE                         105009
#define NTF_NO_TASK                          105011

//= chat
#define MJ_CHAT_MODULE                       106

#define REQ_CHAT_WORLD                       106001
#define RES_CHAT_WORLD                       106002

#define NTF_CHAT_WORLD                       106003

#define REQ_CHAT_TEAM                        106005
#define RES_CHAT_TEAM                        106006

#define NTF_CHAT_TEAM                        106007

#define REQ_CHAT_GUILD                       106009
#define RES_CHAT_GUILD                       106010

#define NTF_CHAT_GUILD                       106011

#define REQ_CHAT_PRIVATE                     106013
#define RES_CHAT_PRIVATE                     106014

#define NTF_CHAT_PRIVATE                     106015

#define REQ_CHAT_SCENE                       106021
#define RES_CHAT_SCENE                       106022

#define NTF_CHAT_SCENE                       106023

#define REQ_FLAUNT_ITEM                      106025
#define RES_FLAUNT_ITEM                      106026

#define NTF_CHAT_FLAUNT_ITEM                 106027

#define REQ_OBTAIN_FLAUNT_INFO               106029
#define RES_OBTAIN_FLAUNT_INFO               106030

//= team
#define MJ_TEAM_MODULE                       107
#define REQ_CREATE_TEAM                      107001
#define RES_CREATE_TEAM                      107002

#define REQ_QUIT_TEAM                        107003
#define RES_QUIT_TEAM                        107004

#define REQ_KICK_MEMBER                      107005
#define RES_KICK_MEMBER                      107006

#define REQ_INVITE_BE_A_TEAM                 107007
#define RES_INVITE_BE_A_TEAM                 107008

#define REQ_AGREE_INVITE                     107009
#define RES_AGREE_INVITE                     107010

#define REQ_REFUSE_INVITE                    107011
#define RES_REFUSE_INVITE                    107012

#define NTF_INVITE_BE_A_TEAM                 107013
#define NTF_REFUSE_INVITE                    107015

#define NTF_ALL_MEMBER_INFO                  107017
#define NTF_ADD_MEMBER                       107019
#define NTF_DEL_MEMBER                       107021
#define NTF_LEAVE_TEAM                       107023
#define NTF_LEADER_CHANGE                    107025
#define NTF_MEMBER_ONLINE                    107027
#define NTF_MEMBER_OFFLINE                   107029

//= mail
#define MJ_MAIL_MODULE                       108
#define REQ_OBTAIN_MAIL_LIST                 108001
#define RES_OBTAIN_MAIL_LIST                 108002

#define REQ_OBTAIN_MAIL_DETAIL               108003
#define RES_OBTAIN_MAIL_DETAIL               108004

#define REQ_PICK_UP_MAIL_ATTACH              108005
#define RES_PICK_UP_MAIL_ATTACH              108006

#define REQ_SEND_MAIL                        108007
#define RES_SEND_MAIL                        108008

#define REQ_DELETE_MAIL                      108009
#define RES_DELETE_MAIL                      108010

#define NTF_HAVED_NEW_MAIL                   108011

//= trade
#define MJ_TRADE_MODULE                      109
#define REQ_TRADE_INVITE                     109001
#define RES_TRADE_INVITE                     109002

#define NTF_TRADE_INVITE                     109003

#define REQ_TRADE_AGREE                      109005
#define RES_TRADE_AGREE                      109006

#define REQ_TRADE_REFUSE                     109007
#define RES_TRADE_REFUSE                     109008
#define NTF_TRADE_REFUSE                     109009

#define NTF_TRADE_BEGIN                      109011

#define REQ_TRADE_MODIFY_ITEM                109013
#define RES_TRADE_MODIFY_ITEM                109014
#define NTF_TRADE_MODIFY_ITEM                109015

#define REQ_TRADE_MODIFY_MONEY               109021
#define RES_TRADE_MODIFY_MONEY               109022
#define NTF_TRADE_MODIFY_MONEY               109023

#define REQ_TRADE_LOCK                       109025
#define RES_TRADE_LOCK                       109026
#define NTF_TRADE_LOCK                       109027

#define REQ_TRADE_CONFIRM                    109029
#define RES_TRADE_CONFIRM                    109030
#define NTF_TRADE_CONFIRM                    109031

#define REQ_TRADE_CANCEL                     109033
#define RES_TRADE_CANCEL                     109034
#define NTF_TRADE_CANCEL                     109035

#define NTF_TRADE_END                        109037

//= passive skill
#define MJ_PASSIVE_SKILL_MODULE              110
#define NTF_PUSH_CHAR_PASSIVE_SKILL_LIST     110001
#define NTF_PASSIVE_SKILL_CAN_LEARN_UPGRADE  110003

#define REQ_UPGRADE_PASSIVE_SKILL            110005
#define RES_UPGRADE_PASSIVE_SKILL            110006

#define NTF_CHAR_PASSIVE_SKILL_UPDATE        110007

#define REQ_ONE_KEY_UPGRADE_PASSIVE_SKILL    110009
#define RES_ONE_KEY_UPGRADE_PASSIVE_SKILL    110010

#define NTF_PASSIVE_SKILL_LUCKY_EGG          110011

//= iu
#define MJ_IU_MODULE                         111
#define REQ_EQUIP_STRENGTHEN                 111001
#define RES_EQUIP_STRENGTHEN                 111002

#define REQ_EQUIP_FEN_JIE                    111003
#define RES_EQUIP_FEN_JIE                    111004

#define REQ_EQUIP_RONG_HE                    111005
#define RES_EQUIP_RONG_HE                    111006

#define REQ_EQUIP_XI_LIAN                    111007
#define RES_EQUIP_XI_LIAN                    111008

#define REQ_XI_LIAN_REPLACE                  111009
#define RES_XI_LIAN_REPLACE                  111010

#define REQ_EQUIP_ONE_KEY_STRENGTHEN         111011
#define RES_EQUIP_ONE_KEY_STRENGTHEN         111012

#define NTF_EQUIP_STRENGTHEN_LUCKY_EGG       111013

#define REQ_EQUIP_JIN_JIE                    111015
#define RES_EQUIP_JIN_JIE                    111016

//= guild
#define MJ_GUILD_MODULE                      112
#define REQ_GUILD_CREATE                     112001
#define RES_GUILD_CREATE                     112002

#define REQ_GUILD_LIST                       112003
#define RES_GUILD_LIST                       112004

#define REQ_GUILD_APPLY                      112005
#define RES_GUILD_APPLY                      112006

#define NTF_GUILD_APPLY                      112007

#define REQ_GUILD_APPLY_LIST                 112009
#define RES_GUILD_APPLY_LIST                 112010

#define REQ_GUILD_REPLY                      112011
#define RES_GUILD_REPLY                      112012

#define REQ_GUILD_REPLY_ALL                  112013
#define RES_GUILD_REPLY_ALL                  112014

#define NTF_GUILD_REPLY                      112015

#define REQ_GUILD_MEM_EXIT                   112017
#define RES_GUILD_MEM_EXIT                   112018

#define REQ_GUILD_MEMBER_LIST                112019
#define RES_GUILD_MEMBER_LIST                112020

#define REQ_GUILD_APPLY_LIMIT_SET            112021
#define RES_GUILD_APPLY_LIMIT_SET            112022

#define REQ_GUILD_MEM_POS_SET                112023
#define RES_GUILD_MEM_POS_SET                112024

#define REQ_GUILD_EXPEL_MEM                  112025
#define RES_GUILD_EXPEL_MEM                  112026

#define REQ_GUILD_INFO                       112027
#define RES_GUILD_INFO                       112028

#define REQ_GUILD_PURPOSE_SET                112029
#define RES_GUILD_PURPOSE_SET                112030

#define NTF_BROADCAST_GUILD_INFO             112031

#define REQ_GUILD_UP_BUILDING                112033
#define RES_GUILD_UP_BULIDING                112034

#define REQ_GUILD_JUAN_XIAN                  112035
#define RES_GUILD_JUAN_XIAN                  112036

#define NTF_GUILD_MEM_CONTRIB                112037

#define NTF_GUILD_RESOURCE                   112039

#define REQ_GUILD_SUMMON_BOSS                112041
#define RES_GUILD_SUMMON_BOSS                112042

#define REQ_GUILD_JU_BAO                     112045
#define RES_GUILD_JU_BAO                     112046

#define REQ_GUILD_ENTER_ZHU_DI               112047
#define RES_GUILD_ENTER_ZHU_DI               112048

#define REQ_GUILD_SKILL_LIST                 112049
#define RES_GUILD_SKILL_LIST                 112050

#define REQ_GUILD_PROMOTE_SKILL              112051
#define RES_GUILD_PROMOTE_SKILL              112052

#define REQ_ENTER_GHZ                        112053

#define REQ_GHZ_OBTAIN_FIGHTING_INFO         112055
#define RES_GHZ_OBTAIN_FIGHTING_INFO         112056

#define REQ_GHZ_ACTIVATE_SHOU_WEI            112057
#define RES_GHZ_ACTIVATE_SHOU_WEI            112058

#define NTF_GUILD_HAD_APPLY                  112059

#define NTF_GUILD_MEM_JOIN_OR_EXIT           112061

#define NTF_GUILD_SUMMON_BOSS_OVER           112063

#define REQ_START_GUILD_SCP                  112065
#define RES_START_GUILD_SCP                  112066

#define REQ_ENTER_GUILD_SCP                  112067
#define RES_ENTER_GUILD_SCP                  112068

#define NTF_GUILD_SCP_OPEN                   112069

#define NTF_GUILD_SCP_OVER                   112071

#define NTF_GUILD_SCP_INFO                   112073

#define NTF_GUILD_MEM_POS                    112075

#define NTF_GHZ_ACT_LEFT_TIME                112077

#define REQ_SUMMON_GUILD_MEM                 112079
#define RES_SUMMON_GUILD_MEM                 112080

#define NTF_SUMMON_GUILD_MEM                 112081

#define REQ_TRANSFER_BY_GUILD_SUMMON         112083
#define RES_TRANSFER_BY_GUILD_SUMMON         112084

#define NTF_GHZ_OVER                         112085

//=market
#define MJ_MARKET_MODULE                     113
#define REQ_GET_MARKET_LIST                  113001
#define RES_GET_MARKET_LIST                  113002

#define REQ_SORT_BY_MONEY                    113003
#define RES_SORT_BY_MONEY                    113004

#define REQ_BUY_MARKET_ITEM                  113005
#define RES_BUY_MARKET_ITEM                  113006

#define REQ_SALE_MARKET_ITEM                 113007
#define RES_SALE_MARKET_ITEM                 113008

#define REQ_SALE_MARKET_MONEY                113009
#define RES_SALE_MARKET_MONEY                113010

#define REQ_CANCEL_MARKET_ITEM               113011
#define RES_CANCEL_MARKET_ITEM               113012

#define REQ_GET_SELF_SALE_LIST               113013
#define RES_GET_SELF_SALE_LIST               113014

#define REQ_SEARCH_MARKET_BY_NAME            113015
#define RES_SEARCH_MARKET_BY_NAME            113016

//= scp
#define MJ_SCP_MODULE                        114
#define REQ_ENTER_SCP                        114001

#define REQ_EXIT_SCP                         114003

#define REQ_IN_SCP_KILL_MONSTER              114005

#define NTF_IN_SCP_KILL_MST_AWARD            114007

#define REQ_IN_SCP_RELIVE                    114009

#define NTF_ALL_SCP_LIST                     114011
#define NTF_SINGLE_SCP_INFO                  114013

#define NTF_XSZC_SCP_INFO                    114015
#define NTF_XSZC_PERSON_INFO                 114017
#define NTF_UPDATE_KILL_COUNT                114019
#define NTF_PLAYER_GROUP                     114021
#define NTF_XSZC_SCP_END                     114023

#define REQ_SCP_END                          114025

#define NTF_NOTIFY_SCP_AWARD                 114027                 

#define REQ_MUSHROOM_SCP_END                 114029

#define REQ_GET_TOTAL_MSTAR                  114031
#define RES_GET_TOTAL_MSTAR                  114032

//=rank
#define MJ_RANK_MODULE                       115
#define REQ_OBTAIN_RANK_INFO                 115001
#define RES_OBTAIN_RANK_INFO                 115002

//= tui tu
#define MJ_TUI_TU_MODULE                     116
#define REQ_OBTAIN_TUI_TU_CHAPTER_INFO       116001
#define RES_OBTAIN_TUI_TU_CHAPTER_INFO       116002

#define REQ_ENTER_TUI_TU                     116003

#define REQ_IN_TUI_TU_KILL_MONSTER           116005

#define NTF_IN_TUI_TU_KILL_MST_AWARD         116007

#define REQ_IN_TUI_TU_RELIVE                 116009

#define REQ_EXIT_TUI_TU                      116011

#define REQ_TUI_TU_END                       116013

#define REQ_TUI_TU_DO_TURN                   116015
#define RES_TUI_TU_DO_TURN                   116016

#define REQ_OBTAIN_TUI_TU_INFO               116017
#define RES_OBTAIN_TUI_TU_INFO               116018

#define NTF_NOTIFY_CLT_TUI_TU_END_INFO       116019
#define REQ_TUI_TU_SAO_DANG                  116021

#define NTF_TUI_TU_BOSS_DROP                 116023

//= mis_award
#define MJ_MIS_AWARD_MODULE                  117
#define REQ_OBTAIN_ONLINE_AWARD              117001
#define RES_OBTAIN_ONLINE_AWARD              117002

#define REQ_GET_ONLINE_AWARD                 117003
#define RES_GET_ONLINE_AWARD                 117004

#define NTF_LOGIN_AWARD_INFO                 117005

#define REQ_GET_LOGIN_AWARD                  117007
#define RES_GET_LOGIN_AWARD                  117008

#define NTF_LVL_AWARD_INFO                   117009

#define REQ_GET_LVL_AWARD                    117011
#define RES_GET_LVL_AWARD                    117012

#define NTF_ONLINE_AWARD                     117013

#define NTF_SEVEN_DAY_LOGIN                  117015

#define REQ_GET_SEVEN_DAY_LOGIN_AWARD        117017

//= vip
#define MJ_VIP_MODULE                        118
#define NTF_BROADCAST_VIP_INFO               118001

#define NTF_VIP_INFO                         118003

#define REQ_VIP_GET_LVL_AWARD                118005
#define REQ_VIP_EXCHANGE_COIN                118007

//= cheng jiu
#define MJ_CHENG_JIU_MODULE                  119
#define REQ_GET_CHENG_JIU_LIST               119001
#define RES_GET_CHENG_JIU_LIST               119002

#define REQ_GET_CHENG_JIU_AWARD              119003
#define RES_GET_CHENG_JIU_AWARD              119004

#define NTF_COMPLETE_CHENG_JIU               119005

#define NTF_CAN_GET_CHENG_JIU_AWARD          119007

#define NTF_CHENG_JIU_LUCKY_EGG              119009

//= bao_shi
#define MJ_BAO_SHI_MODULE                    120
#define REQ_OBTAIN_BAO_SHI                   120001
#define RES_OBTAIN_BAO_SHI                   120002

#define REQ_ACTIVATE_BAO_SHI                 120003
#define RES_ACTIVATE_BAO_SHI                 120004

#define REQ_UPGRADE_BAO_SHI                  120005
#define RES_UPGRADE_BAO_SHI                  120006

#define NTF_BAO_SHI_INFO                     120007

#define REQ_UPGRADE_BAO_SHI_TO_NEXT          120009
#define RES_UPGRADE_BAO_SHI_TO_NEXT          120010

#define NTF_BAO_SHI_UPGRADE_LUCKY_EGG        120011

//= jing ji
#define MJ_JING_JI_MODULE                    121
#define REQ_OBTAIN_JING_JI_INFO              121001
#define RES_OBTAIN_JING_JI_INFO              121002

#define REQ_JING_JI_REFRESH_COMPETITOR       121003
#define RES_JING_JI_REFRESH_COMPETITOR       121004

#define REQ_JING_JI_CHALLENGE                121005
#define RES_JING_JI_CHALLENGE                121006

#define REQ_JING_JI_CLEAN_TIME               121007
#define RES_JING_JI_CLEAN_TIME               121008

#define NTF_JING_JI_CHALLENGE_END            121009

#define REQ_GET_JING_JI_LOG_LIST             121011
#define RES_GET_JING_JI_LOG_LIST             121012

#define NTF_UPDATE_JING_JI_SCORE             121013

#define NTF_NOTIFY_CAN_JING_JI               121015

//= activity
#define MJ_ACTIVITY_MODULE                   122
#define NTF_ACTIVITY_OPENED                  122001
#define NTF_ACTIVITY_CLOSED                  122003

#define REQ_GOT_TI_LI_AWARD                  122005
#define RES_GOT_TI_LI_AWARD                  122006

#define REQ_GOT_MONTH_CARD_AWARD             122007
#define RES_GOT_MONTH_CARD_AWARD             122008
#define NTF_CAN_BUY_MONTH_CARD               122009

#define REQ_ENTER_XSZC                       122011
#define RES_ENTER_XSZC                       122012

#define REQ_OBTAIN_XSZC_ACT_INFO             122013
#define RES_OBTAIN_XSZC_ACT_INFO             122014

#define REQ_OBTAIN_XSZC_GROUP_POS            122015
#define RES_OBTAIN_XSZC_GROUP_POS            122016

#define REQ_GET_DAILY_LUCKY_GOODS_DRAW_CNT   122017
#define RES_GET_DAILY_LUCKY_GOODS_DRAW_CNT   122018

#define REQ_DO_DAILY_LUCKY_GOODS_DRAW        122019
#define RES_DO_DAILY_LUCKY_GOODS_DRAW        122020

#define REQ_OBTAIN_KAI_FU_ACT_RANK           122021
#define RES_OBTAIN_KAI_FU_ACT_RANK           122022

#define REQ_OBTAIN_KAI_FU_ACT_STATE          122023
#define RES_OBTAIN_KAI_FU_ACT_STATE          122024

//= limit time activity
#define MJ_LTIME_ACT_MODULE                  123
#define NTF_LTIME_ACT_OPENED                 123001
#define NTF_LTIME_ACT_CLOSED                 123003

#define REQ_OBTAIN_LUCKY_TURN_INFO           123005
#define RES_OBTAIN_LUCKY_TURN_INFO           123006

#define REQ_OBTAIN_LUCKY_TURN_BIG_AWARD_LOG  123007
#define RES_OBTAIN_LUCKY_TURN_BIG_AWARD_LOG  123008

#define REQ_LUCKY_TURN_ONE                   123009
#define RES_LUCKY_TURN_ONE                   123010

#define REQ_LUCKY_TURN_TEN                   123011
#define RES_LUCKY_TURN_TEN                   123012

#define REQ_LTIME_ACC_RECHARGE_INFO          123013
#define RES_LTIME_ACC_RECHARGE_INFO          123014

#define REQ_LTIME_DAILY_RECHARGE_INFO        123015
#define RES_LTIME_DAILY_RECHARGE_INFO        123016

#define REQ_LTIME_ACC_CONSUME_INFO           123017
#define RES_LTIME_ACC_CONSUME_INFO           123018

#define REQ_LTIME_DAILY_CONSUME_INFO         123019
#define RES_LTIME_DAILY_CONSUME_INFO         123020

//= cross
#define MJ_CROSS_MODULE                      124
#define REQ_GET_RIVAL_LIST                   124001
#define RES_GET_RIVAL_LIST                   124002

#define REQ_GET_OTHER_DETAIL                 124003
#define RES_GET_OTHER_DETAIL                 124004

//= mobai_module
#define MJ_MOBAI_MODULE                      125
#define NTF_WORSHIP_STATUE_INFO              125001

#define REQ_OBTAIN_WORSHIP_INFO              125003
#define RES_OBTAIN_WORSHIP_INFO              125004

#define REQ_WORSHIP_CASTELLAN                125005
#define RES_WORSHIP_CASTELLAN                125006

#define REQ_GET_WORSHIP_COIN                 125007
#define RES_GET_WORSHIP_COIN                 125008

//= water_tree
#define MJ_WATER_TREE_MODULE                 126
#define NTF_WATER_TREE_INFO                  126001

#define REQ_GET_WATER_TIME                   126003
#define RES_GET_WATER_TIME                   126004

#define REQ_WATER_TREE                       126005
#define RES_WATER_TREE                       126006

//= tianfu skill
#define MJ_TIANFU_SKILL_MODULE               127
#define NTF_PUSH_CHAR_TIANFU_SKILL_LIST      127001

#define REQ_UPGRADE_TIANFU_SKILL             127003
#define RES_UPGRADE_TIANFU_SKILL             127004

#define NTF_BROADCAST_TIANFU_SKILL           127005

//= di xia cheng
#define MJ_DI_XIA_CHENG_MODULE               128
#define REQ_ENTER_DIXIACHENG                 128001
#define RES_ENTER_DIXIACHENG                 128002

#define REQ_EXIT_DIXIACHENG                  128003
#define RES_EXIT_DIXIACHENG                  128004

#define REQ_FINISH_DIXIACHENG                128005
#define RES_FINISH_DIXIACHENG                128006

#define REQ_GET_DIXIACHENG_INFO              128007
#define RES_GET_DIXIACHENG_INFO              128008

//= lue duo
#define MJ_LUE_DUO_MODULE                    129
#define REQ_GET_MATE_HOLDER_LIST             129001
#define RES_GET_MATE_HOLDER_LIST             129002

#define REQ_ROB_SOMEONE                      129003
#define RES_ROB_SOMEONE                      129004

#define REQ_GET_LOG_ROBBED                   129005
#define RES_GET_LOG_ROBBED                   129006

#define NTF_LUEDUO_RESULT                    129007

#define REQ_LUEDUO_REVENGE                   129009
#define RES_LUEDUO_REVENGE                   129010

#define REQ_LUEDUO_FANPAI                    129011
#define RES_LUEDUO_FANPAI                    129012

//= fa bao
#define MJ_FA_BAO_MODULE                     130
#define NTF_FA_BAO_EQUIP_INFO                130001

#define REQ_TAKEUP_FA_BAO                    130003

#define REQ_HE_CHENG_FA_BAO                  130005
#define RES_HE_CHENG_FA_BAO                  130006

#define REQ_FA_BAO_JIN_JIE                   130007
#define RES_FA_BAO_JIN_JIE                   130008

#define NTF_FA_BAO_RECOVERY_ATTR             130009

/*************************************************/
//= cross msg list, range in 300000 ~ 399999
/*************************************************/
//* world ~ cross server
#define REQ_WORLD_HEART_BEAT                 300001
#define RES_WORLD_HEART_BEAT                 300002

#define REQ_WORLD_LOGIN                      300003
#define RES_WORLD_LOGIN                      300004

#define REQ_ROUTE_REQ_MSG                    300005
#define RES_ROUTE_RES_MSG                    300006

/*************************************************/
//= db proxy msg list, range in 101 ~ 999
/*************************************************/
//* logic/world ~ db proxy server
#define REQ_DB_PROXY_LOGIN                   101
#define RES_DB_PROXY_LOGIN                   102

#define REQ_DB_PROXY_HEART_BEAT              103
#define RES_DB_PROXY_HEART_BEAT              104

#define REQ_DB_PROXY_PAYLOAD                 105
#define RES_DB_PROXY_PAYLOAD                 106

//= account info
#define REQ_GET_ACCOUNT_INFO                 111
#define RES_GET_ACCOUNT_INFO                 112

#define REQ_INSERT_ACCOUNT_INFO              113
#define RES_INSERT_ACCOUNT_INFO              114

#define REQ_UPDATE_ACCOUNT_INFO              115
#define RES_UPDATE_ACCOUNT_INFO              116

#define REQ_SET_DIAMOND                      117
#define RES_SET_DIAMOND                      118

//= consume log
#define REQ_INSERT_CONSUME_LOG               121
#define RES_INSERT_CONSUME_LOG               122

//= recharge log
#define REQ_INSERT_RECHARGE_LOG              123
#define RES_INSERT_RECHARGE_LOG              124

//= char info
#define REQ_GET_CHAR_LIST                    201
#define RES_GET_CHAR_LIST                    202

#define REQ_GET_NEW_CHAR_INFO                203
#define RES_GET_NEW_CHAR_INFO                204
#define REQ_GET_CHAR_INFO                    205
#define RES_GET_CHAR_INFO                    206

#define REQ_INSERT_CHAR_INFO                 207
#define RES_INSERT_CHAR_INFO                 208

#define REQ_UPDATE_CHAR_INFO                 209
#define RES_UPDATE_CHAR_INFO                 210

//= char extra info
#define REQ_GET_CHAR_EXTRA_INFO              211
#define RES_GET_CHAR_EXTRA_INFO              212

#define REQ_INSERT_CHAR_EXTRA_INFO           213
#define RES_INSERT_CHAR_EXTRA_INFO           214

#define REQ_UPDATE_CHAR_EXTRA_INFO           215
#define RES_UPDATE_CHAR_EXTRA_INFO           216

//= char daily clean info
#define REQ_GET_DAILY_CLEAN_INFO             217
#define RES_GET_DAILY_CLEAN_INFO             218

#define REQ_INSERT_DAILY_CLEAN_INFO          219
#define RES_INSERT_DAILY_CLEAN_INFO          220

#define REQ_UPDATE_DAILY_CLEAN_INFO          221
#define RES_UPDATE_DAILY_CLEAN_INFO          222

//= char db msg queue
#define REQ_GET_CHAR_DB_MSG_LIST             223
#define RES_GET_CHAR_DB_MSG_LIST             224

#define REQ_DELETE_CHAR_DB_MSG               225
#define RES_DELETE_CHAR_DB_MSG               226

#define REQ_INSERT_CHAR_DB_MSG               227
#define RES_INSERT_CHAR_DB_MSG               228

//= social
#define REQ_GET_SOCIAL_LIST                  231
#define RES_GET_SOCIAL_LIST                  232

#define REQ_INSERT_SOCIAL                    233
#define RES_INSERT_SOCIAL                    234

#define REQ_UPDATE_SOCIAL                    235
#define RES_UPDATE_SOCIAL                    236

#define REQ_DELETE_SOCIAL                    237
#define RES_DELETE_SOCIAL                    238

//= task
#define REQ_GET_TASK_LIST                    241
#define RES_GET_TASK_LIST                    242

#define REQ_INSERT_TASK                      243
#define RES_INSERT_TASK                      244

#define REQ_UPDATE_TASK                      245
#define RES_UPDATE_TASK                      246

#define REQ_DELETE_TASK                      247
#define RES_DELETE_TASK                      248

#define REQ_GET_TASK_BIT_ARRAY               249
#define RES_GET_TASK_BIT_ARRAY               250

#define REQ_UPDATE_TASK_BIT_ARRAY            251
#define RES_UPDATE_TASK_BIT_ARRAY            252

//= item
#define REQ_GET_ITEM_SINGLE_PKG              269
#define RES_GET_ITEM_SINGLE_PKG              270

//= skill
#define REQ_INSERT_SKILL                     271
#define RES_INSERT_SKILL                     272

#define REQ_GET_SKILL_LIST                   273
#define RES_GET_SKILL_LIST                   274

#define REQ_UPDATE_SKILL                     275
#define RES_UPDATE_SKILL                     276

//= item
#define REQ_INSERT_ITEM                      277
#define RES_INSERT_ITEM                      278

#define REQ_GET_ITEM_LIST                    279
#define RES_GET_ITEM_LIST                    280

#define REQ_UPDATE_ITEM                      281
#define RES_UPDATE_ITEM                      282

#define REQ_DELETE_ITEM                      283
#define RES_DELETE_ITEM                      284

//= buff
#define REQ_GET_BUFF_LIST                    285
#define RES_GET_BUFF_LIST                    286

#define REQ_UPDATE_BUFF_LIST                 287
#define RES_UPDATE_BUFF_LIST                 288

//= mail
#define REQ_GET_MAIL_LIST                    289
#define RES_GET_MAIL_LIST                    290

#define REQ_GET_MAIL_INFO                    291
#define RES_GET_MAIL_INFO                    292

#define REQ_INSERT_MAIL                      293
#define RES_INSERT_MAIL                      294

#define REQ_UPDATE_MAIL_ATTACH               295
#define RES_UPDATE_MAIL_ATTACH               296

#define REQ_UPDATE_MAIL_READED               297
#define RES_UPDATE_MAIL_READED               298

#define REQ_UPDATE_MAIL_DELETE               299
#define RES_UPDATE_MAIL_DELETE               300

#define REQ_CHECK_NEW_MAIL                   301
#define RES_CHECK_NEW_MAIL                   302

#define REQ_GET_MAIL_ATTACH                  303
#define RES_GET_MAIL_ATTACH                  304

#define REQ_GET_SYSTEM_MAIL_LIST             305
#define RES_GET_SYSTEM_MAIL_LIST             306

//= skill
#define REQ_GET_PASSIVE_SKILL_LIST           313
#define RES_GET_PASSIVE_SKILL_LIST           314

#define REQ_UPDATE_PASSIVE_SKILL             315
#define RES_UPDATE_PASSIVE_SKILL             316

#define REQ_GET_TIANFU_SKILL_LIST            317
#define RES_GET_TIANFU_SKILL_LIST            318

#define REQ_UPDATE_TIANFU_SKILL              319
#define RES_UPDATE_TIANFU_SKILL              320

//=char info search
#define REQ_GET_SEARCH_INFO                  321
#define RES_GET_SEARCH_INFO                  322

#define REQ_INSERT_MALL_BUY_LOG              323
#define RES_INSERT_MALL_BUY_LOG              324

#define REQ_GET_MALL_BUY_LOG                 325
#define RES_GET_MALL_BUY_LOG                 326

//= guild
#define REQ_INSERT_GUILD                     331
#define RES_INSERT_GUILD                     332

#define REQ_DELETE_GUILD                     333
#define RES_DELETE_GUILD                     334

#define REQ_UPDATE_GUILD                     335
#define RES_UPDATE_GUILD                     336

#define REQ_INSERT_GUILD_MEMBER              339
#define RES_INSERT_GUILD_MEMBER              340

#define REQ_DELETE_GUILD_MEMBER              341
#define RES_DELETE_GUILD_MEMBER              342

#define REQ_DELETE_GUILD_MEMBERS             343
#define RES_DELETE_GUILD_MEMBERS             344

#define REQ_UPDATE_GUILD_MEMBER              345
#define RES_UPDATE_GUILD_MEMBER              346

#define REQ_INSERT_GUILD_APPLY               349
#define RES_INSERT_GUILD_APPLY               350

#define REQ_DELETE_GUILD_APPLY               351
#define RES_DELETE_GUILD_APPLY               352

#define REQ_DELETE_GUILD_APPLYS              353
#define RES_DELETE_GUILD_APPLYS              354

#define REQ_INSERT_GUILD_SKILL               355
#define RES_INSERT_GUILD_SKILL               356

#define REQ_GET_GUILD_SKILL_LIST             357
#define RES_GET_GUILD_SKILL_LIST             358

#define REQ_UPDATE_GUILD_SKILL               359
#define RES_UPDATE_GUILD_SKILL               360

//= market
#define REQ_INSERT_MARKET_INFO               361
#define RES_INSERT_MARKET_INFO               362

#define REQ_DELETE_MARKET_INFO               363
#define RES_DELETE_MARKET_INFO               364

//= char attr
#define REQ_UPDATE_CHAR_ATTR                 371
#define RES_UPDATE_CHAR_ATTR                 372

//= tui tu log
#define REQ_GET_TUI_TU_LOG                   377
#define RES_GET_TUI_TU_LOG                   378

#define REQ_INSERT_TUI_TU_LOG                379
#define RES_INSERT_TUI_TU_LOG                379

#define REQ_UPDATE_TUI_TU_LOG                381
#define RES_UPDATE_TUI_TU_LOG                382

//= vip
#define REQ_GET_VIP                          391
#define RES_GET_VIP                          392

#define REQ_INSERT_VIP                       393
#define RES_INSERT_VIP                       394

#define REQ_UPDATE_VIP                       395
#define RES_UPDATE_VIP                       396

//= daily task
#define REQ_GET_DAILY_TASK_LIST              401
#define RES_GET_DAILY_TASK_LIST              402

#define REQ_INSERT_DAILY_TASK                403
#define RES_INSERT_DAILY_TASK                404

#define REQ_UPDATE_DAILY_TASK                405
#define RES_UPDATE_DAILY_TASK                406

#define REQ_CLEAN_ALL_DAILY_TASK             407
#define RES_CLEAN_ALL_DAILY_TASK             408

//= cheng jiu
#define REQ_GET_CHENG_JIU_INFO               411
#define RES_GET_CHENG_JIU_INFO               412

#define REQ_UPDATE_CHENG_JIU_INFO            413
#define RES_UPDATE_CHENG_JIU_INFO            414

//= bao_shi
#define REQ_GET_BAO_SHI                      421
#define RES_GET_BAO_SHI                      422

#define REQ_INSERT_BAO_SHI                   423
#define RES_INSERT_BAO_SHI                   424

#define REQ_UPDATE_BAO_SHI                   425
#define RES_UPDATE_BAO_SHI                   426

//= title
#define REQ_GET_TITLE_INFO                   431
#define RES_GET_TITLE_INFO                   432

#define REQ_INSERT_TITLE_INFO                433
#define RES_INSERT_TITLE_INFO                434

//= sys settings
#define REQ_GET_SYS_SETTINGS                 437
#define RES_GET_SYS_SETTINGS                 438

#define REQ_UPDATE_SYS_SETTINGS              439
#define RES_UPDATE_SYS_SETTINGS              440

//= forbid
#define REQ_INSERT_FORBID_OPT                441
#define RES_INSERT_FORBID_OPT                442

//= notice
#define REQ_UPDATE_NOTICE_INFO               451
#define RES_UPDATE_NOTICE_INFO               452

#define REQ_DELETE_NOTICE_INFO               453
#define RES_DELETE_NOTICE_INFO               454

//= huo yue du
#define REQ_GET_HUO_YUE_DU_INFO              461
#define RES_GET_HUO_YUE_DU_INFO              462

#define REQ_UPDATE_HUO_YUE_DU_INFO           463
#define RES_UPDATE_HUO_YUE_DU_INFO           464

#define REQ_CLEAR_HUO_YUE_DU_INFO            465
#define RES_CLEAR_HUO_YUE_DU_INFO            466

// skill
#define REQ_GET_OTHER_SKILL_LIST             467
#define RES_GET_OTHER_SKILL_LIST             468

// service info
#define REQ_UPDATE_SERVICE_INFO              469
#define RES_UPDATE_SERVICE_INFO              470

//= scp log
#define REQ_GET_SCP_LOG                      471
#define RES_GET_SCP_LOG                      472

#define REQ_INSERT_SCP_LOG                   473
#define RES_INSERT_SCP_LOG                   474

#define REQ_UPDATE_SCP_LOG                   475
#define RES_UPDATE_SCP_LOG                   476

#define REQ_CLEAR_SCP_LOG                    477
#define RES_CLEAR_SCP_LOG                    478

//= jing ji rank
#define REQ_UPDATE_JING_JI_RANK              481
#define RES_UPDATE_JING_JI_RANK              482

#define REQ_UPDATE_JING_JI_AWARD_TIME        483
#define RES_UPDATE_JING_JI_AWARD_TIME        484

#define REQ_GET_JING_JI_LOG                  485
#define RES_GET_JING_JI_LOG                  486

#define REQ_INSERT_JING_JI_LOG               487
#define RES_INSERT_JING_JI_LOG               488

//= limit time activity
#define REQ_UPDATE_LIMIT_TIME_ACT            495
#define RES_UPDATE_LIMIT_TIME_ACT            496

//= char recharge
#define REQ_GET_CHAR_RECHARGE                501
#define RES_GET_CHAR_RECHARGE                502

#define REQ_UPDATE_CHAR_RECHARGE             503
#define RES_UPDATE_CHAR_RECHARGE             504

//= lucky turn
#define REQ_UPDATE_LUCKY_TURN_ACC_AWARD      505
#define RES_UPDATE_LUCKY_TURN_ACC_AWARD      506

#define REQ_INSERT_LUCKY_TURN_BIG_AWARD      507
#define RES_INSERT_LUCKY_TURN_BIG_AWARD      508

#define REQ_DELETE_LUCKY_TURN_BIG_AWARD      509
#define RES_DELETE_LUCKY_TURN_BIG_AWARD      510

#define REQ_GET_LUCKY_TURN_BIG_AWARD_LIST    511
#define RES_GET_LUCKY_TURN_BIG_AWARD_LIST    512

#define REQ_UPDATE_LUCKY_TURN_SCORE          513
#define RES_UPDATE_LUCKY_TURN_SCORE          514

#define REQ_DELETE_LUCKY_TURN_SCORE          515
#define RES_DELETE_LUCKY_TURN_SCORE          516

//= ltime recharge/consume award
#define REQ_GET_LTIME_RECHARGE_AWARD         521
#define RES_GET_LTIME_RECHARGE_AWARD         522

#define REQ_UPDATE_LTIME_RECHARGE_AWARD      523
#define RES_UPDATE_LTIME_RECHARGE_AWARD      524

#define REQ_INSERT_ACTIVATION_CODE           525
#define RES_INSERT_ACTIVATION_CODE           526

#define REQ_UPDATE_ACTIVATION_CODE           527
#define RES_UPDATE_ACTIVATION_CODE           528

//= mobai_module
#define REQ_GET_CASTELLAN_ITEM_SINGLE_PKG    531
#define RES_GET_CASTELLAN_ITEM_SINGLE_PKG    532
#define REQ_UPDATE_WORSHIP_INFO              534
#define RES_UPDATE_WORSHIP_INFO              535

//= water_tree
#define REQ_UPDATE_WATER_TREE_INFO           541
#define RES_UPDATE_WATER_TREE_INFO           542

//= goods award
#define REQ_UPDATE_GOODS_AWARD_INFO          543
#define RES_UPDATE_GOODS_AWARD_INFO          544

//= lueduo
#define REQ_INSERT_LUEDUO_ITEM               551
#define RES_INSERT_LUEDUO_ITEM               552

#define REQ_UPDATE_LUEDUO_ITEM               553
#define RES_UPDATE_LUEDUO_ITEM               554

#define REQ_INSERT_LUEDUO_LOG                555
#define RES_INSERT_LUEDUO_LOG                556

#define REQ_UPDATE_LUEDUO_LOG                557
#define RES_UPDATE_LUEDUO_LOG                558

#define REQ_DELETE_LUEDUO_LOG                559
#define RES_DELETE_LUEDUO_LOG                560

#define REQ_GET_LUEDUO_LOG                   561
#define RES_GET_LUEDUO_LOG                   562

//= di xia cheng
#define REQ_GET_DXC_INFO                     565
#define RES_GET_DXC_INFO                     566

#define REQ_INSERT_DXC_INFO                  567
#define RES_INSERT_DXC_INFO                  568

#define REQ_UPDATE_DXC_INFO                  569
#define RES_UPDATE_DXC_INFO                  570

//= baowu mall
#define REQ_GET_BAOWU_MALL_INFO              571
#define RES_GET_BAOWU_MALL_INFO              572

#define REQ_UPDATE_BAOWU_MALL_INFO           573
#define RES_UPDATE_BAOWU_MALL_INFO           574

//= kai fu act log
#define REQ_GET_KAI_FU_ACT_LOG               575
#define RES_GET_KAI_FU_ACT_LOG               576

#define REQ_INSERT_KAI_FU_ACT_LOG            577
#define RES_INSERT_KAI_FU_ACT_LOG            578

//= cross
#define REQ_CROSS_GET_ITEM_SINGLE_PKG        801
#define RES_CROSS_GET_ITEM_SINGLE_PKG        802

//= end
#define MAX_DB_MSG_ID                        999

#endif // MESSAGE_H_

