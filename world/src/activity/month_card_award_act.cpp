#include "month_card_award_act.h"
#include "recharge_module.h"
#include "activity_mgr.h"
#include "activity_cfg.h"
#include "player_obj.h"
#include "time_util.h"
#include "recharge_config.h"
#include "behavior_id.h"

// Lib header

int month_card_award_act::daily_award = 0;

int month_card_award_act::parse_cfg_param(const char *param)
{
  if (::sscanf(param, "%d", &month_card_award_act::daily_award) != 1)
    return -1;
  return 0;
}
void month_card_award_act::on_new_day(player_obj *player, const bool when_login)
{
  if (when_login) return ;

  int diff_day = time_util::diff_days(player->last_buy_mc_time());
  if (diff_day > MONTHLY_CARD_REBATE_DAYS)
    return ;
  else if (diff_day == MONTHLY_CARD_REBATE_DAYS - 1)
    recharge_module::do_send_mc_disabled_mail(player);
  else if (diff_day == MONTHLY_CARD_REBATE_DAYS)
  {
    mblock mb(client::send_buf, client::send_buf_len);
    mb.wr_ptr(sizeof(proto_head));
    mb << (char)1;
    player->do_delivery(NTF_CAN_BUY_MONTH_CARD, &mb);
  }

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << (int)ACTIVITY_MONTH_CARD_AWARD << 0;
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void month_card_award_act::on_enter_game(player_obj *player)
{
  if (player->last_buy_mc_time() != 0
      && time_util::diff_days(player->last_buy_mc_time())
         >= MONTHLY_CARD_REBATE_DAYS - 1
      && time_util::diff_days(player->out_time(), player->last_buy_mc_time())
         < MONTHLY_CARD_REBATE_DAYS - 1)
    recharge_module::do_send_mc_disabled_mail(player);

  if (recharge_module::can_buy_mc(player, time_util::now))
    return;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << (char)0;
  player->do_delivery(NTF_CAN_BUY_MONTH_CARD, &mb);

  if (player->if_mc_rebate() == 0)
  {
    mblock mb(client::send_buf, client::send_buf_len);
    mb.wr_ptr(sizeof(proto_head));
    mb << (int)ACTIVITY_MONTH_CARD_AWARD << 0;
    player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
  }
}
void month_card_award_act::on_buy_month_card_ok(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << (char)0;
  player->do_delivery(NTF_CAN_BUY_MONTH_CARD, &mb);

  mb.reset();
  mb.wr_ptr(sizeof(proto_head));
  mb << (int)ACTIVITY_MONTH_CARD_AWARD << 0;
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
int month_card_award_act::do_got_month_card_award(player_obj *player)
{
  if (recharge_module::can_buy_mc(player, time_util::now))
    return ERR_DONT_HAVE_MC;
  if (player->if_mc_rebate() != 0) return ERR_HAVE_GOT_TODAYS;

  recharge_cfg_obj *rco =
    recharge_config::instance()->get_recharge_obj(RECHARGE_MONTHLY_CARD);
  if (rco == NULL) return 0;

  if (player->is_money_upper_limit(M_BIND_DIAMOND, rco->b_diamond_1_))
    return ERR_MONEY_UPPER_LIMIT;

  player->do_got_money(rco->b_diamond_1_,
                       M_BIND_DIAMOND,
                       MONEY_GOT_MC_REBATE,
                       player->id());
  player->if_mc_rebate(1);
  player->db_save_daily_clean_info();
  
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << (int)ACTIVITY_MONTH_CARD_AWARD;
  player->do_delivery(NTF_ACTIVITY_CLOSED, &mb);
  return 0;
}
