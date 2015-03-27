#include "db_proxy_logic.h"
#include "db_session_mgr.h"
#include "web_service.h"
#include "player_obj.h"
#include "global_macros.h"
#include "sys_log.h"
#include "istream.h"
#include "message.h"
#include "error.h"
#include "sys_settings.h"
#include "daily_clean_info.h"
#include "char_msg_queue_module.h"
#include "social_module.h"
#include "task_module.h"
#include "skill_module.h"
#include "passive_skill_module.h"
#include "package_module.h"
#include "svc_monitor.h"
#include "buff_module.h"
#include "mail_module.h"
#include "guild_module.h"
#include "tui_tu_module.h"
#include "scp_module.h"
#include "rank_module.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "bao_shi_module.h"
#include "title_module.h"
#include "huo_yue_du_module.h"
#include "jing_ji_module.h"
#include "lucky_turn_module.h"
#include "ltime_act_module.h"
#include "cross_module.h"
#include "all_char_info.h"
#include "mobai_module.h"
#include "tianfu_skill_module.h"
#include "lue_duo_module.h"
#include "dxc_module.h"
#include "baowu_mall_info.h"
#include "kai_fu_act_obj.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("db_proxy");
static ilog_obj *e_log = err_log::instance()->get_ilog("db_proxy");

#define DB_HANDLE_RESULT_SHORT_CODE(M, TB) \
{ \
  int db_sid = -1; \
  int arg = 0; \
  in_stream is(msg, len); \
  is >> db_sid >> arg; \
  player_obj *player = db_session_mgr::instance()->find(db_sid); \
  if (player == NULL) \
  { \
    s_log->rinfo("not found session %d when %s", db_sid, __func__); \
    return 0; \
  } \
  if (res == DB_ASYNC_OK) \
  { \
    M::handle_db_get_##TB##_result(player, is); \
  }else \
  { \
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res); \
  } \
  return 0; \
}

#define DB_ERROR_RESULT_SHORT_CODE(TB) \
{ \
  int sid = -1; \
  in_stream is(msg, len); \
  is >> sid; \
  player_obj *player = db_session_mgr::instance()->find(sid); \
  if (player == NULL) \
  { \
    s_log->wning("not found db session %d when %s", sid, __func__); \
  } \
  else \
  { \
    e_log->rinfo(#TB " db error char = %d res = %d", player->id(), res); \
  } \
  return 0; \
}

int db_proxy_logic::dispatch_msg(const int id,
                                 const int res,
                                 const char *msg,
                                 const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:       \
  ret = this->FUNC(msg, len, res);          \
  break
#define SHORT_DEFAULT default:              \
  e_log->error("unknow msg id %d", id);     \
  break
  //
  int ret = 0;
  switch (id)
  {
    // account
    SHORT_CODE(RES_GET_ACCOUNT_INFO, handle_get_account_info_result);
    SHORT_CODE(RES_INSERT_ACCOUNT_INFO, handle_insert_account_info_err_result);

    // sys settings
    SHORT_CODE(RES_GET_SYS_SETTINGS, handle_get_sys_settings_result);

    // char info
    SHORT_CODE(RES_GET_NEW_CHAR_INFO, handle_get_new_char_info_result);
    SHORT_CODE(RES_GET_CHAR_INFO, handle_get_char_info_result);
    SHORT_CODE(RES_UPDATE_CHAR_INFO, handle_char_info_err_result);
    SHORT_CODE(RES_INSERT_CHAR_INFO, handle_char_info_err_result);

    // char extra info
    SHORT_CODE(RES_GET_CHAR_EXTRA_INFO, handle_get_char_extra_info_result);
    SHORT_CODE(RES_UPDATE_CHAR_EXTRA_INFO, handle_char_extra_info_err_result);
    SHORT_CODE(RES_INSERT_CHAR_EXTRA_INFO, handle_char_extra_info_err_result);

    // char daily clean info
    SHORT_CODE(RES_GET_DAILY_CLEAN_INFO, handle_get_daily_clean_info_result);
    SHORT_CODE(RES_UPDATE_DAILY_CLEAN_INFO, handle_daily_clean_info_err_result);
    SHORT_CODE(RES_INSERT_DAILY_CLEAN_INFO, handle_daily_clean_info_err_result);

    //char db msg queue
    SHORT_CODE(RES_GET_CHAR_DB_MSG_LIST, handle_get_char_msg_list_result);
    SHORT_CODE(RES_DELETE_CHAR_DB_MSG, handle_char_msg_queue_err_result);
    SHORT_CODE(RES_INSERT_CHAR_DB_MSG, handle_char_msg_queue_err_result);

    // social
    SHORT_CODE(RES_GET_SOCIAL_LIST, handle_get_social_list_result);
    SHORT_CODE(RES_GET_SEARCH_INFO, handle_get_search_list_result);
    SHORT_CODE(RES_INSERT_SOCIAL, handle_social_err_result);
    SHORT_CODE(RES_UPDATE_SOCIAL, handle_social_err_result);
    SHORT_CODE(RES_DELETE_SOCIAL, handle_social_err_result);

    // task
    SHORT_CODE(RES_GET_TASK_LIST, handle_get_task_list_result);
    SHORT_CODE(RES_INSERT_TASK, handle_task_info_err_result);
    SHORT_CODE(RES_UPDATE_TASK, handle_task_info_err_result);
    SHORT_CODE(RES_DELETE_TASK, handle_task_info_err_result);

    // task_bit_array
    SHORT_CODE(RES_GET_TASK_BIT_ARRAY, handle_get_task_bit_arrary);
    SHORT_CODE(RES_UPDATE_TASK_BIT_ARRAY, handle_task_bit_array_err_result);

    // skill
    SHORT_CODE(RES_GET_SKILL_LIST, handle_get_skill_list_result);
    SHORT_CODE(RES_GET_OTHER_SKILL_LIST, handle_get_other_skill_list_result);
    SHORT_CODE(RES_INSERT_SKILL, handle_skill_err_result);
    SHORT_CODE(RES_UPDATE_SKILL, handle_skill_err_result);

    // passive skill
    SHORT_CODE(RES_GET_PASSIVE_SKILL_LIST, handle_get_passive_skill_list_result);
    SHORT_CODE(RES_UPDATE_PASSIVE_SKILL, handle_passive_skill_err_result);

    // tianfu skill
    SHORT_CODE(RES_GET_TIANFU_SKILL_LIST, handle_get_tianfu_skill_list_result);
    SHORT_CODE(RES_UPDATE_TIANFU_SKILL, handle_tianfu_skill_err_result);

    // item
    SHORT_CODE(RES_GET_ITEM_SINGLE_PKG, handle_get_item_single_pkg_result);
    SHORT_CODE(RES_GET_ITEM_LIST, handle_get_item_list_result);
    SHORT_CODE(RES_INSERT_ITEM, handle_item_err_result);
    SHORT_CODE(RES_UPDATE_ITEM, handle_item_err_result);
    SHORT_CODE(RES_DELETE_ITEM, handle_item_err_result);
    SHORT_CODE(RES_CROSS_GET_ITEM_SINGLE_PKG, handle_cross_get_item_single_pkg_result);
    SHORT_CODE(RES_GET_CASTELLAN_ITEM_SINGLE_PKG, handle_get_castellan_item_single_pkg);

    // buff
    SHORT_CODE(RES_GET_BUFF_LIST, handle_get_buff_list_result);
    SHORT_CODE(RES_UPDATE_BUFF_LIST, handle_buff_err_result);

    // tui tu
    SHORT_CODE(RES_GET_TUI_TU_LOG, handle_get_tui_tu_log_result);
    SHORT_CODE(RES_INSERT_TUI_TU_LOG, handle_tui_tu_err_result);
    SHORT_CODE(RES_UPDATE_TUI_TU_LOG, handle_tui_tu_err_result);

    // scp
    SHORT_CODE(RES_GET_SCP_LOG, handle_get_scp_log_result);
    SHORT_CODE(RES_INSERT_SCP_LOG, handle_scp_err_result);
    SHORT_CODE(RES_UPDATE_SCP_LOG, handle_scp_err_result);
    SHORT_CODE(RES_CLEAR_SCP_LOG,  handle_scp_err_result);

    // mail
    SHORT_CODE(RES_GET_MAIL_LIST,        handle_get_mail_list);
    SHORT_CODE(RES_GET_MAIL_INFO,        handle_get_mail_info);
    SHORT_CODE(RES_INSERT_MAIL,          handle_mail_err_result);
    SHORT_CODE(RES_UPDATE_MAIL_ATTACH,   handle_mail_err_result);
    SHORT_CODE(RES_UPDATE_MAIL_READED,   handle_mail_err_result);
    SHORT_CODE(RES_UPDATE_MAIL_DELETE,   handle_mail_err_result);
    SHORT_CODE(RES_CHECK_NEW_MAIL,       handle_check_new_mail);
    SHORT_CODE(RES_GET_MAIL_ATTACH,      handle_get_mail_attach);
    SHORT_CODE(RES_GET_SYSTEM_MAIL_LIST, handle_get_system_mail_list);

    SHORT_CODE(RES_GET_MALL_BUY_LOG,     handle_get_mall_buy_log);
    SHORT_CODE(RES_GET_BAOWU_MALL_INFO,  handle_get_baowu_mall_info);

    // guild
    SHORT_CODE(RES_INSERT_GUILD, handle_guild_err_result);
    SHORT_CODE(RES_DELETE_GUILD, handle_guild_err_result);
    SHORT_CODE(RES_UPDATE_GUILD, handle_guild_err_result);
    SHORT_CODE(RES_INSERT_GUILD_MEMBER, handle_guild_err_result);
    SHORT_CODE(RES_DELETE_GUILD_MEMBER, handle_guild_err_result);
    SHORT_CODE(RES_UPDATE_GUILD_MEMBER, handle_guild_err_result);
    SHORT_CODE(RES_INSERT_GUILD_APPLY, handle_guild_err_result);
    SHORT_CODE(RES_DELETE_GUILD_APPLY, handle_guild_err_result);
    SHORT_CODE(RES_DELETE_GUILD_APPLYS, handle_guild_err_result);
    SHORT_CODE(RES_GET_GUILD_SKILL_LIST, handle_get_guild_skill_list_result);
    SHORT_CODE(RES_INSERT_GUILD_SKILL, handle_guild_err_result);
    SHORT_CODE(RES_UPDATE_GUILD_SKILL, handle_guild_err_result);

    // vip
    SHORT_CODE(RES_GET_VIP, handle_get_vip_result);
    SHORT_CODE(RES_INSERT_VIP, handle_vip_err_result);
    SHORT_CODE(RES_UPDATE_VIP, handle_vip_err_result);

    // cheng jiu
    SHORT_CODE(RES_GET_CHENG_JIU_INFO, handle_get_cheng_jiu_result);
    SHORT_CODE(RES_UPDATE_CHENG_JIU_INFO, handle_cheng_jiu_err_result);

    // title
    SHORT_CODE(RES_GET_TITLE_INFO, handle_get_title_result);
    SHORT_CODE(RES_INSERT_TITLE_INFO, handle_title_err_result);

    // bao_shi
    SHORT_CODE(RES_GET_BAO_SHI, handle_get_bao_shi_result);
    SHORT_CODE(RES_INSERT_BAO_SHI, handle_bao_shi_err_result);
    SHORT_CODE(RES_UPDATE_BAO_SHI, handle_bao_shi_err_result);

    // huo yue du
    SHORT_CODE(RES_GET_HUO_YUE_DU_INFO, handle_get_huo_yue_du_result);
    SHORT_CODE(RES_UPDATE_HUO_YUE_DU_INFO, handle_huo_yue_du_err_result);
    SHORT_CODE(RES_CLEAR_HUO_YUE_DU_INFO, handle_huo_yue_du_err_result);

    // jing ji log
    SHORT_CODE(RES_GET_JING_JI_LOG, handle_get_jing_ji_log_result);
    SHORT_CODE(RES_INSERT_JING_JI_LOG, handle_jing_ji_log_err_result);

    // char recharge
    SHORT_CODE(RES_GET_CHAR_RECHARGE, handle_get_char_recharge_result);
    SHORT_CODE(RES_UPDATE_CHAR_RECHARGE, handle_char_recharge_err_result);

    // lucky turn
    SHORT_CODE(RES_GET_LUCKY_TURN_BIG_AWARD_LIST, handle_get_lucky_turn_big_award_list_result);

    // ltime recharge time
    SHORT_CODE(RES_GET_LTIME_RECHARGE_AWARD, handle_get_ltime_recharge_award_result);
    SHORT_CODE(RES_UPDATE_LTIME_RECHARGE_AWARD, handle_ltime_recharge_award_err_result);

    // lueduo
    SHORT_CODE(RES_GET_LUEDUO_LOG, handle_get_lueduo_log_result);
    SHORT_CODE(RES_UPDATE_LUEDUO_LOG, handle_lueduo_err_result);
    SHORT_CODE(RES_INSERT_LUEDUO_LOG, handle_lueduo_err_result);
    SHORT_CODE(RES_DELETE_LUEDUO_LOG, handle_lueduo_err_result);

    // di xia cheng
    SHORT_CODE(RES_GET_DXC_INFO, handle_get_dxc_info_result);
    SHORT_CODE(RES_UPDATE_DXC_INFO, handle_dxc_err_result);
    SHORT_CODE(RES_INSERT_DXC_INFO, handle_dxc_err_result);

    // kai fu act log
    SHORT_CODE(RES_GET_KAI_FU_ACT_LOG, handle_get_kai_fu_act_log_result);
    SHORT_CODE(RES_INSERT_KAI_FU_ACT_LOG, handle_kai_fu_act_log_err_result);

    SHORT_CODE(RES_DB_PROXY_PAYLOAD, handle_get_db_proxy_payload_result);
    SHORT_DEFAULT;
  }

  // log warning
  if (ret != 0)
    e_log->wning("handle msg %d return %d", id, ret);
  return 0;
}
int player_obj::to_load_all_db_info(const int char_id)
{
#define LOAD_DB_SHORT_CODE(MSG)    \
  { \
    out_stream os(client::send_buf, client::send_buf_len); \
    os << this->db_sid_ << char_id; \
    if (db_proxy::instance()->send_request(char_id, MSG, &os) != 0) \
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED; \
  }

  LOAD_DB_SHORT_CODE(REQ_GET_CHAR_EXTRA_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_DAILY_CLEAN_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_SKILL_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_PASSIVE_SKILL_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_ITEM_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_BUFF_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_GUILD_SKILL_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_TIANFU_SKILL_LIST);
  LOAD_DB_SHORT_CODE(REQ_GET_VIP);
  LOAD_DB_SHORT_CODE(REQ_GET_BAO_SHI);
  LOAD_DB_SHORT_CODE(REQ_GET_TITLE_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_CHENG_JIU_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_SCP_LOG);
  LOAD_DB_SHORT_CODE(REQ_GET_CHAR_RECHARGE);
  LOAD_DB_SHORT_CODE(REQ_GET_SYS_SETTINGS);
  LOAD_DB_SHORT_CODE(REQ_GET_TUI_TU_LOG);
  LOAD_DB_SHORT_CODE(REQ_GET_LUEDUO_LOG);
  LOAD_DB_SHORT_CODE(REQ_GET_DXC_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_BAOWU_MALL_INFO);
  LOAD_DB_SHORT_CODE(REQ_GET_KAI_FU_ACT_LOG);

  // MUST BE LAST ONE
  LOAD_DB_SHORT_CODE(REQ_GET_CHAR_INFO);

  this->clt_state_ = CLT_TO_LOAD_ALL_DB;
  return 0;
}
int db_proxy_logic::handle_get_db_proxy_payload_result(const char *msg,
                                                       const int len,
                                                       const int )
{
  svc_monitor::instance()->handle_get_db_proxy_payload_result(msg, len);
  return 0;
}
int db_proxy_logic::handle_get_account_info_result(const char *msg,
                                                   const int len,
                                                   const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char ac_info_bf[sizeof(account_info) + 4] = {0};
      stream_istr ac_info_si(ac_info_bf, sizeof(ac_info_bf));
      is >> ac_info_si;
      if (ac_info_si.str_len() == sizeof(account_info))
      {
        player_obj *player = db_session_mgr::instance()->find(db_sid);
        if (player != NULL)
        {
          player->handle_db_get_account_info_result((account_info *)ac_info_bf);
          return 0;
        }else // 如果新用户刚上来就断线了，那么这个帐号就没有添加到all_char_info中，就永远不会被追加进去了
        {
          account_info *ai = (account_info *)ac_info_bf;
          account_brief_info *abi =
            all_char_info::instance()->get_account_brief_info(ai->account_);
          if (abi == NULL)
          {
            abi = new account_brief_info();
            all_char_info::instance()->insert_account_info(ai->account_, abi);
          }
          return 0;
        }
      }
    }
  }else
    e_log->rinfo("%s account error res = %d", __func__, res);

  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player != NULL)
    player->handle_db_get_account_info_result(NULL);
  return 0;
}
int db_proxy_logic::handle_get_sys_settings_result(const char *msg,
                                                   const int len,
                                                   const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->debug("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char ac_info_bf[sizeof(sys_settings) + 4] = {0};
      stream_istr ac_info_si(ac_info_bf, sizeof(ac_info_bf));
      is >> ac_info_si;
      sys_settings ss;
      in_stream ss_is(ac_info_si.str(), ac_info_si.str_len());
      ss_is >> &ss;
      player->handle_db_get_sys_settings_result(&ss);
      return 0;
    }
  }else
    e_log->rinfo("%s account [%s] error res = %d", __func__, player->account(), res);

  player->handle_db_get_sys_settings_result(NULL);
  return 0;
}
int db_proxy_logic::handle_insert_account_info_err_result(const char *msg,
                                                          const int len,
                                                          const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  e_log->rinfo("insert account [%s] err res = %d", player->account(), res);
  return 0;
}
int db_proxy_logic::handle_get_new_char_info_result(const char *msg,
                                                    const int len,
                                                    const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(char_info) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(char_info))
      {
        player->handle_db_get_new_char_info_result((char_info *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  player->handle_db_get_new_char_info_result(NULL);
  return 0;
}
int db_proxy_logic::handle_get_char_info_result(const char *msg,
                                                const int len,
                                                const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(char_info) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(char_info))
      {
        player->handle_db_get_char_info_result((char_info *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  player->handle_db_get_char_info_result(NULL);
  return 0;
}
int db_proxy_logic::handle_char_info_err_result(const char *msg,
                                                const int len,
                                                const int res)
{ DB_ERROR_RESULT_SHORT_CODE(char_info); }
int db_proxy_logic::handle_get_char_extra_info_result(const char *msg,
                                                      const int len,
                                                      const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(char_extra_info) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(char_extra_info))
      {
        player->handle_db_get_char_extra_info_result((char_extra_info *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  player->handle_db_get_char_extra_info_result(NULL);
  return 0;
}
int db_proxy_logic::handle_char_extra_info_err_result(const char *msg,
                                                      const int len,
                                                      const int res)
{ DB_ERROR_RESULT_SHORT_CODE(char_extra_info); }
int db_proxy_logic::handle_get_daily_clean_info_result(const char *msg,
                                                       const int len,
                                                       const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(daily_clean_info) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(daily_clean_info))
      {
        player->handle_db_get_daily_clean_info_result((daily_clean_info *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  player->handle_db_get_daily_clean_info_result(NULL);
  return 0;
}
int db_proxy_logic::handle_daily_clean_info_err_result(const char *msg,
                                                       const int len,
                                                       const int res)
{ DB_ERROR_RESULT_SHORT_CODE(daily_clean_info); }
int db_proxy_logic::handle_get_mall_buy_log(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    player->handle_db_get_mall_buy_log_result(is);
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);
  return 0;
}
int db_proxy_logic::handle_get_baowu_mall_info(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(baowu_mall_info) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(baowu_mall_info))
      {
        player->handle_db_get_baowu_mall_info_result((baowu_mall_info *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);
  return 0;
}
int db_proxy_logic::handle_get_item_single_pkg_result(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  in_stream is(msg, len);
  is >> db_sid;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }
  if (res == DB_ASYNC_OK)
  {
    player->handle_db_get_item_single_pkg_result(is);
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  return 0;
}
int db_proxy_logic::handle_cross_get_item_single_pkg_result(const char *msg, const int len, const int res)
{
  if (res == DB_ASYNC_OK)
  {
    cross_module::handle_db_cross_get_item_single_pkg_result(msg, len);
  }else
    e_log->rinfo("%s error cross res = %d", __func__, res);

  return 0;
}
int db_proxy_logic::handle_get_castellan_item_single_pkg(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  in_stream is(msg, len);
  is >> db_sid;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }
  if (res == DB_ASYNC_OK)
  {
    mobai_module::handle_db_get_castellan_item_single_pkg_result(player, is);
  }else
    e_log->rinfo("%s error castellan res = %d", __func__, player->account(), res);

  return 0;
}
// char recharge
int db_proxy_logic::handle_get_char_recharge_result(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  int arg = 0;

  in_stream is(msg, len);
  is >> db_sid >> arg;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }

  if (res == DB_ASYNC_OK)
  {
    int res_cnt = 0;
    is >> res_cnt;
    if (res_cnt > 0)
    {
      char info_bf[sizeof(char_recharge) + 4] = {0};
      stream_istr info_si(info_bf, sizeof(info_bf));
      is >> info_si;
      if (info_si.str_len() == sizeof(char_recharge))
      {
        player->handle_db_get_char_recharge_result((char_recharge *)info_bf);
        return 0;
      }
    }
  }else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);

  return 0;
}
int db_proxy_logic::handle_char_recharge_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(char_recharge); }
int db_proxy_logic::handle_get_social_list_result(const char *msg,
                                                  const int len,
                                                  const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(social_module, social_list); }
int db_proxy_logic::handle_get_search_list_result(const char *msg,
                                                  const int len,
                                                  const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(social_module, search_list); }
int db_proxy_logic::handle_social_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(social); }

int db_proxy_logic::handle_get_char_msg_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(char_msg_queue_module, char_msg_queue_list); }
int db_proxy_logic::handle_char_msg_queue_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(char_msg_queue); }

int db_proxy_logic::handle_get_task_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(task_module, task_list); }
int db_proxy_logic::handle_task_info_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(task_info); }

int db_proxy_logic::handle_get_task_bit_arrary(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(task_module, task_bit_array); }
int db_proxy_logic::handle_task_bit_array_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(task_bit_array); }

int db_proxy_logic::handle_get_skill_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(skill_module, skill_list); }
int db_proxy_logic::handle_get_other_skill_list_result(const char *msg, const int len, const int res)
{
  int db_sid = -1;
  in_stream is(msg, len);
  is >> db_sid;
  player_obj *player = db_session_mgr::instance()->find(db_sid);
  if (player == NULL)
  {
    s_log->rinfo("not found session %d when %s", db_sid, __func__);
    return 0;
  }
  if (res == DB_ASYNC_OK)
    skill_module::handle_db_get_other_skill_list_result(player, is);
  else
    e_log->rinfo("%s error account = %s res = %d", __func__, player->account(), res);
  return 0;
}
int db_proxy_logic::handle_skill_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(skill_info); }

int db_proxy_logic::handle_get_passive_skill_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(passive_skill_module, passive_skill_list); }
int db_proxy_logic::handle_passive_skill_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(passive_skill_info); }

int db_proxy_logic::handle_get_tianfu_skill_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(tianfu_skill_module, tianfu_skill_list); }
int db_proxy_logic::handle_tianfu_skill_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(tianfu_skill_info); }

int db_proxy_logic::handle_get_item_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(package_module, item_list); }
int db_proxy_logic::handle_item_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(item_obj); }

int db_proxy_logic::handle_get_buff_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(buff_module, buff_list); }
int db_proxy_logic::handle_buff_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(buff); }

int db_proxy_logic::handle_get_tui_tu_log_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(tui_tu_module, tui_tu_log); }
int db_proxy_logic::handle_tui_tu_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(tui_tu_log); }

int db_proxy_logic::handle_get_scp_log_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(scp_module, scp_log); }
int db_proxy_logic::handle_scp_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(scp_log); }

int db_proxy_logic::handle_mail_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(mail); }
int db_proxy_logic::handle_get_mail_list(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(mail_module, mail_list); }
int db_proxy_logic::handle_get_mail_info(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(mail_module, mail_detail); }
int db_proxy_logic::handle_check_new_mail(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(mail_module, check_new_mail); }
int db_proxy_logic::handle_get_mail_attach(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(mail_module, mail_attach); }
int db_proxy_logic::handle_get_system_mail_list(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(mail_module, system_mail_list); }
// guild
int db_proxy_logic::handle_get_guild_skill_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(guild_module, guild_skill_list); }
int db_proxy_logic::handle_guild_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(guild); }
// vip
int db_proxy_logic::handle_get_vip_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(vip_module, vip); }
int db_proxy_logic::handle_vip_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(vip); }
// cheng jiu
int db_proxy_logic::handle_get_cheng_jiu_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(cheng_jiu_module, cheng_jiu); }
int db_proxy_logic::handle_cheng_jiu_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(cheng_jiu); }
// title
int db_proxy_logic::handle_get_title_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(title_module, title); }
int db_proxy_logic::handle_title_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(title); }
// bao_shi
int db_proxy_logic::handle_get_bao_shi_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(bao_shi_module, bao_shi); }
int db_proxy_logic::handle_bao_shi_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(bao_shi); }
// huo yue du
int db_proxy_logic::handle_get_huo_yue_du_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(huo_yue_du_module, huo_yue_du); }
int db_proxy_logic::handle_huo_yue_du_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(huo_yue_du); }
// jing ji log
int db_proxy_logic::handle_get_jing_ji_log_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(jing_ji_module, jing_ji_log); }
int db_proxy_logic::handle_jing_ji_log_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(jing_ji_log); }
// lucky turn big award
int db_proxy_logic::handle_get_lucky_turn_big_award_list_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(lucky_turn_module, lucky_turn_big_award_list); }
// ltime recharge award
int db_proxy_logic::handle_get_ltime_recharge_award_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(ltime_act_module, ltime_recharge_award); }
int db_proxy_logic::handle_ltime_recharge_award_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(ltime_recharge_award); }

int db_proxy_logic::handle_get_lueduo_log_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(lue_duo_module, lueduo_log); }
int db_proxy_logic::handle_lueduo_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(lueduo_log); }

int db_proxy_logic::handle_get_dxc_info_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(dxc_module, dxc_info); }
int db_proxy_logic::handle_dxc_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(dxc_info); }
int db_proxy_logic::handle_get_kai_fu_act_log_result(const char *msg, const int len, const int res)
{ DB_HANDLE_RESULT_SHORT_CODE(kai_fu_act_obj, kai_fu_act_log); }
int db_proxy_logic::handle_kai_fu_act_log_err_result(const char *msg, const int len, const int res)
{ DB_ERROR_RESULT_SHORT_CODE(kai_fu_act_log); }
