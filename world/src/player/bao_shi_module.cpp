#include "bao_shi_module.h"
#include "istream.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "client.h"
#include "player_obj.h"
#include "behavior_id.h"
#include "clsid.h"
#include "def.h"
#include "item_obj.h"
#include "package_module.h"
#include "huo_yue_du_module.h"
#include "cheng_jiu_module.h"
#include "bao_shi_info.h"
#include "db_proxy.h"
#include "ilist.h"
#include "bao_shi_cfg.h"
#include "global_param_cfg.h"
#include "vip_module.h"
#include "player_mgr.h"
#include "notice_module.h"
#include "lucky_egg_module.h"
#include "kai_fu_act_obj.h"

#define BAO_SHI_VIP_CFG_DIVISOR 10000
#define BAO_SHI_LUCK_CFG_DIVISOR 10000

enum yijianshengji_result
{
  YJSJ_RES_SUCCESS       = 1,
  YJSJ_RES_LACK_CAI_LIAO = 2,
  YJSJ_RES_LACK_COIN     = 3,
};

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("bao_shi");
static ilog_obj *e_log = err_log::instance()->get_ilog("bao_shi");

void bao_shi_module::do_update_bao_shi_2_db(player_obj *player, bao_shi_info *info)
{
  stream_ostr so((const char *)info, sizeof(bao_shi_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << so;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_BAO_SHI, &os);
}
void bao_shi_module::do_insert_bao_shi_2_db(player_obj *player, bao_shi_info *info)
{
  stream_ostr so((const char *)info, sizeof(bao_shi_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << so;
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_BAO_SHI, &os);
}
void bao_shi_module::do_notify_bao_shi_2_clt(player_obj *player, bao_shi_info *info)
{
  out_stream os(client::send_buf, client::send_buf_len);
  bao_shi_module::do_fetch_bao_shi(info, os);
  player->send_request(NTF_BAO_SHI_INFO, &os);
}
void bao_shi_module::do_fetch_bao_shi(bao_shi_info *info, out_stream &os)
{
  os << info->pos_
    << info->bs_idx_
    << info->bs_lvl_
    << info->bs_exp_;
}
void bao_shi_module::destroy(player_obj *player)
{
  while (!player->bao_shi_list_.empty())
    delete player->bao_shi_list_.pop_front();
}
int bao_shi_module::handle_db_get_bao_shi_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt < 0) return 0;

  bao_shi_module::destroy(player);
  for (int i = 0; i < res_cnt; ++i)
  {
    char bf[sizeof(bao_shi_info) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;

    bao_shi_info *info = new bao_shi_info();
    ::memcpy(info, bf, sizeof(bao_shi_info));
    player->bao_shi_list_.push_back(info);
  }
  return 0;
}
int bao_shi_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = bao_shi_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_BAO_SHI,          clt_obtain_bao_shi);
    SHORT_CODE(REQ_ACTIVATE_BAO_SHI,        clt_activate_bao_shi);
    SHORT_CODE(REQ_UPGRADE_BAO_SHI,         clt_upgrade_bao_shi);
    SHORT_CODE(REQ_UPGRADE_BAO_SHI_TO_NEXT, clt_upgrade_bao_shi_to_next);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int bao_shi_module::clt_obtain_bao_shi(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  char *amt = (char *)os.wr_ptr();
  os << (char)0;
  for (ilist_node<bao_shi_info *> *itor = player->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    bao_shi_info *info = itor->value_;
    if (info->bs_lvl_ <= 0) continue;
    bao_shi_module::do_fetch_bao_shi(info, os);
    ++(*amt);
  }
  return player->send_request(RES_OBTAIN_BAO_SHI, &os);
}
int bao_shi_module::clt_activate_bao_shi(player_obj *player, const char *msg, const int len)
{
  char pos = 0, bs_idx = 0, auto_buy = 0;
  in_stream is(msg, len);
  is >> pos >> bs_idx >> auto_buy;

  if (pos <= PART_XXX || pos >= PART_END
      || bs_idx <= 0 || bs_idx > global_param_cfg::bao_shi_pos_hole_amt)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  bao_shi_info *bsi = bao_shi_module::find_bao_shi(player, pos, bs_idx);
  if (bsi != NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_ACTIVATE_HAD);

  if (bao_shi_module::last_row_min_lvl(player, bs_idx) <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_ACTIVATE_LAST_ROW);

  // auto_buy cost diamond and this cost coin so no together cal
  const int cost_money = global_param_cfg::bao_shi_activate_cost;
  int ret = player->is_money_enough(M_COIN, cost_money);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  const int cost_cid = global_param_cfg::bao_shi_jing_hua_cid;
  const int cost_amt = global_param_cfg::bao_shi_activate_amt;
  ret = player->do_check_item_enough_by_auto_buy(cost_cid, cost_amt, auto_buy);
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  const int need_buy_amt = ret;
  if (need_buy_amt < cost_amt)
    package_module::do_remove_item(player,
                                   cost_cid,
                                   cost_amt - need_buy_amt,
                                   ITEM_LOSE_BAO_SHI_ACTIVATE,
                                   0);

  if (auto_buy != 0 && need_buy_amt > 0)
    player->do_auto_buy_item(cost_cid, need_buy_amt);

  player->do_lose_money(cost_money,
                        M_COIN,
                        MONEY_LOSE_BAO_SHI_ACTIVATE,
                        0, 0, 0);

  bao_shi_info *info = new bao_shi_info();
  player->bao_shi_list_.push_back(info);
  info->char_id_ = player->id();
  info->pos_ = pos;
  info->bs_idx_ = bs_idx;
  info->bs_lvl_ = 1;
  player->do_calc_attr_affected_by_bao_shi();
  player->on_attr_update(ZHAN_LI_EQUIP_BAO_SHI, pos);
  bao_shi_module::do_insert_bao_shi_2_db(player, info);
  bao_shi_module::do_notify_bao_shi_2_clt(player, info);

  cheng_jiu_module::on_bao_shi_lvl(player, pos, info->bs_lvl_);
  kai_fu_act_obj::on_all_bao_shi_lvl_update(player,
                                            bao_shi_module::get_bao_shi_all_lvl(player));
  return player->send_respond_ok(RES_ACTIVATE_BAO_SHI);
}
int bao_shi_module::clt_upgrade_bao_shi(player_obj *player, const char *msg, const int len)
{
  char pos = 0, bs_idx = 0, auto_buy = 0;
  in_stream is(msg, len);
  is >> pos >> bs_idx >> auto_buy;

  if (pos <= PART_XXX || pos >= PART_END
      || bs_idx <= 0 || bs_idx > global_param_cfg::bao_shi_pos_hole_amt)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  bao_shi_info *info = bao_shi_module::find_bao_shi(player, pos, bs_idx);
  if (info == NULL || info->bs_lvl_ <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_ACTIVATE_NO);

  const int target_lvl = info->bs_lvl_ + 1;
  if (bao_shi_module::last_row_min_lvl(player, bs_idx) < target_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_UPGRADE_LAST_ROW);

  if (target_lvl > player->lvl()
      || !bao_shi_lvl_cfg::instance()->is_vaild_lvl(target_lvl))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_REACH_MAX_LVL);

  const int old_lvl = info->bs_lvl_;
  int up_cnt = 1;
  int cost_item = 0;
  char result = 0;
  const int ret = bao_shi_module::do_cnt_upgrade_bao_shi_and_up_stop(player,
                                                                     info,
                                                                     auto_buy,
                                                                     up_cnt,
                                                                     cost_item,
                                                                     result);
  if (up_cnt < 1)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  if (info->bs_lvl_ != old_lvl)
  {
    player->do_calc_attr_affected_by_bao_shi();
    player->on_attr_update(ZHAN_LI_EQUIP_BAO_SHI, pos);
  }

  bao_shi_module::do_update_bao_shi_2_db(player, info);
  bao_shi_module::do_notify_bao_shi_2_clt(player, info);
  huo_yue_du_module::on_upgrade_bao_shi(player);
  return player->send_respond_ok(RES_UPGRADE_BAO_SHI);
}
int bao_shi_module::clt_upgrade_bao_shi_to_next(player_obj *player, const char *msg, const int len)
{
  char pos = 0, bs_idx = 0, auto_buy = 0;
  in_stream is(msg, len);
  is >> pos >> bs_idx >> auto_buy;

  if (pos <= PART_XXX || pos >= PART_END
      || bs_idx <= 0 || bs_idx > global_param_cfg::bao_shi_pos_hole_amt)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  bao_shi_info *info = bao_shi_module::find_bao_shi(player, pos, bs_idx);
  if (info == NULL || info->bs_lvl_ <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_ACTIVATE_NO);

  const int target_lvl = info->bs_lvl_ + 1;
  if (bao_shi_module::last_row_min_lvl(player, bs_idx) < target_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_UPGRADE_LAST_ROW);

  if (target_lvl > player->lvl()
      || !bao_shi_lvl_cfg::instance()->is_vaild_lvl(target_lvl))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_BAO_SHI_REACH_MAX_LVL);

  const int old_lvl = info->bs_lvl_;
  const int old_coin = player->coin();
  int up_cnt = MAX_CIRCLE_CNT;
  int cost_item = 0;
  char result = 0;
  bao_shi_module::do_cnt_upgrade_bao_shi_and_up_stop(player,
                                                     info,
                                                     auto_buy,
                                                     up_cnt,
                                                     cost_item,
                                                     result);
  if (info->bs_lvl_ != old_lvl)
  {
    player->do_calc_attr_affected_by_bao_shi();
    player->on_attr_update(ZHAN_LI_EQUIP_BAO_SHI, pos);
  }

  bao_shi_module::do_update_bao_shi_2_db(player, info);
  bao_shi_module::do_notify_bao_shi_2_clt(player, info);
  huo_yue_du_module::on_upgrade_bao_shi(player);

  const int cost_coin = old_coin - player->coin();
  out_stream os(client::send_buf, client::send_buf_len);
  os << result << cost_coin << cost_item;
  return player->send_respond_ok(RES_UPGRADE_BAO_SHI_TO_NEXT, &os);
}
int bao_shi_module::do_cnt_upgrade_bao_shi_and_up_stop(player_obj *player,
                                                       bao_shi_info *info,
                                                       const char auto_buy,
                                                       int &up_cnt,
                                                       int &cost_item,
                                                       char &result)
{
  const int old_lvl = info->bs_lvl_;
  const int target_lvl = info->bs_lvl_ + 1;
  const int cost_cid = bao_shi_lvl_cfg::instance()->lvl_cost_cid(target_lvl);
  const int cost_amt = bao_shi_lvl_cfg::instance()->lvl_cost_amt(target_lvl);
  const int cost_money = bao_shi_lvl_cfg::instance()->lvl_cost_money(target_lvl);
  const int vip_exp_per = vip_module::to_get_bao_shi_exp_per(player);

  int ret = 0;
  int total_cost_money = 0;
  int total_cost_amt = 0;
  int total_buy_amt = 0;
  int total_up_cnt = 0;
  for (; total_up_cnt < up_cnt; ++total_up_cnt)
  {
    if (info->bs_lvl_ > old_lvl)
    {
      result = YJSJ_RES_SUCCESS;
      break;
    }

    // auto_buy cost diamond and this cost coin so no together cal
    ret = player->is_money_enough(M_COIN, cost_money + total_cost_money);
    if (ret != 0)
    {
      result = YJSJ_RES_LACK_COIN;
      break;
    }
    ret = player->do_check_item_enough_by_auto_buy(cost_cid,
                                                   cost_amt + total_cost_amt,
                                                   auto_buy);
    if (ret < 0)
    {
      result = YJSJ_RES_LACK_CAI_LIAO;
      break;
    }

    total_cost_money += cost_money;
    total_cost_amt += cost_amt;
    total_buy_amt = ret;

    if (!bao_shi_module::do_luck_lvl_up(player, info))
    {
      int add_exp = bao_shi_lvl_cfg::instance()->lvl_rand_add_exp(target_lvl);
      add_exp += (int64_t)add_exp * vip_exp_per / BAO_SHI_VIP_CFG_DIVISOR;
      bao_shi_module::do_add_bs_exp(player, info, add_exp);
    }
  }
  up_cnt = total_up_cnt; // return

  cost_item = total_cost_amt;
  if (cost_item > 0)
    package_module::do_remove_item(player,
                                   cost_cid,
                                   total_cost_amt - total_buy_amt,
                                   ITEM_LOSE_BAO_SHI_UPGRADE,
                                   0);

  if (total_buy_amt > 0)
    player->do_auto_buy_item(cost_cid, total_buy_amt);

  player->do_lose_money(total_cost_money,
                        M_COIN,
                        MONEY_LOSE_BAO_SHI_UPGRADE,
                        0, 0, 0);
  return ret > 0 ? 0 : ret;
}
bao_shi_info *bao_shi_module::find_bao_shi(player_obj *player, const char pos, const char bs_idx)
{
  for (ilist_node<bao_shi_info *> *itor = player->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    bao_shi_info *info = itor->value_;
    if (info->pos_ == pos
        && info->bs_idx_ == bs_idx)
      return info;
  }
  return NULL;
}
short bao_shi_module::last_row_min_lvl(player_obj *player, const char bs_idx)
{
  if (bs_idx - 1 <= 0)
    return MAX_BAO_SHI_POS_IDX_LVL;

  short bs_lvl[PART_END] = {0};
  for (ilist_node<bao_shi_info *> *itor = player->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    bao_shi_info *info = itor->value_;
    if (info->bs_idx_ == bs_idx - 1
        && info->pos_ > 0
        && info->pos_ < PART_END)
      bs_lvl[(int)info->pos_] = info->bs_lvl_;
  }

  short min_lvl = MAX_BAO_SHI_POS_IDX_LVL;
  for (int i = 1; i < PART_END; ++i)
  {
    bs_lvl[i] < min_lvl ? min_lvl = bs_lvl[i] : 0;
  }
  return min_lvl;
}
void bao_shi_module::do_add_bs_exp(player_obj *player, bao_shi_info *info, const int add_exp)
{
  if (info == NULL
      || add_exp <= 0)
    return ;

  int left_exp = add_exp;
  const int max_lvl = bao_shi_lvl_cfg::instance()->max_lvl();
  const int old_lvl = info->bs_lvl_;
  for (;;)
  {
    if (info->bs_lvl_ >= max_lvl)
      break ;

    int64_t sum_exp = info->bs_exp_ + left_exp;
    const int need_exp = bao_shi_lvl_cfg::instance()->lvl_up_exp(info->bs_lvl_);
    if (sum_exp >= need_exp)
    {
      left_exp -= (need_exp - info->bs_exp_);
      ++info->bs_lvl_;
      info->bs_exp_ = 0;
      bao_shi_module::on_lvl_up(player, info);
    }else
    {
      info->bs_exp_ = sum_exp;
      break;
    }
  }
  info->bs_lvl_ > old_lvl ? info->up_cnt_ = 0 : ++info->up_cnt_;
}
bool bao_shi_module::do_luck_lvl_up(player_obj *player, bao_shi_info *info)
{
  if (info->up_cnt_ != 0) return false;
  const int ran = rand() % BAO_SHI_LUCK_CFG_DIVISOR + 1;
  if (ran > bao_shi_lvl_cfg::instance()->luck_rate(info->bs_lvl_))
    return false;
  ++info->bs_lvl_;
  info->bs_exp_ = 0;
  info->up_cnt_ = 0;
  bao_shi_module::on_lvl_up(player, info);
  lucky_egg_module::on_bao_shi_upgrade_lucky_egg(player, info->pos_, info->bs_lvl_);
  return true;
}
void bao_shi_module::on_lvl_up(player_obj *player, bao_shi_info *info)
{
  if (info->bs_lvl_ % 10 == 0)
  {
    notice_module::bao_shi_up_to_10th(player->id(),
                                      player->name(),
                                      info->pos_,
                                      info->bs_lvl_);
  }
  cheng_jiu_module::on_bao_shi_lvl(player, info->pos_, info->bs_lvl_);

  short sum_bao_shi_lvl = bao_shi_module::get_sum_bao_shi_lvl(player);
  if (sum_bao_shi_lvl != player->sum_bao_shi_lvl())
  {
    player->sum_bao_shi_lvl(sum_bao_shi_lvl);
    player->broadcast_sum_bao_shi_lvl();
  }
  kai_fu_act_obj::on_all_bao_shi_lvl_update(player,
                                            bao_shi_module::get_bao_shi_all_lvl(player));
}
int bao_shi_module::get_bao_shi_all_lvl(player_obj *player)
{
  if (player->bao_shi_list_.size() != global_param_cfg::bao_shi_pos_hole_amt * (PART_END - 1))
    return 0;
  int all_lvl = 9999;
  for (ilist_node<bao_shi_info *> *itor = player->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
    all_lvl = util::min(all_lvl, itor->value_->bs_lvl_);
  return all_lvl;
}
int bao_shi_module::get_sum_bao_shi_lvl(player_obj *player)
{
  int sum_lvl = 0;
  for (ilist_node<bao_shi_info *> *itor = player->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
    sum_lvl += itor->value_->bs_lvl_;
  return sum_lvl;
}
void player_obj::broadcast_sum_bao_shi_lvl()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << this->sum_bao_shi_lvl_;
  ph->set(0, NTF_BROADCAST_SUM_BAO_SHI_LVL, 0, mb.length());
  this->do_broadcast(&mb, true);
}
