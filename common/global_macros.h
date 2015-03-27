// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-30 17:30
 */
//========================================================================

#ifndef GLOBAL_MACROS_H_
#define GLOBAL_MACROS_H_

#define _MACRO_TO_STR(X)                       #X
#define MACRO_TO_STR(X)                        _MACRO_TO_STR(X)
#define MAX_IP_ADDR_LEN                        15
#define MAX_FILE_PATH_LEN                      127
#define MAX_HOST_NAME                          127
#define MAX_DATETIME_LEN                       19
#define MAX_CHANNEL_LEN                        30

#define MAX_CHAR_RECHARGE_LEN                  127
#define MAX_GROUP_NAME_LEN                     31
#define MAX_ACCOUNT_LEN                        63
#define MAX_NAME_LEN                           31
#define MAX_PURE_NAME_LEN                      18
#define MAX_ITEM_NAME_LEN                      27
#define MAX_AUTH_KEY_LEN                       15
#define CLT_VIEW_WIDTH_RADIUS                  12  // 960/48/2
#define CLT_VIEW_HEIGHT_RADIUS                 12  // 640/32/2

#define MAX_SQL_LEN                            8191
#define MAX_BUFF_DATA_LEN                      255
#define MAX_SYS_SETTINGS_LEN                   511

// 内部几个服务之间通信的socket 缓冲区
#define MAX_INNER_SOCKET_BUF_SIZE              524288 // 512

// 只针对客户端的socket发送缓冲区
#define MAX_CLIENT_SEND_BUF_SIZE               262144 // 256K

// 整个游戏世界内单个消息的最大长度
#define MAX_ONE_MESSAGE_SIZE                   65536  // 64K

// 接收客户端消息的缓冲区大小(not socketbuf)
#define CLIENT_RECV_BUFF_SIZE                  1024

#define TO_CLIENT_MSG_SLICE_BUF_SIZE           2048
#define MAX_SNAP_SLICE_MB_SIZE                 2048
#define MAX_SNAP_ONE_UNIT_INFO_SIZE            256

//
#define MIN_SERVICE_SN                         0
#define MAX_SERVICE_SN                         1999
#define CHAR_ID_RANGE                          1000000
#define MIN_CHAR_ID                            1
#define MAX_CHAR_ID                            ((MAX_SERVICE_SN + 1)*CHAR_ID_RANGE-1)

#define GUILD_ID_RANGE                         100000
#define MAX_GUILD_ID                           ((MAX_SERVICE_SN + 1)*GUILD_ID_RANGE-1)

#define FORBID_ID_RANGE                        100000
#define MAX_FORBID_ID                          ((MAX_SERVICE_SN + 1)*FORBID_ID_RANGE-1)

#define MONSTER_ID_RANGE                         10000000
#define MONSTER_ID_BASE                        2010000000

#define DROPPED_ITEM_ID_RANGE                    10000000
#define DROPPED_ITEM_ID_BASE                   2020000000

#define EFFECT_ID_RANGE                          10000000
#define EFFECT_ID_BASE                         2030000000

#define PLAYER_COPY_ID_RANGE                     10000000
#define PLAYER_COPY_ID_BASE                    2040000000

#define CROSS_HEART_BEAT                       5
#define DB_PROXY_HEART_BEAT                    5

#define PLAYER_DEFAULT_SPEED                   100

//= scene
#define MAX_SCENE_CID                          9999
#define MAX_SCP_COUNT                          10000
#define MIN_SCP_ID                             ((MAX_SCENE_CID) + 1)
#define MAX_SCP_ID                             (MIN_SCP_ID + MAX_SCP_COUNT)
#define MAX_LAYOUT_ID                          35

//= item
#define MAX_ITEM_EXTRA_INFO_LEN                511
#define MAX_PACKAGE_CAPACITY                   245
#define MAX_DJ_UPPER_LIMIT                     999

//= task
#define MAX_TASK_NUM                           1024
#define HALF_TASK_NUM                          (MAX_TASK_NUM/2)

//= chat
#define MAX_CHAT_CONTENT_LEN                   4096

//= mail
#define MAX_MAIL_CONTENT_LEN                   600
#define MAX_MAIL_TITLE_LEN                     60
#define ALL_ITEMS_LEN                          ((MAX_ITEM_EXTRA_INFO_LEN + 64) * MAX_ATTACH_NUM)

//= notice
#define MAX_NOTICE_CONTENT_LEN                 512
#define MAX_NOTICE_ID                          100000

//#define MAX_ITEMS_LEN                          2048
#define MAX_MAIL_COUNT                         96
#define MAX_ATTACH_NUM                         3

// guild
#define MAX_GUILD_PURPOSE_LEN                  450
#define MAX_INVALID_INT                        2147483647 // 32
#define MAX_VALID_INT                          99999999

// market
#define MARKET_ID_RANGE                        100000

// mall
#define BAOWU_MALL_ITEM_COUNT                  6

// vip
#define MAX_VIP_LVL                            17

// bao_shi
#define MAX_BAO_SHI_POS_IDX_LVL                100
#define MAX_CIRCLE_CNT                         500

// replace string
#define STRING_CHAR                            "char"
#define STRING_ITEM                            "item"
#define STRING_EQUIP                           "equip"
#define STRING_NUMBER                          "number"
#define STRING_ATTR                            "attr"
#define STRING_PRICE                           "price"
#define STRING_GUILD                           "guild"
#define STRING_MONEY_TYPE                      "money_type"
#define STRING_DATE                            "date"
#define STRING_RANK                            "rank"
#define STRING_ACTIVITY                        "activity"
#define STRING_SCENE                           "scene"
#define STRING_MONSTER                         "monster"
#define STRING_ATTR                            "attr"
#define STRING_TITLE                           "title"
#define STRING_DATE_TIME                       "datetime"
#define STRING_NPC                             "npc"
#define STRING_PART                            "part"
#define STRING_PSKILL                          "pskill"
#define STRING_TSKILL                          "tskill"
#define STRING_EQUIP_DJ                        "equip_dj"

// iu
#define MAX_XI_LIAN_ATTR_NUM                   6

#define FA_BAO_PART_CNT                        6
#define FA_BAO_MAX_DJ                          10 

#define MONTHLY_CARD_REBATE_DAYS               30

#endif // GLOBAL_MACROS_H_

