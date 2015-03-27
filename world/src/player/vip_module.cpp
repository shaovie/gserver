#include "vip_module.h"
#include "player_obj.h"
#include "sys_log.h"
#include "error.h"
#include "vip_info.h"
#include "db_proxy.h"
#include "global_param_cfg.h"
#include "vip_config.h"
#include "package_module.h"
#include "behavior_id.h"
#include "title_module.h"
#include "zhan_xing_module.h"
#include "cheng_jiu_module.h"
#include "all_char_info.h"
#include "notice_module.h"
#include "clsid.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("vip");
static ilog_obj *e_log = err_log::instance()->get_ilog("vip");

char vip_module::vip_lvl(player_obj *player)
{
  if (player->vip_info_ == NULL) return 0;
  return player->vip_info_->vip_lvl_;
}
void vip_module::on_after_login(player_obj *player)
{ vip_module::do_notify_vip_2_client(player); }
void vip_module::on_char_lvl_up(player_obj *player)
{
  if (player->lvl() >= global_param_cfg::vip_auto_be_at_lvl
      && vip_module::vip_lvl(player) == 0)
    vip_module::do_be_vip(player);
}
void vip_module::on_new_day(player_obj *player, const bool when_login)
{
  if (player->vip_info_ == NULL) return ;
  player->vip_info_->guild_jb_cnt_ = 0;
  player->vip_info_->buy_ti_li_cnt_ = 0;
  player->vip_info_->exchange_coin_cnt_ = 0;
  player->vip_info_->free_relive_cnt_ = 0;
  vip_module::do_update_vip_2_db(player);
  if (!when_login)
    vip_module::do_notify_vip_2_client(player);
}
void vip_module::on_lose_money(player_obj *player,
                               const char money_type,
                               const int behavior_sub_type,
                               const int cost)
{
  if (money_type != M_DIAMOND
      || behavior_sub_type == MONEY_LOSE_MAIL_ATTACH
      || behavior_sub_type == MONEY_LOSE_TRADE
      || behavior_sub_type == MONEY_LOSE_MARKET_BUY
      || behavior_sub_type == MONEY_LOSE_MARKET_SALE
      || vip_module::vip_lvl(player) < global_param_cfg::vip_buy_add_exp_min_vip_lvl)
    return ;
  vip_module::do_add_vip_exp(player, cost);
}
void vip_module::on_recharge_ok(player_obj *player, const int val)
{
  if (vip_module::vip_lvl(player) == 0)
    vip_module::do_be_vip(player);
  vip_module::do_add_vip_exp(player, val);
}
void vip_module::on_be_vip(player_obj *player)
{
  player->broadcast_vip_info();
  zhan_xing_module::on_vip_lvl_up(player);
  cheng_jiu_module::on_vip_level(player, vip_module::vip_lvl(player));
  vip_module::do_award_title(player, 1);
  char_brief_info::on_char_vip_up(player->id(), player->vip_info_->vip_lvl_);
}
void vip_module::on_vip_lvl_up(player_obj *player)
{
  player->broadcast_vip_info();
  zhan_xing_module::on_vip_lvl_up(player);
  cheng_jiu_module::on_vip_level(player, vip_module::vip_lvl(player));
  vip_module::do_award_title(player, vip_module::vip_lvl(player));
  char_brief_info::on_char_vip_up(player->id(), player->vip_info_->vip_lvl_);
}
int vip_module::can_buy_item(player_obj *player, const int npc_cid)
{
  if (!vip_config::instance()->find_trade_npc(npc_cid))
    return 0;
  if (vip_module::vip_lvl(player) == 0)
    return ERR_NOT_VIP;
  if (vip_config::instance()->trade_npc_cid(player->vip_info_->vip_lvl_) != npc_cid)
    return ERR_CAN_NOT_BUY_ITEM;
  return 0;
}
bool vip_module::do_cost_effect(player_obj *player, const char vip_effect_id, const int val)
{
  if (player->vip_info_ == NULL || val <= 0)
    return false;
  switch (vip_effect_id)
  {
  case VIP_EFF_GUILD_JU_BAO_CNT:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      if (player->vip_info_->guild_jb_cnt_ + val > max_cnt) return false;
      player->vip_info_->guild_jb_cnt_ += val;
      break;
    }
  case VIP_EFF_BUY_TI_LI:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      if (player->vip_info_->buy_ti_li_cnt_ + val > max_cnt) return false;
      player->vip_info_->buy_ti_li_cnt_ += val;
      break;
    }
  case VIP_EFF_EXCHANGE_COIN:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      if (player->vip_info_->exchange_coin_cnt_ + val > max_cnt) return false;
      player->vip_info_->exchange_coin_cnt_ += val;
      break;
    }
  case VIP_EFF_FREE_RELIVE:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      if (player->vip_info_->free_relive_cnt_ + val > max_cnt) return false;
      player->vip_info_->free_relive_cnt_ += val;
      break;
    }
  default:
    return false;
  }
  vip_module::do_update_vip_2_db(player);
  vip_module::do_notify_vip_2_client(player);
  return true;
}
int vip_module::to_get_effect_left(player_obj *player, const char vip_effect_id)
{
  if (player->vip_info_ == NULL) return 0;
  switch (vip_effect_id)
  {
  case VIP_EFF_GUILD_JU_BAO_CNT:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      return max_cnt - player->vip_info_->guild_jb_cnt_;
    }
  case VIP_EFF_BUY_TI_LI:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      return max_cnt - player->vip_info_->buy_ti_li_cnt_;
    }
  case VIP_EFF_EXCHANGE_COIN:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      return max_cnt - player->vip_info_->exchange_coin_cnt_;
    }
  case VIP_EFF_FREE_RELIVE:
    {
      const int max_cnt = vip_config::instance()->effect_val1(vip_module::vip_lvl(player), vip_effect_id);
      return max_cnt - player->vip_info_->free_relive_cnt_;
    }
  }
  return 0;
}
void player_obj::broadcast_vip_info()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << vip_module::vip_lvl(this);
  ph->set(0, NTF_BROADCAST_VIP_INFO, 0, mb.length());
  this->do_broadcast(&mb, false);
}
void vip_module::do_notify_vip_2_client(player_obj *player)
{
  if (player->vip_info_ == NULL) return ;
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->vip_info_->vip_lvl_
    << player->vip_info_->vip_exp_;
  vip_module::do_fetch_vip_lvl_award(player, os);
  os << player->vip_info_->guild_jb_cnt_
    << player->vip_info_->buy_ti_li_cnt_
    << player->vip_info_->exchange_coin_cnt_
    << player->vip_info_->free_relive_cnt_;
  player->send_request(NTF_VIP_INFO, &os);
}
void vip_module::do_fetch_vip_lvl_award(player_obj *player, out_stream &os)
{
  char *amt = (char *)os.wr_ptr();
  os << (char)0;

  for (int i = 0;
       i < MAX_VIP_LVL && player->vip_info_->get_idxs_[i] != '\0';
       ++i)
  {
    if (player->vip_info_->get_idxs_[i] == '1')
    {
      os << (char)(i + 1);
      ++(*amt);
    }
  }
}
void vip_module::do_insert_vip_2_db(player_obj *player)
{
  if (player->vip_info_ == NULL) return ;
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->vip_info_;
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_VIP, &os);
}
void vip_module::do_update_vip_2_db(player_obj *player)
{
  if (player->vip_info_ == NULL) return ;
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->vip_info_;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_VIP, &os);
}
void vip_module::do_add_vip_exp(player_obj *player, const int exp)
{
  if (player->vip_info_ == NULL
      || exp <= 0)
    return ;

  int left_exp = exp;
  const int max_lvl = vip_config::instance()->max_vip_lvl();
  for (;;)
  {
    if (player->vip_info_->vip_lvl_ >= max_lvl)
      break ;

    int64_t sum_exp = player->vip_info_->vip_exp_ + left_exp;
    const int need_exp = vip_config::instance()->lvl_up_exp(player->vip_info_->vip_lvl_);
    if (sum_exp >= need_exp)
    {
      left_exp -= (need_exp - player->vip_info_->vip_exp_);
      player->vip_info_->vip_exp_ = 0;
      ++player->vip_info_->vip_lvl_;
      vip_module::on_vip_lvl_up(player);
    }else
    {
      player->vip_info_->vip_exp_ = sum_exp;
      break;
    }
  }

  vip_module::do_update_vip_2_db(player);
  vip_module::do_notify_vip_2_client(player);
}
void vip_module::do_be_vip(player_obj *player)
{
  player->vip_info_ = new vip_info();
  player->vip_info_->vip_lvl_ = 1;
  player->vip_info_->char_id_ = player->id();
  vip_module::do_insert_vip_2_db(player);
  vip_module::do_notify_vip_2_client(player);
  vip_module::on_be_vip(player);
}
int vip_module::handle_db_get_vip_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt <= 0) return 0;

  char info_bf[sizeof(vip_info) + 4] = {0};
  stream_istr info_si(info_bf, sizeof(info_bf));
  is >> info_si;
  in_stream info_is(info_bf, info_si.str_len());

  if (player->vip_info_ == NULL)
    player->vip_info_ = new vip_info();
  info_is >> player->vip_info_;
  return 0;
}
void vip_module::destroy(player_obj *player)
{
  if (player->vip_info_ != NULL)
    delete player->vip_info_;
}
void vip_module::do_award_title(player_obj *player, const char vip_lvl)
{
  if (player->vip_info_ == NULL) return ;
  const int title = vip_config::instance()->award_title(vip_lvl);
  if (title <= 0) return ;
  title_module::add_new_title(player, title);
}

int vip_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = vip_module::FUNC(player, msg, len);       \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_VIP_GET_LVL_AWARD, clt_get_lvl_award);
    SHORT_CODE(REQ_VIP_EXCHANGE_COIN, clt_exchange_coin);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int vip_module::clt_get_lvl_award(player_obj *player, const char *msg, const int len)
{
  char vip_lvl = 0;
  in_stream is(msg, len);
  is >> vip_lvl;

  if (player->vip_info_ == NULL
      || player->vip_info_->vip_lvl_ < vip_lvl
      || vip_lvl <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_VIP_LVL_LACK);

  if (vip_module::if_vip_lvl_award_get(player, vip_lvl))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_HAD_GET);

  ilist<item_amount_bind_t> *al = vip_config::instance()->lvl_award_list(vip_lvl);
  if (al == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  const int ret = package_module::do_insert_award_item_list(player,
                                                            al,
                                                            blog_t(ITEM_GOT_VIP_LVL_AWARD, 0, 0));
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  // notice
  for (ilist_node<item_amount_bind_t> *itor = al->head();
       itor != NULL;
       itor = itor->next_)
  {
    item_amount_bind_t &io = itor->value_;
    if (clsid::is_char_equip(io.cid_))
    {
      item_obj notice_item;
      notice_item.cid_ = clsid::get_equip_cid_by_career(player->career(), io.cid_);
      notice_item.amount_ = io.amount_;
      notice_item.bind_ = io.bind_;
      notice_module::got_vip_equip_award(player->id(), player->name(), vip_lvl, &notice_item);
    }
  }

  vip_module::do_set_vip_lvl_award_get(player, vip_lvl);
  vip_module::do_update_vip_2_db(player);
  vip_module::do_notify_vip_2_client(player);
  return 0;
}
bool vip_module::if_vip_lvl_award_get(player_obj *player, const char vip_lvl)
{
  if (player->vip_info_ == NULL
      || vip_lvl <= 0 || vip_lvl > MAX_VIP_LVL)
    return true;
  return player->vip_info_->get_idxs_[vip_lvl - 1] == '1' ? true : false;
}
void vip_module::do_set_vip_lvl_award_get(player_obj *player, const char vip_lvl)
{
  if (player->vip_info_ == NULL
      || vip_lvl <= 0 || vip_lvl > MAX_VIP_LVL)
    return ;

  player->vip_info_->get_idxs_[vip_lvl - 1] = '1';
  for (int i = vip_lvl - 2;
       i >= 0 && player->vip_info_->get_idxs_[i] == '\0';
       --i)
    player->vip_info_->get_idxs_[i] = '0';
}
int vip_module::clt_exchange_coin(player_obj *player, const char *, const int )
{
  if (vip_module::vip_lvl(player) == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_VIP);

  if (vip_module::to_get_left_exchange_coin_cnt(player) <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int buy_amt = 0;
  int cost = 0;
  if (vip_purchase_cfg::instance()->coin(vip_module::to_get_had_exchange_coin_cnt(player) + 1,
                                          cost,
                                          buy_amt) != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  const int ret = player->is_money_enough(M_DIAMOND, cost);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  if (player->is_money_upper_limit(M_COIN, buy_amt))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);

  player->do_lose_money(cost,
                      M_DIAMOND,
                      MONEY_LOSE_VIP_EXCHANGE_COIN,
                      0, 0, 0);

  vip_module::do_cost_exchange_coin(player);
  player->do_got_money(buy_amt, M_COIN, MONEY_GOT_VIP_EXCHANGE_COIN, 0);
  return 0;
}
void vip_module::do_cost_guild_ju_bao_cnt(player_obj *player)
{ vip_module::do_cost_effect(player, VIP_EFF_GUILD_JU_BAO_CNT); }
int vip_module::to_get_left_guild_ju_bao_cnt(player_obj *player)
{ return vip_module::to_get_effect_left(player, VIP_EFF_GUILD_JU_BAO_CNT); }
int vip_module::to_get_passive_skill_add_rate(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_PASSIVE_SKILL_UP_RATE); }
int vip_module::to_get_tili_add_upper_limit(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_TI_LI_UPPER_LIMIT); }
int vip_module::to_get_tui_tu_add_exp(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_TUI_TU_PASS_EXP); }
int vip_module::to_get_tui_tu_free_turn_cnt(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_TUI_TU_FREE_TURN_CNT); }
int vip_module::to_get_equip_strengthen_add_rate(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_EQUIP_QIANG_HUA_RATE); }
int vip_module::to_get_add_zhan_xing_value(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_ZHAN_XING_CNT); }
int vip_module::to_get_add_friend_amt(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_FRIEND_CNT); }
int vip_module::to_get_day_jing_ji_cnt(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_DAY_JING_JI_CNT); }
int vip_module::to_get_bao_shi_exp_per(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_BAO_SHI_EXP_PER); }
int vip_module::to_get_add_worship_cnt(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_WORSHIP_CNT); }
int vip_module::to_get_left_buy_ti_li_cnt(player_obj *player)
{ return vip_module::to_get_effect_left(player, VIP_EFF_BUY_TI_LI); }
int vip_module::to_get_had_buy_ti_li_cnt(player_obj *player)
{ return player->vip_info_->buy_ti_li_cnt_; }
void vip_module::do_cost_buy_ti_li_cnt(player_obj *player)
{ vip_module::do_cost_effect(player, VIP_EFF_BUY_TI_LI); }
int vip_module::to_get_left_exchange_coin_cnt(player_obj *player)
{ return vip_module::to_get_effect_left(player, VIP_EFF_EXCHANGE_COIN); }
int vip_module::to_get_had_exchange_coin_cnt(player_obj *player)
{ return player->vip_info_->exchange_coin_cnt_; }
int vip_module::to_get_jingli_add_upper_limit(player_obj *player)
{ return vip_config::instance()->effect_val1(vip_module::vip_lvl(player), VIP_EFF_JING_LI_UPPER_LIMIT); }
void vip_module::do_cost_exchange_coin(player_obj *player)
{ vip_module::do_cost_effect(player, VIP_EFF_EXCHANGE_COIN); }
int vip_module::to_get_left_free_relive_cnt(player_obj *player)
{ return vip_module::to_get_effect_left(player, VIP_EFF_FREE_RELIVE); }
int vip_module::to_get_had_free_relive_cnt(player_obj *player)
{ return player->vip_info_->free_relive_cnt_; }
void vip_module::do_cost_free_relive(player_obj *player)
{ vip_module::do_cost_effect(player, VIP_EFF_FREE_RELIVE); }
