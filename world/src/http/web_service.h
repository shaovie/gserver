// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-21 19:02
 */
//========================================================================

#ifndef WEB_SERVICE_H_
#define WEB_SERVICE_H_

// Lib header
#include "ilist.h"

#include "http_srv.h"

// Forward declarations
class item_obj;

//= GET
#define IF_CFG_UPDATE                "cfg_update"       // cfg_update?f=scene.json,exp.json
#define IF_VERSION                   "version"          // version
#define IF_ONLINE                    "online"           // online
#define IF_SCENE_ONLINE              "scene_online"     // scene_online
#define IF_DB_PAYLOAD                "db_payload"       // db_payload
#define IF_KICK_OUT                  "kick"             // kick?char_id=id
#define IF_TO_LVL                    "to_lvl"           // to_lvl?char_id=id&lvl=number
#define IF_OPEN_TIME                 "open_time"        // open_time?t=unix_timestamp
#define IF_RECHARGE                  "recharge"         // recharge?account=url_encode(account)&value=number&type=number
#define IF_SET_DIAMOND               "set_diamond"      // set_diamond?account=url_encode(account)&diamond=number
#define IF_SET_LTIME_ACT             "set_ltime_act"    /* set_ltime_act?id=id&begin_time=unix_timestamp
                                                           &end_time=unix_timestamp*/
#define IF_GM_ADD_FORBID_OPT         "forbid"           /* forbid?opt=type&begin_time=unix_timestamp
                                                           &end_time=unix_timestamp&target=url_encode(string) */
// 无id或id=0时是添加公告,有id时是更新已有公告
#define IF_NOTICE                    "notice"           /* notice?id=id&content=url_encode(string)&
                                                           begin_time=unix_timestamp&
                                                           end_time=unix_timestamp&
                                                           interval_time=second */
#define IF_GM_SEND_MAIL              "send_mail"        /* send_mail?char_id=id
                                                           &title=url_encode(string)
                                                           &content=url_encode(string)
                                                           &items=url_encode({"1":{"m_cid":21100001,"m_bind":1,"m_amt":5},
                                                           "2":{"m_cid":21100002,"m_bind":1,"m_amt":5}})
                                                           &coin=number
                                                           &diamond_b=number
                                                           &diamond=number */
#define IF_GM_SEND_MAIL_ALL          "all_send_mail"    /* all_send_mail?title=url_encode(string)
                                                           &content=url_encode(string)
                                                           &items=url_encode({"1":{"m_cid":21100001,"m_bind":1,"m_amt":5},
                                                           "2":{"m_cid":21100002,"m_bind":1,"m_amt":5}})
                                                           &coin=number
                                                           &diamond_b=number
                                                           &diamond=number
                                                           &level=number */
#define IF_ACTIVATION_CODE           "activation_code"  /* activation_code?char_id=id&content=url_encode(string)
                                                           &coin=number&b_diamond=number
                                                           &item=url_encode(cid:amount,cid:amount)*/

#define IF_PLATFORM_RECHARGE         "platform_recharge"// platform_recharge?trade_no=000&account=ttt&diamond=100&pay=10.00&platform=xxx&orderid=xxx&debug=0

#define IF_GET_RANK_DATA             "rank_data"        /* rank=lvl&n=number */   // rank=lvl,zhanli,jingji,guild,mstar

//= POST

//
#define IS_INTERFACE(IF)  ::strncmp(interface, IF, sizeof(IF) - 1) == 0

/**
 * @class web_service
 *
 * @brief
 */
class web_service : public http_srv
{
public:
  web_service();
  virtual ~web_service();

protected:
  int dispatch_get_interface(char *interface, char *param_begin);
  int dispatch_post_interface(char *interface, char *body);

  //= GET
  int if_cfg_update(char *param);
  int if_version(char *param);
  int if_online(char *param);
  int if_scene_online(char *param);
  int if_db_payload(char *param);
  int if_open_time(char *param);
  int if_recharge(char *param);
  int if_notice(char *param);
  int if_send_mail(char *param);
  int if_send_mail_all(char *param);
  int if_kick_out(char *param);
  int if_add_forbid_opt(char *param);
  int if_set_ltime_act(char *param);
  int if_platform_recharge(char *param);
  int if_activation_code(char *param);
  int if_to_lvl(char *param);
  int if_rank_data(char *param);
  int if_set_diamond(char *param);

  // = POST METHOD
  int handle_post_method();
private:
  void responed_result(const int ret, const char *, const char *body = NULL);
  void responed_pure_result(const char *desc);
};

#endif // WEB_SERVICE_H_

