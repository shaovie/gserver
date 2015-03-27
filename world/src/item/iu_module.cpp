#include "iu_module.h"
#include "iu_config.h"
#include "player_obj.h"
#include "istream.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "item_obj.h"
#include "item_config.h"
#include "client.h"
#include "clsid.h"
#include "util.h"
#include "def.h"
#include "behavior_id.h"
#include "item_extra_info_opt.h"
#include "behavior_log.h"
#include "package_module.h"
#include "equip_module.h"
#include "notice_module.h"
#include "huo_yue_du_module.h"
#include "vip_module.h"
#include "shop_config.h"
#include "cheng_jiu_module.h"
#include "global_param_cfg.h"
#include "lucky_egg_module.h"
#include "attr_module.h"
#include "all_char_info.h"
#include "task_module.h"

// Lib header
#include <json/json.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("iu");
static ilog_obj *e_log = err_log::instance()->get_ilog("iu");

int iu_module::dispatch_msg(player_obj *player,
                            const int msg_id,
                            const char *msg,
                            const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = iu_module::FUNC(player, msg, len);        \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch(msg_id)
  {
    SHORT_CODE(REQ_EQUIP_STRENGTHEN, clt_equip_strengthen);
    SHORT_CODE(REQ_EQUIP_ONE_KEY_STRENGTHEN, clt_equip_one_key_strengthen);
    SHORT_CODE(REQ_EQUIP_FEN_JIE,    clt_equip_fen_jie);
    SHORT_CODE(REQ_EQUIP_RONG_HE,    clt_equip_rong_he);
    SHORT_CODE(REQ_EQUIP_XI_LIAN,    clt_equip_xi_lian);
    SHORT_CODE(REQ_XI_LIAN_REPLACE,  clt_xi_lian_replace);
    SHORT_CODE(REQ_EQUIP_JIN_JIE,    clt_equip_jin_jie);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int iu_module::clt_equip_strengthen(player_obj *player, const char *msg, const int len)
{
  int equip_id = 0;
  char auto_buy = 0;
  in_stream is(msg, len);
  is >> equip_id >> auto_buy;

  item_obj *equip_obj = package_module::find_item(player, equip_id);
  if (equip_obj == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  if (equip_obj->pkg_ != PKG_PACKAGE
      && equip_obj->pkg_ != PKG_EQUIP)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
  if (eco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip_obj, extra_json);

  int last_qh_lvl = item_extra_info_opt::get_qh_lvl(extra_json);
  if (last_qh_lvl < 0) last_qh_lvl = 0;
  if (last_qh_lvl >= player->lvl()
      || last_qh_lvl >= eco->strength_limit_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  const equip_strengthen_cfg_obj *esco = 
    equip_strengthen_cfg::instance()->get_strengthen_info(last_qh_lvl + 1);
  if (esco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int cost_coin_amt = 0;
  int total_cost_cai_liao_amt = 0;
  int cost_cai_liao_amt = 0;
  int auto_buy_cai_liao_amt = 0;
  int new_qh_lvl = 0;
  char result = 1; // 成功
  int ret = iu_module::do_equip_strengthen(player,
                                           equip_obj,
                                           esco,
                                           extra_json,
                                           auto_buy,
                                           cost_coin_amt,
                                           total_cost_cai_liao_amt,
                                           cost_cai_liao_amt,
                                           auto_buy_cai_liao_amt,
                                           new_qh_lvl,
                                           result);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_lose_money(cost_coin_amt,
                        M_COIN,
                        MONEY_LOSE_EQUIP_STRENGTHEN,
                        equip_obj->cid_,
                        0,
                        0);
  player->do_auto_buy_item(esco->item_cid_, auto_buy_cai_liao_amt);

  package_module::do_remove_item(player,
                                 esco->item_cid_,
                                 cost_cai_liao_amt,
                                 ITEM_LOSE_EQUIP_STRENGTHEN,
                                 equip_obj->cid_);

  iu_module::on_equip_strengthen_over(player, equip_obj);
  if (new_qh_lvl != last_qh_lvl)
  {
    if (equip_obj->pkg_ == PKG_EQUIP)
    {
      player->do_calc_attr_affected_by_equip();
      player->on_attr_update(ZHAN_LI_EQUIP_STRENGTHEN, equip_obj->cid_);
      attr_module::on_all_qh_update(player);
    }
    player->send_respond_ok(RES_EQUIP_STRENGTHEN, 0);
  }else
    player->send_respond_err(RES_EQUIP_STRENGTHEN, ERR_EQUIP_STRENGTHEN_FAILED);
  return 0;
}
int iu_module::clt_equip_one_key_strengthen(player_obj *player, const char *msg, const int len)
{
  int equip_id = 0;
  char auto_buy = 0;
  in_stream is(msg, len);
  is >> equip_id >> auto_buy;

  item_obj *equip_obj = package_module::find_item(player, equip_id);
  if (equip_obj == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);
  const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
  if (eco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (equip_obj->pkg_ != PKG_PACKAGE
      && equip_obj->pkg_ != PKG_EQUIP)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip_obj, extra_json);

  int last_qh_lvl = item_extra_info_opt::get_qh_lvl(extra_json);
  if (last_qh_lvl < 0) last_qh_lvl = 0;
  if (last_qh_lvl >= player->lvl()
      || last_qh_lvl >= eco->strength_limit_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int cost_coin_amt = 0;
  int total_cost_cai_liao_amt = 0;
  int cost_cai_liao_amt = 0;
  int auto_buy_cai_liao_amt = 0;
  int cost_item_cid = 0;
  int new_qh_lvl = -1;
  char result = 1; // 成功
  while (true)
  {
    const equip_strengthen_cfg_obj *esco = 
      equip_strengthen_cfg::instance()->get_strengthen_info(last_qh_lvl + 1);
    if (esco == NULL) break;
    if (cost_item_cid == 0) cost_item_cid = esco->item_cid_;

    int ret = iu_module::do_equip_strengthen(player,
                                             equip_obj,
                                             esco,
                                             extra_json,
                                             auto_buy,
                                             cost_coin_amt,
                                             total_cost_cai_liao_amt,
                                             cost_cai_liao_amt,
                                             auto_buy_cai_liao_amt,
                                             new_qh_lvl,
                                             result);
    if (ret != 0)
      break;

    if (new_qh_lvl > last_qh_lvl)
      break;
  } // end of `while (true)'

  if (new_qh_lvl != -1)
  {
    player->do_lose_money(cost_coin_amt,
                          M_COIN,
                          MONEY_LOSE_EQUIP_STRENGTHEN,
                          equip_obj->cid_,
                          0,
                          0);
    player->do_auto_buy_item(cost_item_cid, auto_buy_cai_liao_amt);

    package_module::do_remove_item(player,
                                   cost_item_cid,
                                   cost_cai_liao_amt,
                                   ITEM_LOSE_EQUIP_STRENGTHEN,
                                   equip_obj->cid_);

    iu_module::on_equip_strengthen_over(player, equip_obj);
    if (new_qh_lvl != last_qh_lvl
        && equip_obj->pkg_ == PKG_EQUIP)
    {
      player->do_calc_attr_affected_by_equip();
      player->on_attr_update(ZHAN_LI_EQUIP_STRENGTHEN, equip_obj->cid_);
      attr_module::on_all_qh_update(player);
    }
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << result << cost_coin_amt << cost_cai_liao_amt + auto_buy_cai_liao_amt;
  return player->send_request(RES_EQUIP_ONE_KEY_STRENGTHEN, &os);
}
int iu_module::do_equip_strengthen(player_obj *player,
                                   item_obj *equip_obj,
                                   const equip_strengthen_cfg_obj *esco,
                                   Json::Value &extra_json,
                                   const char auto_buy,
                                   int &cost_coin_amt,
                                   int &total_cost_cai_liao_amt,
                                   int &cost_cai_liao_amt,
                                   int &auto_buy_cai_liao_amt,
                                   int &new_qh_lvl,
                                   char &result)
{
  int ret = player->is_money_enough(M_COIN, esco->cost_ + cost_coin_amt);
  if (ret != 0)
  {
    result = 3;  // 3：金币不足
    return ret;
  }

  ret = player->do_check_item_enough_by_auto_buy(esco->item_cid_,
                                                 esco->item_cnt_ + total_cost_cai_liao_amt,
                                                 auto_buy);
  if (ret < 0)
  {
    result = 2; // 2：材料不足
    return ret;
  }

  // check ok ...
  auto_buy_cai_liao_amt = ret;

  int last_qh_lvl = item_extra_info_opt::get_qh_lvl(extra_json);
  if (last_qh_lvl < 0) last_qh_lvl = 0;

  int cur_qh_cnt = item_extra_info_opt::get_qh_cnt(extra_json);
  if (cur_qh_cnt < 0) cur_qh_cnt = 0;

  int cur_qh_bless = item_extra_info_opt::get_qh_bless(extra_json);
  if (cur_qh_bless < 0) cur_qh_bless = 0;

  int r = rand() % 10000 + 1;
  if (cur_qh_cnt == 0
      && r <= esco->lucky_rate_) // lucky egg
  {
    new_qh_lvl = last_qh_lvl + 1;
    cur_qh_cnt = 0;
    cur_qh_bless = 0;
    lucky_egg_module::on_equip_strengthen_egg(player, equip_obj, new_qh_lvl);
  }else // normal
  {
    if (cur_qh_cnt < esco->min_strengthen_cnt_ // 没有达到最小升级次数，是不走概率的
        || r > esco->probability_ + vip_module::to_get_equip_strengthen_add_rate(player))
    {
      int add_bless = esco->min_bless_val_;
      if (esco->min_bless_val_ < esco->max_bless_val_)
        add_bless = esco->min_bless_val_ + rand() % (esco->max_bless_val_ - esco->min_bless_val_ + 1);

      if (cur_qh_bless + add_bless >= esco->total_bless_val_)
      {
        new_qh_lvl = last_qh_lvl + 1;
        cur_qh_cnt = 0;
        cur_qh_bless = 0;
      }else // failed
      {
        new_qh_lvl = last_qh_lvl;
        cur_qh_cnt += 1;
        cur_qh_bless += add_bless;
      }
    }else
    {
      new_qh_lvl = last_qh_lvl + 1;
      cur_qh_cnt = 0;
      cur_qh_bless = 0;
    }
  }

  cost_coin_amt += esco->cost_;
  total_cost_cai_liao_amt += esco->item_cnt_;
  if (auto_buy_cai_liao_amt < total_cost_cai_liao_amt)
    cost_cai_liao_amt = (total_cost_cai_liao_amt - auto_buy_cai_liao_amt);

  item_extra_info_opt::set_qh_cnt(equip_obj, cur_qh_cnt, extra_json);
  item_extra_info_opt::set_qh_bless(equip_obj, cur_qh_bless, extra_json);

  if (new_qh_lvl > last_qh_lvl)
  {
    item_extra_info_opt::set_qh_lvl(equip_obj, new_qh_lvl, extra_json);

    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
    if (eco != NULL)
      cheng_jiu_module::on_equip_qh(player, eco->part_, new_qh_lvl);

    notice_module::equip_strengthen(player->id(),
                                    player->name(),
                                    equip_obj,
                                    new_qh_lvl);
  }
  huo_yue_du_module::on_equip_qh(player);
  return 0;
}
void iu_module::on_equip_strengthen_over(player_obj *player, item_obj *equip_obj)
{
  int item_update_size = 0;
  g_item_update_info[item_update_size++] = package_module::UPD_EXTRA_INFO;
  package_module::on_item_update(player,
                                 UPD_ITEM,
                                 equip_obj,
                                 g_item_update_info,
                                 item_update_size,
                                 0);
  task_module::on_equip_strengthen(player);
}
int iu_module::clt_equip_fen_jie(player_obj *player, const char *msg, const int len)
{
  char equip_cnt = 0;
  in_stream is(msg, len);
  is >> equip_cnt;
  if (equip_cnt <= 0 || equip_cnt > 8)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int fen_jie_result_cnt = 0;
  ilist<item_obj *> to_fen_jie_equip_list;
  int total_cost = 0;
  for (int i = 0; i < equip_cnt; ++i)
  {
    int equip_id = 0;
    is >> equip_id;
    item_obj *equip_obj = package_module::find_item(player, equip_id);
    if (equip_obj == NULL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

    if (equip_obj->pkg_ != PKG_PACKAGE)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
    if (eco == NULL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

    const equip_fen_jie_cfg_obj *efjco =
      equip_fen_jie_cfg::instance()->get_fen_jie_info(eco->color_);
    if (efjco == NULL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

    total_cost += efjco->cost_;
    to_fen_jie_equip_list.push_back(equip_obj);

    bool merge = false;
    for (int item_i = 0; item_i < fen_jie_result_cnt; ++item_i)
    {
      if (g_item_amount_bind[0][item_i] == efjco->item_cid_)
      {
        g_item_amount_bind[1][item_i] += efjco->item_cnt_;
        merge = true;
        break;
      }
    }
    if (!merge)
    {
      g_item_amount_bind[0][fen_jie_result_cnt] = efjco->item_cid_;
      g_item_amount_bind[1][fen_jie_result_cnt] = efjco->item_cnt_;
      g_item_amount_bind[2][fen_jie_result_cnt] = BIND_TYPE;
      ++fen_jie_result_cnt;
    }
  }

  int ret = player->is_money_enough(M_COIN, total_cost);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    g_item_amount_bind[0],
                                    g_item_amount_bind[1],
                                    g_item_amount_bind[2],
                                    fen_jie_result_cnt))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  while (!to_fen_jie_equip_list.empty())
  {
    item_obj *p = to_fen_jie_equip_list.pop_front();
    package_module::do_remove_item(player,
                                   p,
                                   p->amount_,
                                   ITEM_LOSE_EQUIP_FEN_JIE,
                                   0);
  }
  for (int i = 0; i < fen_jie_result_cnt; ++i)
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   g_item_amount_bind[0][i],
                                   g_item_amount_bind[1][i],
                                   g_item_amount_bind[2][i],
                                   ITEM_GOT_EQUIP_FEN_JIE,
                                   0,
                                   0);
  player->do_lose_money(total_cost,
                        M_COIN,
                        MONEY_LOSE_EQUIP_FEN_JIE,
                        0,
                        0,
                        0);

  cheng_jiu_module::on_equip_fen_jie(player);
  huo_yue_du_module::on_equip_fen_jie(player);

  return player->send_respond_ok(RES_EQUIP_FEN_JIE, NULL);
}
int iu_module::clt_equip_rong_he(player_obj *player, const char *msg, const int len)
{
  char auto_buy  = 0;
  int equip_id_1 = 0;
  int equip_id_2 = 0;
  in_stream is(msg, len);
  is >> auto_buy >> equip_id_1 >> equip_id_2;
  if (equip_id_1 <= 0 || equip_id_2 <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  item_obj *equip_obj_1 = package_module::find_item(player, equip_id_1);
  if (equip_obj_1 == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);
  item_obj *equip_obj_2 = package_module::find_item(player, equip_id_2);
  if (equip_obj_2 == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  const equip_cfg_obj *eco_1 = item_config::instance()->get_equip(equip_obj_1->cid_);
  if (eco_1 == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  const equip_cfg_obj *eco_2 = item_config::instance()->get_equip(equip_obj_2->cid_);
  if (eco_2 == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (eco_1->career_ != eco_2->career_
      || eco_1->part_ != eco_2->part_
      || (equip_obj_1->pkg_ != PKG_PACKAGE && equip_obj_1->pkg_ != PKG_EQUIP)
      || (equip_obj_2->pkg_ != PKG_PACKAGE && equip_obj_2->pkg_ != PKG_EQUIP)
     )
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int color = eco_1->color_ > eco_2->color_ ? eco_1->color_ : eco_2->color_;

  const equip_rong_he_cfg_obj *erhco =
    equip_rong_he_cfg::instance()->get_rong_he_info(color);
  if (erhco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int ret = player->is_money_enough(M_COIN, erhco->cost_);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = player->do_check_item_enough_by_auto_buy(erhco->item_cid_,
                                                 erhco->item_cnt_,
                                                 auto_buy);
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  int auto_buy_cai_liao_amt = ret;

  item_obj *equip_new = iu_module::do_equip_rong_he(player, equip_obj_1, equip_obj_2);

  if (eco_1->color_ >= ICOLOR_PURPLE
      || eco_2->color_ >= ICOLOR_PURPLE)
  {
    notice_module::equip_rong_he(player->id(),
                                 player->name(),
                                 equip_obj_1,
                                 equip_obj_2,
                                 equip_new);
  }
  // ok
  package_module::do_remove_item(player,
                                 equip_obj_1,
                                 equip_obj_1->amount_,
                                 ITEM_LOSE_EQUIP_RONG_HE,
                                 0);
  package_module::do_remove_item(player,
                                 equip_obj_2,
                                 equip_obj_2->amount_,
                                 ITEM_LOSE_EQUIP_RONG_HE,
                                 0);

  package_module::do_insert_item(player,
                                 equip_new->pkg_,
                                 equip_new,
                                 ITEM_GOT_EQUIP_RONG_HE,
                                 0,
                                 0);
  player->do_lose_money(erhco->cost_,
                        M_COIN,
                        MONEY_LOSE_EQUIP_RONG_HE,
                        0,
                        0,
                        0);
  player->do_auto_buy_item(erhco->item_cid_, auto_buy_cai_liao_amt);
  if (auto_buy_cai_liao_amt < erhco->item_cnt_)
    package_module::do_remove_item(player,
                                   erhco->item_cid_,
                                   erhco->item_cnt_ - auto_buy_cai_liao_amt,
                                   ITEM_LOSE_EQUIP_RONG_HE,
                                   0);
  if (equip_new->pkg_ == PKG_EQUIP)
  {
    if (eco_1->part_ == PART_ZHU_WU
        || eco_1->part_ == PART_FU_WU)
    {
      char_brief_info::on_char_equip_update(player->id(), eco_1->part_, equip_new->cid_);
      player->broadcast_equip_fino();
    }
    player->do_calc_attr_affected_by_equip();
    player->on_attr_update(ZHAN_LI_EQUIP_RONG_HE, 0);
  }
  return player->send_respond_ok(RES_EQUIP_RONG_HE, NULL);
}
item_obj *iu_module::do_equip_rong_he(player_obj *player,
                                      item_obj *equip_obj_1,
                                      item_obj *equip_obj_2)
{
  Json::Value extra_json_1;
  item_extra_info_opt::decode_extra_info(equip_obj_1, extra_json_1);
  Json::Value extra_json_2;
  item_extra_info_opt::decode_extra_info(equip_obj_2, extra_json_2);

  int qh_lvl_1 = item_extra_info_opt::get_qh_lvl(extra_json_1);
  if (qh_lvl_1 < 0) qh_lvl_1 = 0;
  int qh_lvl_2 = item_extra_info_opt::get_qh_lvl(extra_json_2);
  if (qh_lvl_2 < 0) qh_lvl_2 = 0;
  int qh_lvl_new = util::max(qh_lvl_1, qh_lvl_2);

  item_obj *equip_obj_new = package_module::alloc_new_item(equip_obj_1->char_id_,
                                                           0,
                                                           1,
                                                           BIND_TYPE);
  equip_obj_new->bind_ = equip_obj_1->bind_;
  if (equip_obj_2->bind_ == BIND_TYPE)
    equip_obj_new->bind_ = BIND_TYPE;

  if (equip_obj_1->cid_ > equip_obj_2->cid_)
  {
    equip_obj_new->cid_ = equip_obj_1->cid_;
    equip_obj_new->attr_float_coe_ = equip_obj_1->attr_float_coe_;
  }else if (equip_obj_1->cid_ < equip_obj_2->cid_)
  {
    equip_obj_new->cid_ = equip_obj_2->cid_;
    equip_obj_new->attr_float_coe_ = equip_obj_2->attr_float_coe_;
  }else
  {
    equip_obj_new->cid_ = equip_obj_1->cid_;
    equip_obj_new->attr_float_coe_ = util::max(equip_obj_1->attr_float_coe_,
                                               equip_obj_2->attr_float_coe_);
  }

  equip_obj_new->pkg_ = equip_obj_1->pkg_;
  if (equip_obj_2->pkg_ == PKG_EQUIP)
    equip_obj_new->pkg_ = equip_obj_2->pkg_;

  // 根据战力高低选洗炼属性
  obj_attr::reset_attr_v_add();
  equip_module::do_calc_attr_affected_by_equip_xi_lian(extra_json_1);
  int zhan_li_1 = attr_module::do_calc_zhan_li(player);

  obj_attr::reset_attr_v_add();
  equip_module::do_calc_attr_affected_by_equip_xi_lian(extra_json_2);
  int zhan_li_2 = attr_module::do_calc_zhan_li(player);

  if (zhan_li_1 > zhan_li_2)
    item_extra_info_opt::copy_xl_info(equip_obj_new, extra_json_1);
  else
    item_extra_info_opt::copy_xl_info(equip_obj_new, extra_json_2);

  // 保存属性
  Json::Value extra_json_new;
  item_extra_info_opt::decode_extra_info(equip_obj_new, extra_json_new);
  item_extra_info_opt::set_qh_lvl(equip_obj_new, qh_lvl_new, extra_json_new);

  return equip_obj_new;
}
int iu_module::clt_equip_xi_lian(player_obj *player, const char *msg, const int len)
{
  int equip_id = 0;
  char auto_buy = 0;
  char lock[MAX_XI_LIAN_ATTR_NUM] = {0};
  char lock_cnt = 0;
  in_stream is(msg, len);
  is >> equip_id >> auto_buy; 
  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    is >> lock[i];
    if (lock[i] > 0) ++lock_cnt;
  }

  item_obj *equip_obj = package_module::find_item(player, equip_id);
  if (equip_obj == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  if (equip_obj->pkg_ != PKG_PACKAGE
      && equip_obj->pkg_ != PKG_EQUIP)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
  if (eco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  equip_xi_lian_obj *exlo =
    equip_xi_lian_config::instance()->get_equip_xi_lian_obj(eco->color_);
  xi_lian_value_obj *xlvo =
    xi_lian_value_config::instance()->get_xi_lian_value_obj(eco->wash_id_);
  if (exlo == NULL || xlvo == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  if (lock_cnt >= exlo->max_num_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = 0;
  int auto_buy_cai_liao_amt = 0;
  int auto_buy_lock_amt = 0;
  int cost_money[M_DEF_CNT] = {0};
  ::memset(cost_money, 0, sizeof(cost_money));

  // coin
  cost_money[M_COIN] += exlo->cost_;

  // 洗练卡
  {
    // auto buy
    ret = player->do_check_item_enough_by_auto_buy(exlo->item_cid_,
                                                   exlo->item_cnt_,
                                                   auto_buy);
    if (ret < 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);
    if (ret > 0)
    {
      auto_buy_cai_liao_amt = ret;
      const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                               exlo->item_cid_);
      if (go != NULL)
        cost_money[(int)go->price_type_] += go->price_ * ret;
    }
  }
  // 洗练锁
  if (lock_cnt > 0)
  {
    // auto buy
    ret = player->do_check_item_enough_by_auto_buy(global_param_cfg::xi_lian_lock_cid,
                                                   lock_cnt,
                                                   auto_buy);
    if (ret < 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);
    if (ret > 0)
    {
      auto_buy_lock_amt = ret;
      const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                               global_param_cfg::xi_lian_lock_cid);
      if (go != NULL)
        cost_money[(int)go->price_type_] += go->price_ * ret;
    }
  }
  for (int i = 0; i < M_DEF_CNT; ++i)
  {
    if (cost_money[i] <= 0) continue;
    ret = player->is_money_enough(i, cost_money[i]);
    if (ret != 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  }

  ret = iu_module::do_equip_xi_lian(player,
                                    equip_obj, 
                                    exlo, xlvo,
                                    lock_cnt, lock,
                                    auto_buy_cai_liao_amt, auto_buy_lock_amt);
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  cheng_jiu_module::on_equip_xi_lian(player);
  huo_yue_du_module::on_equip_xi_lian(player);

  return player->send_respond_ok(RES_EQUIP_XI_LIAN);
}
int iu_module::do_equip_xi_lian(player_obj *player,
                                item_obj *equip_obj,
                                equip_xi_lian_obj *exlo,
                                xi_lian_value_obj *xlvo,
                                const char lock_cnt,
                                const char lock[],
                                const int auto_buy_cai_liao_amt,
                                const int auto_buy_lock_amt)
{
  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip_obj, extra_json);

  int xl_cnt = item_extra_info_opt::get_xl_cnt(extra_json);
  if (xl_cnt < 0) xl_cnt = 0;
  int cur_xl[MAX_XI_LIAN_ATTR_NUM][4] = {{0}};
  item_extra_info_opt::get_cur_xl(cur_xl, extra_json);

  bool cur_change = false;
  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    if (cur_xl[i][1] == 0) break;
    if (cur_xl[i][3] != lock[i])
    {
      cur_xl[i][3] = lock[i];
      cur_change = true;
    }
  }
  int count = 0;
  if (exlo->min_num_ <= lock_cnt && exlo->max_num_ > lock_cnt)
    count = rand() % (exlo->max_num_ - lock_cnt) + 1;
  else
    count = rand() % (exlo->max_num_ - exlo->min_num_ + 1) + exlo->min_num_ - lock_cnt;
  if (count + lock_cnt > MAX_XI_LIAN_ATTR_NUM)
    return ERR_CONFIG_NOT_EXIST;

  int new_xl[MAX_XI_LIAN_ATTR_NUM][4] = {{0}};

  iu_module::new_xi_lian_attr(xlvo, xl_cnt, count, cur_xl, new_xl);
  ++xl_cnt;

  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    if (new_xl[i][3] != 0) continue ;
    // 8星以上公告
    if (new_xl[i][1] >= 8)
      notice_module::equip_xi_lian(player->id(),
                                   player->name(),
                                   equip_obj,
                                   new_xl[i][1],
                                   new_xl[i][0]);
  }

  iu_module::on_equip_xi_lian_over(player,
                                   equip_obj,
                                   extra_json,
                                   xl_cnt,
                                   cur_change,
                                   cur_xl, new_xl);

  player->do_lose_money(exlo->cost_,
                        M_COIN,
                        MONEY_LOSE_EQUIP_XI_LIAN,
                        equip_obj->cid_,
                        0,
                        0);
  if (auto_buy_cai_liao_amt > 0)
    player->do_auto_buy_item(exlo->item_cid_, auto_buy_cai_liao_amt);
  if (auto_buy_lock_amt > 0)
    player->do_auto_buy_item(global_param_cfg::xi_lian_lock_cid, auto_buy_lock_amt);

  if (auto_buy_cai_liao_amt < exlo->item_cnt_)
    package_module::do_remove_item(player,
                                   exlo->item_cid_,
                                   exlo->item_cnt_ - auto_buy_cai_liao_amt,
                                   ITEM_LOSE_EQUIP_XI_LIAN,
                                   equip_obj->cid_);
  if (auto_buy_lock_amt < lock_cnt)
    package_module::do_remove_item(player,
                                   global_param_cfg::xi_lian_lock_cid,
                                   lock_cnt - auto_buy_lock_amt,
                                   ITEM_LOSE_EQUIP_XI_LIAN,
                                   equip_obj->cid_);

  if (equip_obj->pkg_ == PKG_EQUIP)
  {
    player->do_calc_attr_affected_by_equip();
    player->on_attr_update(ZHAN_LI_EQUIP_XI_LIAN, equip_obj->cid_);
  }
  return 0;
}
void iu_module::new_xi_lian_attr(xi_lian_value_obj *xlvo,
                                 const int xl_cnt,
                                 int count,
                                 int cur_xl[][4],
                                 int new_xl[][4])
{
  int last_index = 0;
  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    if (cur_xl[i][1] == 0) break;
    if (cur_xl[i][3] != 0)
    {
      new_xl[last_index][0] = cur_xl[i][0];
      new_xl[last_index][1] = cur_xl[i][1];
      new_xl[last_index][2] = cur_xl[i][2];
      new_xl[last_index][3] = cur_xl[i][3];
      ++last_index;
    }
  }
  int size = xlvo->s_value_info_.size();
  int index = 0;
  int attr_kind[ATTR_T_ITEM_CNT] = {0};
  for (int i = 0; i < size && index < ATTR_T_ITEM_CNT; ++i)
  {
    if (xlvo->s_value_info_.find(i) != NULL)
      attr_kind[index++] = i;
  }
  if (index == 0) return;
  for (; count > 0; --count, ++last_index)
  {
    // 属性
    int n = rand() % index;
    new_xl[last_index][0] = attr_kind[n];
    // 星级
    int star = 1;
    for (; star <= MAX_XI_LIAN_STAR; ++star)
    {
      int cfg_cnt = xi_lian_rule_config::instance()->xl_cnt(star);
      if (xl_cnt < cfg_cnt) break;
    }
    if (star != 1) --star;
    int all_rate = 0;
    for (int s = star; s <= MAX_XI_LIAN_STAR; ++s)
      all_rate += xi_lian_rule_config::instance()->rate(s);
    int star_rate = rand() % all_rate + 1;
    for (; star <= MAX_XI_LIAN_STAR; ++star)
    {
      star_rate -= xi_lian_rule_config::instance()->rate(star);
      if (star_rate <= 0) break;
    }
    new_xl[last_index][1] = star;

    // 数值
    xi_lian_value_obj::_s_value *sv = xlvo->s_value_info_.find(new_xl[last_index][0]);
    if (sv == NULL)
      new_xl[last_index][2] = 0;
    else
      new_xl[last_index][2] = sv->value_[star];
    // 锁
    new_xl[last_index][3] = 0;
  }
}
void iu_module::on_equip_xi_lian_over(player_obj *player,
                                      item_obj *equip_obj,
                                      Json::Value &extra_json,
                                      const int xl_cnt,
                                      const bool cur_change,
                                      int cur_xl[][4],
                                      int new_xl[][4])
{
  item_extra_info_opt::set_xl_cnt(equip_obj, xl_cnt, extra_json);

  if (cur_change)
    item_extra_info_opt::set_cur_xl(equip_obj, cur_xl, extra_json);

  item_extra_info_opt::set_new_xl(equip_obj, new_xl, extra_json);

  int item_update_size = 0;
  g_item_update_info[item_update_size++] = package_module::UPD_EXTRA_INFO;
  package_module::on_item_update(player,
                                 UPD_ITEM,
                                 equip_obj,
                                 g_item_update_info,
                                 item_update_size,
                                 0);
}
int iu_module::clt_xi_lian_replace(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int equip_id = 0;
  is >> equip_id;

  item_obj *equip_obj = package_module::find_item(player, equip_id);
  if (equip_obj == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (item_extra_info_opt::replace_cur_by_new_xl(equip_obj) != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int item_update_size = 0;
  g_item_update_info[item_update_size++] = package_module::UPD_EXTRA_INFO;
  package_module::on_item_update(player,
                                 UPD_ITEM,
                                 equip_obj,
                                 g_item_update_info,
                                 item_update_size,
                                 0);

  if (equip_obj->pkg_ == PKG_EQUIP)
  {
    player->do_calc_attr_affected_by_equip();
    player->on_attr_update(ZHAN_LI_EQUIP_XI_LIAN_REPLACE, 0);
  }

  return player->send_respond_ok(RES_XI_LIAN_REPLACE);
}
void iu_module::on_new_equip_for_xi_lian(item_obj *equip_obj)
{
  // 获得一件新装备，免费洗炼一次
  equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
  if (eco == NULL
      || eco->color_ < ICOLOR_PURPLE)
    return ;

  equip_xi_lian_obj *exlo =
    equip_xi_lian_config::instance()->get_equip_xi_lian_obj(eco->color_);
  xi_lian_value_obj *xlvo =
    xi_lian_value_config::instance()->get_xi_lian_value_obj(eco->wash_id_);
  if (exlo == NULL || xlvo == NULL) return ;

  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip_obj, extra_json);

  int xl_cnt = item_extra_info_opt::get_xl_cnt(extra_json);
  if (xl_cnt > 0) return ;

  int cur_xl[MAX_XI_LIAN_ATTR_NUM][4] = {{0}};
  int count = rand() % (exlo->max_num_ - exlo->min_num_ + 1) + exlo->min_num_;

  iu_module::new_xi_lian_attr(xlvo, xl_cnt, count, cur_xl, cur_xl);

  ++xl_cnt;
  item_extra_info_opt::set_xl_cnt(equip_obj, xl_cnt, extra_json);

  item_extra_info_opt::set_cur_xl(equip_obj, cur_xl, extra_json);
}
int iu_module::clt_equip_jin_jie(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int equip_id = 0;
  is >> equip_id;

  item_obj *equip_obj = package_module::find_item(player, equip_id);
  if (equip_obj == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (equip_obj->pkg_ != PKG_PACKAGE
      && equip_obj->pkg_ != PKG_EQUIP)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_obj->cid_);
  if (eco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  equip_jin_jie_cfg_obj *jin_jie_cfg = equip_jin_jie_cfg::instance()->get_jin_jie_info(equip_obj->cid_);
  if (jin_jie_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int ret = player->is_money_enough(M_COIN, jin_jie_cfg->cost_);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  for (ilist_node<pair_t<int> > *itor = jin_jie_cfg->cai_liao_info_.head();
       itor != NULL;
       itor = itor->next_)
  {
    int left_cai_liao_amt = package_module::calc_item_amount(player, itor->value_.first_);
    if (left_cai_liao_amt < itor->value_.second_)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAI_LIAO_NOT_ENOUGH);
  }

  equip_obj->cid_ = jin_jie_cfg->to_equip_cid_;

  player->do_lose_money(jin_jie_cfg->cost_,
                        M_COIN,
                        MONEY_LOSE_EQUIP_JIN_JIE,
                        0,
                        0,
                        0);
  for (ilist_node<pair_t<int> > *itor = jin_jie_cfg->cai_liao_info_.head();
       itor != NULL;
       itor = itor->next_)
  {
    package_module::do_remove_item(player,
                                   itor->value_.first_,
                                   itor->value_.second_,
                                   ITEM_LOSE_EQUIP_JIN_JIE,
                                   0);
  }

  int item_update_size = 0;
  g_item_update_info[item_update_size++] = package_module::UPD_CID;
  package_module::on_item_update(player,
                                 UPD_ITEM,
                                 equip_obj,
                                 g_item_update_info,
                                 item_update_size,
                                 0);

  if (equip_obj->pkg_ == PKG_EQUIP)
  {
    if (eco->part_ == PART_ZHU_WU
        || eco->part_ == PART_FU_WU)
    {
      char_brief_info::on_char_equip_update(player->id(), eco->part_, equip_obj->cid_);
      player->broadcast_equip_fino();
    }
    player->do_calc_attr_affected_by_equip();
    player->on_attr_update(ZHAN_LI_EQUIP_JIN_JIE, 0);
    attr_module::on_all_equip_color_update(player);
  }
  return player->send_respond_ok(RES_EQUIP_JIN_JIE, NULL);
}
