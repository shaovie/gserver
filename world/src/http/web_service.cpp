#include "web_service.h"
#include "svc_monitor.h"
#include "all_char_info.h"
#include "notice_info.h"
#include "forbid_opt_module.h"
#include "gm_notice_module.h"
#include "forbid_opt.h"
#include "behavior_log.h"
#include "behavior_id.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "db_proxy.h"
#include "sys_log.h"
#include "time_util.h"
#include "istream.h"
#include "daemon.h"
#include "util.h"
#include "sys.h"
#include "def.h"
#include "mail_module.h"
#include "mail_info.h"
#include "order_serial.h"
#include "ltime_act.h"
#include "ltime_act_module.h"
#include "recharge_config.h"
#include "recharge_module.h"
#include "service_info.h"
#include "item_config.h"
#include "global_param_cfg.h"
#include "lvl_param_cfg.h"
#include "rank_module.h"
#include "jing_ji_module.h"
#include "guild_module.h"

// Lib header
#include <stdio.h>
#include <cstring>
#include "mblock.h"
#include "time_value.h"
#include "date_time.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("base");
static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *m_log = imp_log::instance()->get_ilog("base");

#define GM_CTRL_RESULT_OK       200
#define GM_CTRL_PARAM_ERR       101

namespace help
{
  inline int get_string_and_url_decode(const char *src,
                                       const char *sub_str,
                                       const int sub_str_len,
                                       char *out_str,
                                       const int out_str_len)
  {
    char *p = ::strstr((char *)src, sub_str);
    if (p == NULL) return -1;
    int len = ::strcspn(p + sub_str_len, "&");
    if (len == 0 || len >= out_str_len) return -1;

    ::strncpy(out_str, p + sub_str_len, len);
    util::php_url_decode(out_str, len);
    return 0;
  }
}
web_service::web_service() :
  http_srv("gm ctrl")
{ }
web_service::~web_service()
{ }
int web_service::dispatch_get_interface(char *interface, char *param_begin)
{
  if (IS_INTERFACE(IF_CFG_UPDATE))
    return this->if_cfg_update(param_begin);
  else if (IS_INTERFACE(IF_VERSION))
    return this->if_version(param_begin);
  else if (IS_INTERFACE(IF_ONLINE))
    return this->if_online(param_begin);
  else if (IS_INTERFACE(IF_SCENE_ONLINE))
    return this->if_scene_online(param_begin);
  else if (IS_INTERFACE(IF_DB_PAYLOAD))
    return this->if_db_payload(param_begin);
  else if (IS_INTERFACE(IF_OPEN_TIME))
    return this->if_open_time(param_begin);
  else if (IS_INTERFACE(IF_RECHARGE))
    return this->if_recharge(param_begin);
  else if (IS_INTERFACE(IF_SET_DIAMOND))
    return this->if_set_diamond(param_begin);
  else if (IS_INTERFACE(IF_NOTICE))
    return this->if_notice(param_begin);
  else if (IS_INTERFACE(IF_GM_SEND_MAIL))
    return this->if_send_mail(param_begin);
  else if (IS_INTERFACE(IF_GM_SEND_MAIL_ALL))
    return this->if_send_mail_all(param_begin);
  else if (IS_INTERFACE(IF_KICK_OUT))
    return this->if_kick_out(param_begin);
  else if (IS_INTERFACE(IF_GM_ADD_FORBID_OPT))
    return this->if_add_forbid_opt(param_begin);
  else if (IS_INTERFACE(IF_SET_LTIME_ACT))
    return this->if_set_ltime_act(param_begin);
  else if (IS_INTERFACE(IF_PLATFORM_RECHARGE))
    return this->if_platform_recharge(param_begin);
  else if (IS_INTERFACE(IF_ACTIVATION_CODE))
    return this->if_activation_code(param_begin);
  else if (IS_INTERFACE(IF_TO_LVL))
    return this->if_to_lvl(param_begin);
  else if (IS_INTERFACE(IF_GET_RANK_DATA))
    return this->if_rank_data(param_begin);

  e_log->error("invalid interface![%s]", interface);
  return -1;
}
// ---------------------------GET METHOD---------------------------
void web_service::responed_result(const int ret, const char *err, const char *body)
{
  char s_result[4096] = {0};
  int len = ::snprintf(s_result, sizeof(s_result),
                       "{\"result\": %d, \"err\": \"%s\"",
                       ret, err);
  if (body == NULL)
    len += ::snprintf(s_result + len, sizeof(s_result) - len, "}");
  else
    len += ::snprintf(s_result + len, sizeof(s_result) - len, ", %s}", body);

  char resp[4096] = {0};
  int r_len = ::snprintf(resp, sizeof(resp), "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=utf-8\r\n"
                         "Content-Length: %d\r\n"
                         "Server: iworld\r\n"
                         "Connection: close\r\n"
                         "\r\n%s",
                         len, s_result);
  this->send_data(resp, r_len);
}
int web_service::if_cfg_update(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  char *p = ::strchr(param, '=');
  if (p == NULL || *(p + 1) == '\0') return -1;
  p += 1;

  char result[2048] = {0};
  int result_len = 0;
  sys::reload_config(p, ::strlen(p), result, sizeof(result), result_len);

  this->responed_result(GM_CTRL_RESULT_OK, "ok", result);
  return -1;
}
int web_service::if_online(char * /*param*/)
{
  char bf[256] = {0};
  svc_monitor::instance()->do_build_online(bf, sizeof(bf));
  this->responed_result(GM_CTRL_RESULT_OK, "ok", bf);
  return -1;
}
int web_service::if_scene_online(char * /*param*/)
{
  char bf[512] = {0};
  svc_monitor::instance()->do_build_scene_online(bf, sizeof(bf));
  this->responed_result(GM_CTRL_RESULT_OK, "ok", bf);
  return -1;
}
int web_service::if_db_payload(char * /*param*/)
{
  char bf[256] = {0};
  svc_monitor::instance()->do_build_db_proxy_payload(bf, sizeof(bf));
  this->responed_result(GM_CTRL_RESULT_OK, "ok", bf);
  return -1;
}
int web_service::if_version(char * /*param*/)
{
  struct tm tb;
  strptime(__DATE__, "%b %d %Y", &tb);
  char date_bf[16] = {0};
  strftime(date_bf, sizeof(date_bf), "%Y-%m-%d", &tb);
  char build_time[64] = {0};
  ::snprintf(build_time, sizeof(build_time), "%s %s", date_bf, __TIME__);

  char s_buf[256] = {0};
  int result_len = 0;
  char *result = build_version_info(g_svc_name,
                                    SVC_EDITION,
                                    BIN_V,
                                    build_time,
                                    time_util::now - time_value::start_time.sec(),
                                    result_len);
  ::strncpy(s_buf, result, sizeof(s_buf));
  ::strncpy(s_buf + result_len, ", ", 2);
  result_len += 2;
  const char *db_version = db_proxy::instance()->version();
  ::strncpy(s_buf + result_len, db_version, sizeof(s_buf) - result_len);

  this->responed_result(GM_CTRL_RESULT_OK, "ok", s_buf);
  return -1;
}
int web_service::if_open_time(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  int t = util::get_value(param, "t=", sizeof("t=") - 1);
  if (t != sys::svc_info->open_time)
    sys::svc_info->opened = 0;
  sys::svc_info->open_time = t;

  sys::update_svc_info();
  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_recharge(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  int rc_type = util::get_value(param, "type=", sizeof("type=") - 1);
  int diamond = util::get_value(param, "value=", sizeof("value=") - 1);
  recharge_cfg_obj *rco = recharge_config::instance()->get_recharge_obj(rc_type);
  if (rco == NULL
      && (diamond <= 0 || diamond >= MONEY_UPPER_LIMIT))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "diamond invalid or account is null", NULL);
    return -1;
  }else if (rco != NULL)
    diamond = rco->diamond_;
  else
    rc_type = 0;

  char account[MAX_ACCOUNT_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "account=",
                                      sizeof("account=") - 1,
                                      account,
                                      sizeof(account)) != 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "account too long or error", NULL);
    return -1;
  }

  util::to_lower(account);
  player_obj *player = player_mgr::instance()->find(account);
  if (player == NULL || player->clt_state() != player_obj::CLT_ENTER_GAME_OK)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "player is offline", NULL);
    return -1;
  }

  if (player->is_money_upper_limit(M_DIAMOND, diamond))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "diamond out of limit", NULL);
    return -1;
  }
  if (rc_type == RECHARGE_MONTHLY_CARD
      && !recharge_module::can_buy_mc(player, time_util::now))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "can not buy month card!");
    return -1;
  }
  recharge_module::do_recharge(player,
                               diamond,
                               rc_type,
                               0,
                               "",
                               "");

  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_set_diamond(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  int diamond = util::get_value(param, "diamond=", sizeof("diamond=") - 1);
  if (diamond < 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "diamond error", NULL);
    return -1;
  }
  char account[MAX_ACCOUNT_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "account=",
                                      sizeof("account=") - 1,
                                      account,
                                      sizeof(account)) != 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "account too long or error", NULL);
    return -1;
  }

  util::to_lower(account);
  player_obj *player = player_mgr::instance()->find(account);
  if (player == NULL)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << 0 /*db_sid*/<< stream_ostr(account, ::strlen(account)) << diamond;
    db_proxy::instance()->send_request(::abs(account[0]), REQ_SET_DIAMOND, &os);
  }else if (player->clt_state() != player_obj::CLT_ENTER_GAME_OK)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "try again", NULL);
    return -1;
  }else if (player->diamond() > diamond)
  {
    player->do_lose_money(player->diamond() - diamond, M_DIAMOND, MONEY_LOSE_GM, 0, 0, 0);
  }else if (player->diamond() < diamond)
  {
    player->do_got_money(diamond - player->diamond(), M_DIAMOND, MONEY_GOT_GM, 0);
  }

  m_log->rinfo("GM set account[%s]'s diamond[%d] ok", account, diamond);
  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_kick_out(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  int char_id = util::get_value(param, "char_id=", sizeof("char_id=") - 1);
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "player is offline", NULL);
    return -1;
  }
  player->do_kick_out();

  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_to_lvl(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  int char_id = util::get_value(param, "char_id=", sizeof("char_id=") - 1);
  int to_lvl = util::get_value(param, "lvl=", sizeof("lvl=") - 1);
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "player is offline", NULL);
    return -1;
  }
  if (to_lvl < 2
      || to_lvl >= global_param_cfg::lvl_limit)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "lvl error", NULL);
    return -1;
  }
  while (player->lvl() < to_lvl)
  {
    int64_t add_exp = lvl_param_cfg::instance()->lvl_up_exp(player->lvl());
    if (add_exp > 0)
      player->do_got_exp(add_exp);
  }

  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_notice(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  notice_info ni;
  ni.id_            = util::get_value(param, "id=", sizeof("id=") - 1);
  ni.begin_time_    = util::get_value(param, "begin_time=", sizeof("begin_time=") - 1);
  ni.end_time_      = util::get_value(param, "end_time=", sizeof("end_time=") - 1);
  ni.interval_time_ = util::get_value(param, "interval_time=", sizeof("interval_time=") - 1);
  if (ni.id_ < 0
      || ni.begin_time_ < 0
      || ni.end_time_ < 0
      || ni.interval_time_ <= 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "time set wrong", NULL);
    return -1;
  }

  char buf[sizeof(ni.content_) * 3] = {0};
  if (help::get_string_and_url_decode(param,
                                      "content=",
                                      sizeof("content=") - 1,
                                      buf,
                                      sizeof(buf)) != 0
      || ::strlen(buf) >= sizeof(ni.content_)
      || !util::verify_db(buf))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "content too long or error", NULL);
    return -1;
  }
  ::strncpy(ni.content_, buf, sizeof(ni.content_) - 1);

  gm_notice_module::replace_notice(ni);

  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_send_mail(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param");
    return -1;
  }

  int char_id = util::get_value(param, "char_id=", sizeof("char_id=") - 1);
  int coin    = util::get_value(param, "coin=", sizeof("coin=") - 1);
  int diamond = util::get_value(param, "diamond=", sizeof("diamond=") - 1);
  int b_diamond = util::get_value(param, "diamond_b=", sizeof("diamond_b=") - 1);
  if (char_id <= 0 || coin < 0 || diamond < 0 || b_diamond < 0
      || all_char_info::instance()->get_char_brief_info(char_id) == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "param err");
    return -1;
  }

  char title[MAX_MAIL_TITLE_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "title=",
                                      sizeof("title=") - 1,
                                      title,
                                      sizeof(title)) != 0
      || ::strlen(title) >= MAX_MAIL_TITLE_LEN
      || !util::verify_db(title))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "title err");
    return -1;
  }

  char content[MAX_MAIL_CONTENT_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "content=",
                                      sizeof("content=") - 1,
                                      content,
                                      sizeof(content)) != 0
      || ::strlen(content) >= MAX_MAIL_CONTENT_LEN
      || !util::verify_db(content))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "content err");
    return -1;
  }

  char items[ALL_ITEMS_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "items=",
                                      sizeof("items=") - 1,
                                      items,
                                      sizeof(items)) != 0
      || ::strlen(items) >= ALL_ITEMS_LEN
      || !util::is_json(items)
      || items[0] != '{')
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "items err");
    return -1;
  }

  if (mail_module::do_send_mail(char_id,
                                mail_info::MAIL_SEND_SYSTEM_ID,
                                "GM", title, content,
                                mail_info::MAIL_TYPE_GM,
                                coin, diamond, b_diamond,
                                items,
                                0, time_util::now, 0) != 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "send mail logic err");
    return -1;
  }
  player_obj *reciver = player_mgr::instance()->find(char_id);
  if (reciver != NULL)
    mail_module::do_notify_haved_new_mail(reciver, 1);

  this->responed_result(GM_CTRL_RESULT_OK, "ok");
  return -1;
}
int web_service::if_send_mail_all(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param");
    return -1;
  }

  int coin    = util::get_value(param, "coin=", sizeof("coin=") - 1);
  int diamond = util::get_value(param, "diamond=", sizeof("diamond=") - 1);
  int b_diamond = util::get_value(param, "diamond_b=", sizeof("diamond_b=") - 1);
  int lvl     = util::get_value(param, "level=", sizeof("level=") - 1);
  if (coin < 0 || diamond < 0 || b_diamond < 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "param err");
    return -1;
  }

  char title[MAX_MAIL_TITLE_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "title=",
                                      sizeof("title=") - 1,
                                      title,
                                      sizeof(title)) != 0
      || ::strlen(title) > MAX_MAIL_TITLE_LEN
      || !util::verify_db(title))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "title err");
    return -1;
  }

  char content[MAX_MAIL_CONTENT_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "content=",
                                      sizeof("content=") - 1,
                                      content,
                                      sizeof(content)) != 0
      || ::strlen(content) >= MAX_MAIL_CONTENT_LEN
      || !util::verify_db(content))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "content err");
    return -1;
  }

  char items[ALL_ITEMS_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "items=",
                                      sizeof("items=") - 1,
                                      items,
                                      sizeof(items)) != 0
      || ::strlen(items) >= ALL_ITEMS_LEN
      || !util::is_json(items))
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "items err");
    return -1;
  }

  if (mail_module::do_send_mail_2_all("GM", title, content, coin, diamond, b_diamond, items, lvl) != 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "do send mail logic err");
    return -1;
  }

  this->responed_result(GM_CTRL_RESULT_OK, "ok");
  return -1;
}
int web_service::if_add_forbid_opt(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param");
    return -1;
  }
  forbid_opt fo;
  fo.opt_ = util::get_value(param, "opt=", sizeof("opt=") - 1);
  fo.begin_time_ = util::get_value(param, "begin_time=", sizeof("begin_time=") - 1);
  fo.end_time_ = util::get_value(param, "end_time=", sizeof("end_time=") - 1);
  if (fo.opt_ <= 0
      || fo.opt_ >= forbid_opt::FORBID_END
      || fo.begin_time_ <= 0
      || fo.end_time_ <= 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "opt or time error");
    return -1;
  }
  char target[MAX_ACCOUNT_LEN * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "target=",
                                      sizeof("target=") - 1,
                                      target,
                                      sizeof(target)) != 0
      || ::strlen(target) > MAX_ACCOUNT_LEN)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "target err");
    return -1;
  }

  ::strncpy(fo.target_, target, sizeof(fo.target_) - 1);
  forbid_opt_module::do_insert_forbid_opt(fo, false);
  this->responed_result(GM_CTRL_RESULT_OK, "ok");
  return -1;
}
int web_service::if_set_ltime_act(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param");
    return -1;
  }
  ltime_act lta;
  lta.act_id_ = util::get_value(param, "id=", sizeof("id=") - 1);
  lta.begin_time_ = util::get_value(param, "begin_time=", sizeof("begin_time=") - 1);
  lta.end_time_ = util::get_value(param, "end_time=", sizeof("end_time=") - 1);
  if (lta.act_id_ <= 0
      || lta.begin_time_ <= 0
      || lta.end_time_ <= 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "id or time error");
    return -1;
  }

  if (time_util::now - sys::svc_launch_time < LTIME_ACT_SET_LIMITED_TIME+2)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "please wait a moment!");
    return -1;
  }

  s_log->rinfo("set ltime act:[%s]", param);
  ltime_act_module::do_update_act(&lta, false);
  this->responed_result(GM_CTRL_RESULT_OK, "ok");
  return -1;
}
void web_service::responed_pure_result(const char *desc)
{
  static char resp[1024] = {0};
  int r_len = ::snprintf(resp, sizeof(resp), "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=utf-8\r\n"
                         "Content-Length: %ld\r\n"
                         "Server: iWorld\r\n"
                         "Connection: close\r\n"
                         "\r\n%s",
                         (desc != NULL ? ::strlen(desc) : 0),
                         (desc != NULL ? desc : ""));
  this->send_data(resp, r_len);
}
int web_service::if_platform_recharge(char *param)
{
  if (param == NULL) return -1;
  m_log->rinfo("platform recharge [%s]", param);

  int rc_type = util::get_value(param, "rc_type=", sizeof("rc_type=") - 1);
  recharge_cfg_obj *rco = recharge_config::instance()->get_recharge_obj(rc_type);
  if (rco == NULL)
  {
    m_log->rinfo("recharge failed! rc_type %d error!", rc_type);
    this->responed_pure_result("PARAM ERROR");
    return -1;
  }
  int debug = util::get_value(param, "debug=", sizeof("debug=") - 1);
  if (debug < 0)
  {
    m_log->rinfo("recharge failed! debug %d error!", debug);
    this->responed_pure_result("DEBUG ERROR");
    return -1;
  }
  char trade_no[32*3 + 1] = {0};
  char account[MAX_ACCOUNT_LEN*3 + 1] = {0};
  char pay[16*3 + 1] = {0};
  char platform[MAX_CHANNEL_LEN*3 + 1] = {0};
  char orderid[16*3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "trade_no=",
                                      sizeof("trade_no=") - 1,
                                      trade_no,
                                      sizeof(trade_no)) != 0
      || help::get_string_and_url_decode(param,
                                         "account=",
                                         sizeof("account=") - 1,
                                         account,
                                         sizeof(account)) != 0
      || help::get_string_and_url_decode(param,
                                         "pay=",
                                         sizeof("pay=") - 1,
                                         pay,
                                         sizeof(pay)) != 0
      || help::get_string_and_url_decode(param,
                                         "platform=",
                                         sizeof("platform=") - 1,
                                         platform,
                                         sizeof(platform)) != 0
      || help::get_string_and_url_decode(param,
                                         "orderid=",
                                         sizeof("orderid=") - 1,
                                         orderid,
                                         sizeof(orderid)) != 0
      ) // end of `if'
  {
    m_log->rinfo("recharge failed! other param error !");
    this->responed_pure_result("TRADE_NO NOT EXIST");
    return -1;
  }

  if (!order_serial::instance()->find(trade_no))
  {
    m_log->rinfo("%s recharge failed! trade_no[%s] not exist!", platform, trade_no);
    this->responed_pure_result("TRADE_NO NOT EXIST");
    return -1;
  }

  out_stream os(NULL, 0);
  player_obj *player = NULL;
  util::to_lower(account);
  if (all_char_info::instance()->get_account_brief_info(account) == NULL)
  {
    m_log->rinfo("%s recharge failed! account[%s] not exist!", platform, account);
    this->responed_pure_result("ACCOUNT NOT EXIST");
    goto END;
  }

  player = player_mgr::instance()->find(account);
  if (player == NULL || player->clt_state() != player_obj::CLT_ENTER_GAME_OK)
  {
    m_log->rinfo("%s recharge failed! account[%s] offline !", platform, account);
    this->responed_pure_result("ACCOUNT OFFLINE");
    return -1;
  }

  if (player->is_money_upper_limit(M_DIAMOND, rco->diamond_))
  {
    m_log->rinfo("%s recharge failed! account[%s]'s diamond is out of limit !", platform, account);
    this->responed_pure_result("DIAMOND OUT OF LIMIT");
    goto END;
  }

  if (rc_type == RECHARGE_MONTHLY_CARD
      && !recharge_module::can_buy_mc(player, time_util::now))
  {
    m_log->rinfo("%s recharge failed! account[%s]'s monthly card isn't invalidation !", platform, account);
    this->responed_pure_result("MC IS NOT INVALIDATION");
    goto END;
  }

  // appstore 没有 pay 信息
  if (::strcmp(pay, "apple") == 0)
    ::snprintf(pay, sizeof(pay), "%d", rco->charge_rmb_);

  // 快用 存在使用固定面额卡片充值，会扣掉所有钱，以至于高于定价
  if (rco->charge_rmb_ > int(::atof(pay)))
  {
    m_log->rinfo("%s recharge failed! paying money[%f]rmb is error!", platform, ::atof(pay));
    this->responed_pure_result("Price ERROR");
    goto END;
  }

  recharge_module::do_recharge(player,
                               rco->diamond_,
                               rc_type,
                               int(::atof(pay) * 100),
                               orderid,
                               platform);

  this->responed_pure_result("SUCCESS");

  os.reset(client::send_buf, client::send_buf_len);
  os << stream_ostr((const char *)trade_no, ::strlen(trade_no));
  player->send_request(NTF_SUCCESS_TRADE_NO, &os);
END:
  order_serial::instance()->destroy(trade_no);
  return -1;
}
int web_service::if_activation_code(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }

  const size_t max_content_len = 90;
  char content[max_content_len * 3 + 1] = {0};
  if (help::get_string_and_url_decode(param,
                                      "content=",
                                      sizeof("content=") - 1,
                                      content,
                                      sizeof(content)) != 0
      || ::strlen(content) >= max_content_len)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "content err");
    return -1;
  }
  int char_id = util::get_value(param, "char_id=", sizeof("char_id=") - 1);
  int coin = util::get_value(param, "coin=", sizeof("coin=") - 1);
  int b_diamond = util::get_value(param, "b_diamond=", sizeof("b_diamond=") - 1);
  s_log->rinfo("coin = %d, b_diamond = %d", coin, b_diamond);

  char item_list_bf[256] = {0};
  ilist<item_amount_bind_t> item_list;
  if (help::get_string_and_url_decode(param,
                                      "item=",
                                      sizeof("item=") - 1,
                                      item_list_bf,
                                      sizeof(item_list_bf)) == 0)
  {
    if (::strlen(item_list_bf) >= sizeof(item_list_bf))
    {
      this->responed_result(GM_CTRL_PARAM_ERR, "item err");
      return -1;
    }
    char *tok_p = NULL;
    char *token = NULL;
    for (token = ::strtok_r(item_list_bf, ",", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ",", &tok_p))
    {
      int item_cid = 0;
      int amount = 0;
      if (::sscanf(token, "%d:%d", &item_cid, &amount) != 2)
      {
        this->responed_result(GM_CTRL_PARAM_ERR, "item err");
        return -1;
      }
      if (!item_config::instance()->find(item_cid))
      {
        this->responed_result(GM_CTRL_PARAM_ERR, "item_cid error", NULL);
        return -1;
      }
      item_list.push_back(item_amount_bind_t(item_cid, amount, BIND_TYPE));
    }
  }

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "player is offline", NULL);
    return -1;
  }

  if (coin + player->coin() > MONEY_UPPER_LIMIT)
    coin = MONEY_UPPER_LIMIT - player->coin();
  if (b_diamond + player->b_diamond() > MONEY_UPPER_LIMIT)
    b_diamond = MONEY_UPPER_LIMIT - player->b_diamond();
  player->do_exchange_activation_code(content,
                                      coin,
                                      b_diamond,
                                      item_list);
  this->responed_result(GM_CTRL_RESULT_OK, "ok", NULL);
  return -1;
}
int web_service::if_rank_data(char *param)
{
  if (param == NULL)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "no param", NULL);
    return -1;
  }
  char rank_type[32] = {0};
  if (help::get_string_and_url_decode(param,
                                      "rank=",
                                      sizeof("rank=") - 1,
                                      rank_type,
                                      sizeof(rank_type)) != 0)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "rank err");
    return -1;
  }
  int n = util::get_value(param, "n=", sizeof("n=") - 1);
  if (n <= 0 || n > global_param_cfg::rank_amount)
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "n err");
    return -1;
  }
  ilist<int> id_list;
  if (::strcmp(rank_type, "lvl") == 0)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_LVL,
                                           id_list,
                                           n);
  }else if (::strcmp(rank_type, "zhanli") == 0)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_ZHANLI,
                                           id_list,
                                           n);
  }else if (::strcmp(rank_type, "mstar") == 0)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_MSTAR,
                                           id_list,
                                           n);
  }else if (::strcmp(rank_type, "jingji") == 0)
  {
    jing_ji_module::get_char_list_before_rank(id_list, n);
  }else if (strcmp(rank_type, "guild") == 0)
  {
    guild_module::get_guild_list_before_rank(id_list, n);
  }else
  {
    this->responed_result(GM_CTRL_PARAM_ERR, "rank err");
    return -1;
  }
  char buf[1024] = {0};
  int len = ::snprintf(buf, sizeof(buf), "\"id\":[");
  while (!id_list.empty())
  {
    int char_id = id_list.pop_front();
    len += ::snprintf(buf + len, sizeof(buf) - len, "%d,", char_id);
  }
  if (buf[len - 1] == ',')
    len -= 1;
  ::snprintf(buf + len, sizeof(buf) - len, "]");

  this->responed_result(GM_CTRL_RESULT_OK, "ok", buf);
  return -1;
}
// --------------------------POST METHOD---------------------------
int web_service::dispatch_post_interface(char *interface, char * /*body*/)
{
  e_log->error("invalid interface![%s]", interface);
  return -1;
}
