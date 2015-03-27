#include "activity_module.h"
#include "activity_cfg.h"
#include "activity_mgr.h"
#include "ghz_module.h"
#include "player_obj.h"
#include "message.h"
#include "sys_log.h"
#include "ghz_activity_obj.h"
#include "global_param_cfg.h"
#include "award_ti_li_act.h"
#include "month_card_award_act.h"
#include "xszc_activity_obj.h"
#include "goods_award_act.h"
#include "kai_fu_act_obj.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("act");
static ilog_obj *e_log = err_log::instance()->get_ilog("act");

int activity_module::init()
{
  ghz_activity_obj::parse_shou_wei_idx_cfg(global_param_cfg::ghz_shou_wei_idx);
  return activity_mgr::instance()->init();
}
void activity_module::destroy(player_obj *player)
{
  kai_fu_act_obj::destroy(player);
}
activity_obj *activity_module::construct(const int act_id)
{
  activity_obj *ao = NULL;
  if (act_id == ACTIVITY_GHZ)
    ao = new ghz_activity_obj();
  else if (act_id == ACTIVITY_AWARD_TI_LI)
    ao = new award_ti_li_act();
  else if (act_id == ACTIVITY_XSZC)
    ao = new xszc_activity_obj();
  else if (act_id == ACTIVITY_GOODS_AWARD)
    ao = new goods_award_act();
  else if (act_id == ACTIVITY_KAI_FU)
    ao = new kai_fu_act_obj();
  return ao;
}
void activity_module::on_new_day(player_obj *player, const bool when_login)
{
  month_card_award_act::on_new_day(player, when_login);
}
void activity_module::on_char_login(player_obj *player)
{
  ghz_module::on_char_login(player);
}
void activity_module::on_enter_game(player_obj *player)
{
  ghz_module::on_enter_game(player);
  award_ti_li_act::on_enter_game(player);
  month_card_award_act::on_enter_game(player);
  xszc_activity_obj::on_enter_game(player);
  goods_award_act::on_enter_game(player);
  kai_fu_act_obj::on_enter_game(player);
}
void activity_module::on_char_lvl_up(player_obj *player)
{
  ghz_module::on_char_lvl_up(player);
  award_ti_li_act::on_char_lvl_up(player);
  xszc_activity_obj::on_char_lvl_up(player);
  goods_award_act::on_char_lvl_up(player);
  kai_fu_act_obj::on_char_lvl_up(player);
}
void activity_module::on_transfer_scene(player_obj *player,
                                        const int /*from_scene_id*/,
                                        const int /*from_scene_cid*/,
                                        const int /*to_scene_id*/,
                                        const int to_scene_cid)
{
  ghz_module::on_transfer_scene(player, to_scene_cid);
}
int activity_module::can_transfer_to(player_obj *player, const int to_scene_cid)
{
  int ret = ghz_module::can_transfer_to(player, to_scene_cid);
  if (ret != 0) return ret;
  return 0;
}
int activity_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = activity_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GOT_TI_LI_AWARD,          clt_got_ti_li_award);
    SHORT_CODE(REQ_GOT_MONTH_CARD_AWARD,     clt_got_month_card_award);
    SHORT_CODE(REQ_ENTER_XSZC,               clt_enter_xszc);
    SHORT_CODE(REQ_OBTAIN_XSZC_ACT_INFO,     clt_obtain_xszc_act_info);
    SHORT_CODE(REQ_OBTAIN_XSZC_GROUP_POS,    clt_obtain_xszc_group_pos);
    SHORT_CODE(REQ_GET_DAILY_LUCKY_GOODS_DRAW_CNT, clt_get_daily_lucky_goods_draw_cnt);
    SHORT_CODE(REQ_DO_DAILY_LUCKY_GOODS_DRAW, clt_do_lucky_goods_draw);
    SHORT_CODE(REQ_OBTAIN_KAI_FU_ACT_RANK,   clt_obtain_kai_fu_act_rank);
    SHORT_CODE(REQ_OBTAIN_KAI_FU_ACT_STATE,  clt_obtain_kai_fu_act_state);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int activity_module::clt_got_ti_li_award(player_obj *player, const char *, const int)
{
  int ret = award_ti_li_act::do_got_ti_li_award(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_got_month_card_award(player_obj *player, const char *, const int)
{
  int ret = month_card_award_act::do_got_month_card_award(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_enter_xszc(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int scene_id = 0;
  is >> scene_id;

  int ret = xszc_activity_obj::do_enter_xszc(player, scene_id);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_obtain_xszc_act_info(player_obj *player, const char *, const int )
{
  int ret = xszc_activity_obj::do_obtain_xszc_act_info(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_obtain_xszc_group_pos(player_obj *player, const char *, const int )
{
  int ret = xszc_activity_obj::do_obtain_xszc_group_pos(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_get_daily_lucky_goods_draw_cnt(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->daily_goods_lucky_draw_cnt();
  player->send_respond_ok(RES_GET_DAILY_LUCKY_GOODS_DRAW_CNT, &os);
  return 0;
}
int activity_module::clt_do_lucky_goods_draw(player_obj *player, const char *, const int )
{
  int ret = goods_award_act::do_lucky_goods_draw(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int activity_module::clt_obtain_kai_fu_act_rank(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int act_type = 0;
  is >> act_type;
  kai_fu_act_obj::do_obtain_kai_fu_act_rank(player, act_type);
  return 0;
}
int activity_module::clt_obtain_kai_fu_act_state(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int act_type = 0;
  is >> act_type;
  kai_fu_act_obj::do_obtain_kai_fu_act_state(player, act_type);
  return 0;
}
