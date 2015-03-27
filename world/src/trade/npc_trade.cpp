#include "player_obj.h"
#include "package_module.h"
#include "item_config.h"
#include "item_obj.h"
#include "shop_config.h"
#include "behavior_id.h"
#include "istream.h"
#include "sys_log.h"
#include "message.h"
#include "util.h"
#include "error.h"
#include "def.h"
#include "behavior_log.h"
#include "task_module.h"
#include "global_param_cfg.h"
#include "mall_module.h"
#include "vip_module.h"
#include "ghz_module.h"
#include "equip_module.h"
#include "clsid.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");

#define MAX_ONCE_BUY_ITEM_AMT    999

int player_obj::clt_buy_item(const char *msg, const int len)
{
  int npc_cid  = 0;
  int group_id = 0;
  int amt  = 0;
  int param = 0;
  in_stream is(msg, len);
  is >> npc_cid >> group_id >> amt >> param;

  int ret = this->do_buy_item(npc_cid, group_id, amt, param);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int player_obj::do_buy_item(const int npc_cid,
                            const int group_id,
                            const int amt,
                            const int param)
{
  const goods_obj *go = shop_config::instance()->get_goods(npc_cid, group_id);
  if (go == NULL)
    return ERR_CONFIG_NOT_EXIST;

  if (amt < 0 || amt > MAX_ONCE_BUY_ITEM_AMT)
    return ERR_CLIENT_OPERATE_ILLEGAL;

  int ret = this->can_buy_item(npc_cid, group_id, amt, param);
  if (ret != 0) return ret;

  char bind_type = go->bind_type_;
  if (go->price_type_ == M_BIND_UNBIND_DIAMOND)
    bind_type = BIND_TYPE;
  int total_amt = go->item_cnt_ * amt;
  if (package_module::would_be_full(this,
                                    PKG_PACKAGE,
                                    go->item_cid_,
                                    total_amt,
                                    bind_type))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  const int cost = go->price_ * amt;
  ret = this->is_money_enough(go->price_type_, cost);
  if (ret != 0) return ret;

  // ok
  this->do_lose_money(cost,
                      go->price_type_,
                      MONEY_LOSE_NPC_TRADE,
                      npc_cid,
                      go->item_cid_,
                      total_amt);

  package_module::do_insert_item(this,
                                 PKG_PACKAGE,
                                 go->item_cid_,
                                 total_amt,
                                 bind_type,
                                 ITEM_GOT_NPC_TRADE,
                                 npc_cid,
                                 0);
  this->on_buy_item_ok(npc_cid, group_id, amt, param);
  return 0;
}
int player_obj::clt_buy_and_use_item(const char *msg, const int len)
{
  int amt = 0;
  int npc_cid = 0;
  int group_id = 0;
  in_stream is(msg, len);
  is >> npc_cid >> group_id >> amt;

  // check
  const goods_obj *go = shop_config::instance()->get_goods(npc_cid, group_id);
  if (go == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  const item_cfg_obj *ico = item_config::instance()->get_item(go->item_cid_);
  if (ico == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (amt < 0 || amt > 1) // 为了避免乘法溢出，限制单次购买数量（跟叠加上限无关）
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const int cost = amt * go->price_;
  int ret = this->is_money_enough(go->price_type_, cost);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = this->can_buy_item(npc_cid, group_id, amt);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  char bind_type = go->bind_type_;
  if (go->price_type_ == M_BIND_UNBIND_DIAMOND)
    bind_type = BIND_TYPE;
  int total_amt = amt * go->item_cnt_;
  ret = this->do_buy_and_use_item(ico,
                                  npc_cid,
                                  cost,
                                  go->price_type_,
                                  group_id,
                                  total_amt,
                                  go->item_cid_,
                                  amt,
                                  bind_type);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  this->on_buy_item_ok(npc_cid, group_id, amt);
  return 0;
}
int player_obj::can_buy_item(const int npc_cid,
                             const int item_cid,
                             const int amount,
                             const int param)
{
  int ret = mall_module::can_buy_item(this, npc_cid, item_cid, amount, param);
  if (ret != 0) return ret;

  ret = ghz_module::can_buy_item(this, npc_cid);
  if (ret != 0) return ret;

  ret = vip_module::can_buy_item(this, npc_cid);
  if (ret != 0) return ret;
  return 0;
}
void player_obj::on_buy_item_ok(const int npc_cid,
                                const int group_id,
                                const int amt,
                                const int param)
{
  mall_module::on_buy_item_ok(this, npc_cid, group_id, amt, param);
}
int player_obj::do_buy_and_use_item(const item_cfg_obj *ico,
                                    const int npc_cid,
                                    const int cost,
                                    const int price_type,
                                    const int /*group_id*/,
                                    const int total_amt,
                                    const int item_cid,
                                    const int /*amt*/,
                                    const char bind)
{
  item_obj *io = package_module::alloc_new_item(this->id(), item_cid, total_amt, bind);
  int ret = this->do_use_item(io, ico, total_amt);
  package_module::release_item(io);
  if (ret < 0) return ret;
  this->do_lose_money(cost, price_type, MONEY_LOSE_NPC_TRADE, npc_cid, item_cid, total_amt);
  return 0;
}
int player_obj::clt_sell_item(const char *msg, const int len)
{
  int amt = 0;
  int item_id = 0;
  in_stream is(msg, len);
  is >> item_id >> amt;

  int ret = this->do_sell_item(item_id, amt);
  if (ret < 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int player_obj::do_sell_item(const int item_id, const int amt)
{
  item_obj *io = package_module::find_item(this, item_id);
  if (io == NULL) return ERR_ITEM_NOT_EXIST;
  if (amt <= 0 || io->amount_ < amt) return ERR_ITEM_AMOUNT_NOT_ENOUGH;

  int total_price = 0;
  fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao(io->cid_);
  if (fbco != NULL)
    total_price = fbco->price_ * amt;
  else if (item_config::instance()->get_equip(io->cid_) != NULL)
  {
    int score = equip_module::do_calc_equip_score(io);
    total_price = (int)::ceil(score * global_param_cfg::equip_price_coe / 1000.0);
  }else
  {
    item_cfg_obj *ico = item_config::instance()->get_item(io->cid_);
    if (ico != NULL)
    {
      if (!ico->can_sell_) return ERR_ITEM_CAN_NOT_SELL;
      total_price = ico->price_ * amt;
    }else
      return ERR_CONFIG_NOT_EXIST;
  }
  if (total_price <= 0) return ERR_CLIENT_OPERATE_ILLEGAL;

  if (this->is_money_upper_limit(M_COIN, total_price))
    return ERR_MONEY_UPPER_LIMIT;

  this->do_got_money(total_price, M_COIN, MONEY_GOT_NPC_TRADE, io->cid_);
  package_module::do_remove_item(this, io, amt, ITEM_LOSE_NPC_TRADE, 0); // item release
  return 0;
}
int player_obj::do_check_item_enough_by_auto_buy(const int item_cid,
                                                 const int need_amount,
                                                 const char auto_buy)
{
  int left_cai_liao_amt = package_module::calc_item_amount(this, item_cid);
  int auto_buy_cai_liao_amt = 0;
  if (left_cai_liao_amt < need_amount)
  {
    if (auto_buy == 0)
      return ERR_CAI_LIAO_NOT_ENOUGH;
    else
    {
      auto_buy_cai_liao_amt = need_amount - left_cai_liao_amt;

      const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                               item_cid);
      if (go == NULL)
        return ERR_CONFIG_NOT_EXIST;
      if (go->price_ <= 0) return 0;

      int ret = this->is_money_enough(go->price_type_, go->price_ * auto_buy_cai_liao_amt);
      if (ret != 0) return ret;
    }
  }
  return auto_buy_cai_liao_amt;
}
int player_obj::do_auto_buy_item(const int item_cid, const int amt)
{
  if (amt <= 0) return 0;
  const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                           item_cid);
  if (go == NULL) return ERR_CONFIG_NOT_EXIST;

  const int cost = go->price_ * amt;
  if (cost < 0) return ERR_UNKNOWN;
  int ret = this->is_money_enough(go->price_type_, cost);
  if (ret != 0) return ret;

  this->do_lose_money(cost,
                      go->price_type_,
                      MONEY_LOSE_NPC_TRADE,
                      global_param_cfg::mall_npc_cid,
                      item_cid,
                      amt);
  return cost;
}
int player_obj::clt_diamond_to_coin(const char *msg, const int len)
{
  int type  = 0;
  in_stream is(msg, len);
  is >> type;
  if (type < 1
      || size_t(type) > sizeof(global_param_cfg::diamond_to_coin)/sizeof(global_param_cfg::diamond_to_coin[0]))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = this->is_money_enough(M_DIAMOND, global_param_cfg::diamond_to_coin[type].first_);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  if (this->is_money_upper_limit(M_COIN, global_param_cfg::diamond_to_coin[type].second_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);

  // ok
  this->do_lose_money(global_param_cfg::diamond_to_coin[type].first_,
                      M_DIAMOND,
                      MONEY_LOSE_DIAMOND_TO_COIN,
                      type,
                      0,
                      0);
  this->do_got_money(global_param_cfg::diamond_to_coin[type].second_,
                     M_COIN,
                     MONEY_GOT_DIAMOND_TO_COIN,
                     type);
  return this->send_respond_ok(RES_DIAMOND_TO_COIN, NULL);
}
