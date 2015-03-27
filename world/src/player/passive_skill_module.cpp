#include "passive_skill_module.h"
#include "passive_skill_cfg.h"
#include "passive_skill_upgrade_cfg.h"
#include "passive_skill_info.h"
#include "package_module.h"
#include "task_module.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "huo_yue_du_module.h"
#include "lucky_egg_module.h"
#include "notice_module.h"
#include "behavior_id.h"
#include "player_obj.h"
#include "db_proxy.h"
#include "sys_log.h"
#include "istream.h"
#include "message.h"
#include "error.h"
#include "clsid.h"
#include "kai_fu_act_obj.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("skill");
static ilog_obj *e_log = err_log::instance()->get_ilog("skill");

void passive_skill_module::destroy(player_obj *player)
{
  while (!player->passive_skill_list_.empty())
  {
    passive_skill_info *p = player->passive_skill_list_.pop_front();
    passive_skill_info_pool::instance()->release(p);
  }
}
passive_skill_info *passive_skill_module::get_skill(player_obj *player, const int skill_cid)
{
  ilist_node<passive_skill_info *> *l_itor = player->passive_skill_list_.head();
  for (; l_itor != NULL; l_itor = l_itor->next_)
  {
    if (l_itor->value_->cid_ == skill_cid)
      return l_itor->value_;
  }
  return NULL;
}
int passive_skill_module::handle_db_get_passive_skill_list_result(player_obj *player, in_stream &is)
{
  player->handle_db_get_passive_skill_list_result(is);
  return 0;
}
void player_obj::handle_db_get_passive_skill_list_result(in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char skill_bf[sizeof(passive_skill_info) + 4] = {0};
    stream_istr si(skill_bf, sizeof(skill_bf));
    is >> si;
    passive_skill_info *sinfo = (passive_skill_info *)skill_bf;

    const passive_skill_cfg_obj *p = passive_skill_cfg::instance()->get_skill(sinfo->cid_);
    if (p == NULL) continue;

    passive_skill_info *new_sinfo = passive_skill_info_pool::instance()->alloc();
    ::memcpy(new_sinfo, sinfo, sizeof(passive_skill_info));
    this->passive_skill_list_.push_back(new_sinfo);
  }
}
int passive_skill_module::db_update_skill_info(player_obj *player, passive_skill_info *psi)
{
  stream_ostr char_so((const char *)psi, sizeof(passive_skill_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << player->db_sid() << char_so;
  if (db_proxy::instance()->send_request(player->id(),
                                         REQ_UPDATE_PASSIVE_SKILL,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
void passive_skill_module::on_enter_game(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  char *cnt = mb.wr_ptr();
  *cnt = 0;
  mb << *cnt;
  ilist_node<passive_skill_info *> *itor = player->passive_skill_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    mb << itor->value_->cid_
      << itor->value_->lvl_
      << itor->value_->bless_;
    ++(*cnt);
  }
  player->do_delivery(NTF_PUSH_CHAR_PASSIVE_SKILL_LIST, &mb);

  //passive_skill_module::do_notify_skills_can_learn_or_upgrade(player);
}
void passive_skill_module::on_char_lvl_up(player_obj * /*player*/)
{
  //passive_skill_module::do_notify_skills_can_learn_or_upgrade(player);
}
void passive_skill_module::do_notify_skills_can_learn_or_upgrade(player_obj *player)
{
  ilist<int> *sl = passive_skill_cfg::instance()->get_skill_list(player->career());
  if (sl == NULL) return ;

  for (ilist_node<int> *itor = sl->head(); itor != NULL; itor = itor->next_)
  {
    int skill_cid = itor->value_;
    const passive_skill_cfg_obj *psuco = passive_skill_cfg::instance()->get_skill(skill_cid);
    if (psuco == NULL) continue;
    passive_skill_info *psi = passive_skill_module::get_skill(player, skill_cid);
    if ((psi == NULL && player->lvl() >= psuco->learn_lvl_)
        || (psi != NULL && player->lvl() >= psi->lvl_))
    {
      player->send_request(NTF_PASSIVE_SKILL_CAN_LEARN_UPGRADE, NULL);
      break;
    }
  }
}
int passive_skill_module::dispatch_msg(player_obj *player,
                                       const int msg_id,
                                       const char *msg,
                                       const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = passive_skill_module::FUNC(player, msg, len);     \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_UPGRADE_PASSIVE_SKILL, clt_upgrade_skill);
    SHORT_CODE(REQ_ONE_KEY_UPGRADE_PASSIVE_SKILL, clt_one_key_upgrade_skill);
    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle skill msg %d return %d", msg_id, ret);
  return ret;
}
int passive_skill_module::clt_upgrade_skill(player_obj *player, const char *msg, const int len)
{
  int skill_cid = 0;
  char auto_buy = 0;
  in_stream is(msg, len);
  is >> skill_cid >> auto_buy;

  const passive_skill_cfg_obj *p = passive_skill_cfg::instance()->get_skill(skill_cid);
  passive_skill_info *psi = passive_skill_module::get_skill(player, skill_cid);
  if (p == NULL
      || p->career_ != player->career()
      || player->lvl() < p->learn_lvl_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int cost_coin_amt = 0;
  int total_cost_cai_liao_amt = 0;
  int cost_cai_liao_amt = 0;
  int auto_buy_cai_liao_amt = 0;
  int cost_item_cid = 0;
  int new_up_lvl = 0;
  int last_up_lvl = 0;
  char result = 1; // 成功
  int ret = passive_skill_module::do_upgrade_skill(player,
                                                   p,
                                                   auto_buy,
                                                   skill_cid,
                                                   cost_item_cid,
                                                   cost_coin_amt,
                                                   total_cost_cai_liao_amt,
                                                   cost_cai_liao_amt,
                                                   auto_buy_cai_liao_amt,
                                                   last_up_lvl,
                                                   new_up_lvl,
                                                   result);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_lose_money(cost_coin_amt,
                        M_COIN,
                        MONEY_LOSE_UPGRADE_PASSIVE_SKILL,
                        0,
                        0,
                        0);
  player->do_auto_buy_item(cost_item_cid, auto_buy_cai_liao_amt);
  package_module::do_remove_item(player,
                                 cost_item_cid,
                                 cost_cai_liao_amt,
                                 ITEM_LOSE_UPGRADE_PASSIVE_SKILL,
                                 0);
  psi = passive_skill_module::get_skill(player, skill_cid);
  if (psi != NULL)
    passive_skill_module::on_upgrade_over(player, psi);
  if (last_up_lvl != new_up_lvl)
  {
    player->do_calc_attr_affected_by_passive_skill();
    player->on_attr_update(ZHAN_LI_PSKILL, skill_cid);
    kai_fu_act_obj::on_all_pskill_lvl_update(player,
                                             passive_skill_module::get_pskill_all_lvl(player));
  }
  return 0;
}
void passive_skill_module::on_upgrade_over(player_obj *player, passive_skill_info *psi)
{
  out_stream os(client::send_buf, client::send_buf_len);
  passive_skill_module::do_build_skill_info(psi, os);
  player->send_request(NTF_CHAR_PASSIVE_SKILL_UPDATE, &os);

  passive_skill_module::db_update_skill_info(player, psi);
  task_module::on_upgrade_pskill(player);
}
void passive_skill_module::do_build_skill_info(passive_skill_info *psi, out_stream &os)
{
  os << psi->cid_ << psi->lvl_ << psi->bless_;
}
int passive_skill_module::clt_one_key_upgrade_skill(player_obj *player, const char *msg, const int len)
{
  int skill_cid = 0;
  char auto_buy = 0;
  in_stream is(msg, len);
  is >> skill_cid >> auto_buy;

  const passive_skill_cfg_obj *p = passive_skill_cfg::instance()->get_skill(skill_cid);
  if (p == NULL
      || p->career_ != player->career()
      || player->lvl() < p->learn_lvl_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = passive_skill_module::do_one_key_upgrade_skill(player, skill_cid, auto_buy);
  if (ret != 0)
    return player->send_msg(NTF_OPERATE_RESULT, ret, NULL);
  return 0;
}
int passive_skill_module::do_one_key_upgrade_skill(player_obj *player,
                                                   const int skill_cid,
                                                   const char auto_buy)
{
  const passive_skill_cfg_obj *psco = passive_skill_cfg::instance()->get_skill(skill_cid);
  if (psco == NULL) return ERR_CONFIG_NOT_EXIST;

  int cost_coin_amt = 0;
  int total_cost_cai_liao_amt = 0;
  int cost_cai_liao_amt = 0;
  int auto_buy_cai_liao_amt = 0;
  int cost_item_cid = 0;
  int new_up_lvl = -1;
  int last_up_lvl = 0;
  char result = 1; // 成功
  while (true) // upgrade one skill until failed!
  {
    passive_skill_info *psi = passive_skill_module::get_skill(player, skill_cid);
    if ((psi == NULL && player->lvl() >= psco->learn_lvl_)
        || (psi != NULL && player->lvl() > psi->lvl_))
    {
      int ret = passive_skill_module::do_upgrade_skill(player,
                                                       psco,
                                                       auto_buy,
                                                       skill_cid,
                                                       cost_item_cid,
                                                       cost_coin_amt,
                                                       total_cost_cai_liao_amt,
                                                       cost_cai_liao_amt,
                                                       auto_buy_cai_liao_amt,
                                                       last_up_lvl,
                                                       new_up_lvl,
                                                       result);
      if (ret != 0)
        break;

      if (last_up_lvl != new_up_lvl)
        break;
    }else
      break;
  } // end of `while (true)'

  if (new_up_lvl != -1)
  {
    player->do_lose_money(cost_coin_amt,
                          M_COIN,
                          MONEY_LOSE_UPGRADE_PASSIVE_SKILL,
                          0,
                          0,
                          0);
    player->do_auto_buy_item(cost_item_cid, auto_buy_cai_liao_amt);
    package_module::do_remove_item(player,
                                   cost_item_cid,
                                   cost_cai_liao_amt,
                                   ITEM_LOSE_UPGRADE_PASSIVE_SKILL,
                                   0);

    passive_skill_info *psi = passive_skill_module::get_skill(player, skill_cid);
    if (psi != NULL)
      passive_skill_module::on_upgrade_over(player, psi);

    if (last_up_lvl != new_up_lvl)
    {
      player->do_calc_attr_affected_by_passive_skill();
      player->on_attr_update(ZHAN_LI_PSKILL, skill_cid);
      kai_fu_act_obj::on_all_pskill_lvl_update(player,
                                               passive_skill_module::get_pskill_all_lvl(player));
    }
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << result << cost_coin_amt << cost_cai_liao_amt + auto_buy_cai_liao_amt;
  return player->send_request(RES_ONE_KEY_UPGRADE_PASSIVE_SKILL, &os);
}
int passive_skill_module::do_upgrade_skill(player_obj *player,
                                           const passive_skill_cfg_obj *psco,
                                           const char auto_buy,
                                           const int skill_cid,
                                           int &cost_item_cid,
                                           int &cost_coin_amt,
                                           int &total_cost_cai_liao_amt,
                                           int &cost_cai_liao_amt,
                                           int &auto_buy_cai_liao_amt,
                                           int &last_up_lvl,
                                           int &new_up_lvl,
                                           char &result)
{
  passive_skill_info *psi = passive_skill_module::get_skill(player, skill_cid);
  passive_skill_info tmp_psi;
  if (psi == NULL)
  {
    psi = &tmp_psi;
    psi->char_id_ = player->id();
    psi->cid_ = skill_cid;
    psi->lvl_ = 0;
  }
  const passive_skill_upgrade_cfg_obj *psuco =
    passive_skill_upgrade_cfg::instance()->get_upgrade_info(psi->lvl_ + 1);
  if (psuco == NULL)
    return ERR_CONFIG_NOT_EXIST;
  if (psi->lvl_ >= player->lvl())
    return ERR_LVL_NOT_ENOUGH;
  int ret = player->is_money_enough(M_COIN, cost_coin_amt + psuco->cost_);
  if (ret != 0)
  {
    result = 3;  // 3：金币不足
    return ret;
  }
  if (cost_item_cid == 0) cost_item_cid = psuco->item_cid_;

  ret = player->do_check_item_enough_by_auto_buy(psuco->item_cid_,
                                                 psuco->item_cnt_ + total_cost_cai_liao_amt,
                                                 auto_buy);
  if (ret < 0)
  {
    result = 2; // 2：材料不足
    return ret;
  }

  // check ok ...
  auto_buy_cai_liao_amt = ret;

  last_up_lvl = psi->lvl_;
  int r = rand() % 10000 + 1;
  if (psi->up_cnt_ == 0
      && last_up_lvl > 0
      && r <= psuco->lucky_rate_) // lucky egg
  {
    new_up_lvl = last_up_lvl + 1;
    psi->up_cnt_ = 0;
    psi->bless_ = 0;
    lucky_egg_module::on_passive_skill_egg(player, skill_cid, new_up_lvl);
  }else // normal
  {
    if (psi->up_cnt_ < psuco->min_upgrade_cnt_ // 没有达到最小升级次数，是不走概率的
        || r > psuco->probability_ + vip_module::to_get_passive_skill_add_rate(player))
    {
      int add_bless = psuco->min_bless_val_;
      if (psuco->min_bless_val_ < psuco->max_bless_val_)
        add_bless = psuco->min_bless_val_ + rand() % (psuco->max_bless_val_ - psuco->min_bless_val_ + 1);

      if (psi->bless_ + add_bless >= psuco->total_bless_val_)
      {
        new_up_lvl = last_up_lvl + 1;
        psi->up_cnt_ = 0;
        psi->bless_ = 0;
      }else
      {
        new_up_lvl = last_up_lvl;
        psi->up_cnt_ += 1;
        psi->bless_ += add_bless;
      }
    }else
    {
      new_up_lvl = last_up_lvl + 1;
      psi->up_cnt_ = 0;
      psi->bless_ = 0;
    }
  }
  psi->lvl_ = new_up_lvl;

  if (tmp_psi.char_id_ != 0) // new one
  {
    psi = passive_skill_info_pool::instance()->alloc();
    ::memcpy(psi, &tmp_psi, sizeof(passive_skill_info));
    player->passive_skill_list_.push_back(psi);
  }

  cost_coin_amt += psuco->cost_;
  total_cost_cai_liao_amt += psuco->item_cnt_;
  if (auto_buy_cai_liao_amt < total_cost_cai_liao_amt)
    cost_cai_liao_amt = (total_cost_cai_liao_amt - auto_buy_cai_liao_amt);

  if (new_up_lvl != last_up_lvl)
  {
    cheng_jiu_module::on_passive_skill_level_up(player, psco->effect_id_, new_up_lvl);
    notice_module::passive_skill_upgrade(player->id(),
                                         player->name(),
                                         skill_cid,
                                         new_up_lvl);
  }

  huo_yue_du_module::on_upgrade_passive_skill(player);
  return 0;
}
int passive_skill_module::get_pskill_all_lvl(player_obj *player)
{
  ilist<int> *sl = passive_skill_cfg::instance()->get_skill_list(player->career());
  if (sl == NULL
      || sl->size() != player->passive_skill_list_.size())
    return 0;

  int all_lvl = 0;
  for (ilist_node<passive_skill_info *> *l_itor = player->passive_skill_list_.head();
       l_itor != NULL;
       l_itor = l_itor->next_)
    all_lvl = util::min(all_lvl, l_itor->value_->lvl_);
  return all_lvl;
}
