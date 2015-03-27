#include "award_ti_li_act.h"
#include "global_param_cfg.h"
#include "behavior_log.h"
#include "activity_mgr.h"
#include "activity_cfg.h"
#include "player_obj.h"
#include "vip_module.h"
#include "time_util.h"

// Lib header

int award_ti_li_act::award_ti_li = 0;

award_ti_li_act::award_ti_li_act() :
  activity_obj(ACTIVITY_AWARD_TI_LI)
{ }
int award_ti_li_act::parse_cfg_param(const char *param)
{
  if (::sscanf(param, "%d", &award_ti_li_act::award_ti_li) != 1)
    return -1;
  return 0;
}
void award_ti_li_act::on_enter_game(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_AWARD_TI_LI);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() < activity_cfg::instance()->open_lvl(ACTIVITY_AWARD_TI_LI)
      || player->got_ti_li_award_time() > ao->begin_time())
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void award_ti_li_act::on_char_lvl_up(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_AWARD_TI_LI);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() != activity_cfg::instance()->open_lvl(ACTIVITY_AWARD_TI_LI))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
int award_ti_li_act::do_got_ti_li_award(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_AWARD_TI_LI);
  if (ao == NULL
      || !ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;
  if (player->lvl() < activity_cfg::instance()->open_lvl(ACTIVITY_AWARD_TI_LI))
    return ERR_LVL_NOT_ENOUGH;
  if (player->got_ti_li_award_time() > ao->begin_time())
    return ERR_HAD_GOT_AWARD;

  player->do_add_ti_li(award_ti_li_act::award_ti_li, true, true);
  player->got_ti_li_award_time(time_util::now);
  player->db_save_char_extra_info();

  behavior_log::instance()->store(BL_JOIN_ACTVITY,
                                  time_util::now,
                                  "%d|%d",
                                  player->id(), ACTIVITY_AWARD_TI_LI);

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id();
  player->do_delivery(NTF_ACTIVITY_CLOSED, &mb);
  return 0;
}
