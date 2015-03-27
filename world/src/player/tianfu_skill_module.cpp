#include "tianfu_skill_module.h"
#include "tianfu_skill_info.h"
#include "tianfu_skill_cfg.h"
#include "tianfu_skill_upgrade_cfg.h"
#include "player_obj.h"
#include "package_module.h"
#include "behavior_id.h"
#include "global_param_cfg.h"
#include "all_char_info.h"
#include "vip_module.h"
#include "kai_fu_act_obj.h"
#include "notice_module.h"
#include "db_proxy.h"
#include "message.h"
#include "error.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("skill");
static ilog_obj *e_log = err_log::instance()->get_ilog("skill");

void tianfu_skill_module::destroy(char_obj *player)
{
  while (!player->tianfu_skill_list_.empty())
  {
    tianfu_skill_info *p = player->tianfu_skill_list_.pop_front();
    tianfu_skill_info_pool::instance()->release(p);
  }
}
void tianfu_skill_module::on_enter_game(player_obj *player)
{
  if (player->tianfu_skill_list_.empty()) return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  char *cnt = mb.wr_ptr();
  *cnt = 0;
  mb << *cnt;
  ilist_node<tianfu_skill_info *> *itor = player->tianfu_skill_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    mb << itor->value_->cid_
      << itor->value_->lvl_
      << itor->value_->exp_;
    ++(*cnt);
  }
  player->do_delivery(NTF_PUSH_CHAR_TIANFU_SKILL_LIST, &mb);
}
int tianfu_skill_module::handle_db_get_tianfu_skill_list_result(player_obj *player,
                                                                in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char skill_bf[sizeof(tianfu_skill_info) + 4] = {0};
    stream_istr si(skill_bf, sizeof(skill_bf));
    is >> si;
    tianfu_skill_info *info = (tianfu_skill_info *)skill_bf;

    if (tianfu_skill_cfg::instance()->get_skill(info->cid_) == NULL)
      continue;

    tianfu_skill_info *new_sinfo = tianfu_skill_info_pool::instance()->alloc();
    ::memcpy(new_sinfo, info, sizeof(tianfu_skill_info));
    player->tianfu_skill_list_.push_back(new_sinfo);
  }
  return 0;
}
void char_obj::do_tianfu_effect(char_obj *target, const time_value &now)
{
  if (this->tianfu_skill_list_.empty()) return ;

  if ((now - this->last_tianfu_worked_time_).msec() < global_param_cfg::tianfu_skill_cd)
    return ;

  int randnum = rand() % 10000 + 1, sum = 0;
  const tianfu_skill_cfg_obj *co = NULL;
  ilist_node<tianfu_skill_info *> *iter = this->tianfu_skill_list_.head();
  for (; iter != NULL; iter = iter->next_)
  {
    co = tianfu_skill_cfg::instance()->get_skill(iter->value_->cid_);
    if (co == NULL || co->buff_info_.empty()) continue;
    sum += co->init_val_ + (iter->value_->lvl_ - 1)*co->inc_val_;
    if (randnum <= sum)
      break;
  }
  if (iter == NULL) return;

  this->last_tianfu_worked_time_ = now;
  this->broadcast_tianfu_skill(iter->value_->cid_);

  if (co->init_val_ + (iter->value_->lvl_ - 1)*co->inc_val_
      <= target->tianfu_work_rate(co->anti_cid_))
  {
    target->broadcast_tianfu_skill(co->anti_cid_);
    return ;
  }
  this->sk_do_insert_buff(target,
                          iter->value_->cid_,
                          0,
                          now,
                          co->buff_info_,
                          SK_BUFF_TARGET_AREA,
                          0);
}
int char_obj::tianfu_work_rate(const int skill_cid)
{
  tianfu_skill_info *tsi = tianfu_skill_module::get_skill(this, skill_cid);
  if (tsi == NULL) return 0;

  const tianfu_skill_cfg_obj *p = tianfu_skill_cfg::instance()->get_skill(skill_cid);
  if (p == NULL) return 0;
  return p->init_val_ + p->inc_val_ * (tsi->lvl_ - 1);
}

int tianfu_skill_module::dispatch_msg(player_obj *player,
                                      const int msg_id,
                                      const char *msg,
                                      const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = tianfu_skill_module::FUNC(player, msg, len);     \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_UPGRADE_TIANFU_SKILL, clt_upgrade_skill);
    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle msg %d return %d", msg_id, ret);
  return ret;
}

int tianfu_skill_module::clt_upgrade_skill(player_obj *player,
                                           const char *msg,
                                           const int len)
{
  int skill_cid = 0;
  in_stream is(msg, len);
  is >> skill_cid;

  if (tianfu_skill_cfg::instance()->get_skill(skill_cid) == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int aim_lvl = 1;
  tianfu_skill_info *tsi = tianfu_skill_module::get_skill(player, skill_cid);
  if (tsi != NULL)
    aim_lvl = tsi->lvl_ + 1;

  const tianfu_skill_upgrade_cfg_obj *p
    = tianfu_skill_upgrade_cfg::instance()->get_upgrade_info(aim_lvl);
  if (p == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = player->is_money_enough(M_COIN, p->cost_);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  int m_count = package_module::calc_item_amount(player, p->item_cid_);
  if (m_count < p->item_cnt_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAI_LIAO_NOT_ENOUGH);

  int times = 1;
  char vip_lvl = vip_module::vip_lvl(player);
  if (vip_lvl != 0
      && rand() % 100 + 1 <= global_param_cfg::tianfu_skill_upgrade_baoji_rate)
  {
    int min_times = vip_config::instance()->effect_val1(vip_lvl,
                                                        VIP_EFF_TIANFU_SKILL_BAOJI_TIMES);
    int max_times = vip_config::instance()->effect_val2(vip_lvl,
                                                        VIP_EFF_TIANFU_SKILL_BAOJI_TIMES);
    if (min_times > 0 && min_times <= max_times)
      times = rand() % (max_times - min_times + 1) + min_times;
  }
  int got_exp = p->item_cnt_ * p->item_exp_ * times;
  tsi = tianfu_skill_module::get_exp(player, tsi, skill_cid, got_exp, p->exp_);

  player->do_lose_money(p->cost_,
                        M_COIN,
                        MONEY_LOSE_UPGRADE_TIANFU_SKILL,
                        skill_cid,
                        0,
                        0);
  package_module::do_remove_item(player,
                                 p->item_cid_,
                                 p->item_cnt_,
                                 ITEM_LOSE_USE,
                                 skill_cid);

  out_stream os(client::send_buf, client::send_buf_len);
  os << tsi->cid_ << tsi->lvl_ << tsi->exp_ << times;
  return player->send_respond_ok(RES_UPGRADE_TIANFU_SKILL, &os);
}
tianfu_skill_info *tianfu_skill_module::get_skill(char_obj *player,
                                                  const int skill_cid)
{
  ilist_node<tianfu_skill_info *> *iter = player->tianfu_skill_list_.head();
  for (; iter != NULL; iter = iter->next_)
    if (iter->value_->cid_ == skill_cid)
      return iter->value_;

  return NULL;
}
void tianfu_skill_module::fill_char_brief_info(player_obj *player,
                                               char_brief_info *cbi)
{
  cbi->tianfu_skill_list_.clear();
  ilist_node<tianfu_skill_info *> *iter = player->tianfu_skill_list_.head();
  for (; iter != NULL; iter = iter->next_)
    cbi->tianfu_skill_list_.push_back(pair_t<int> (iter->value_->cid_, iter->value_->lvl_));
}
tianfu_skill_info *tianfu_skill_module::get_exp(player_obj *player,
                                                tianfu_skill_info *tsi,
                                                const int skill_cid,
                                                const int exp,
                                                int upgrade_exp)
{
  if (tsi == NULL)
  {
    tsi = tianfu_skill_info_pool::instance()->alloc();
    tsi->char_id_ = player->id();
    tsi->cid_ = skill_cid;
    tsi->lvl_ = 0;
    tsi->exp_ = 0;
    player->tianfu_skill_list_.push_back(tsi);
  }

  tsi->exp_ += exp;
  while (tsi->exp_ >= upgrade_exp)
  {
    ++ tsi->lvl_; 
    tsi->exp_ -= upgrade_exp;

    kai_fu_act_obj::on_all_tian_fu_lvl_update(player,
                                              tianfu_skill_module::get_tian_fu_all_lvl(player));
    if (tsi->lvl_ % 10 == 0)
      notice_module::tianfu_skill_upgrade(player->id(),
                                          player->name(),
                                          skill_cid,
                                          tsi->lvl_);

    const tianfu_skill_upgrade_cfg_obj *tp
      = tianfu_skill_upgrade_cfg::instance()->get_upgrade_info(tsi->lvl_ + 1);
    if (tp == NULL) break;
    upgrade_exp = tp->exp_;
  }

  stream_ostr char_so((const char *)tsi, sizeof(tianfu_skill_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << player->db_sid() << char_so;
  db_proxy::instance()->send_request(player->id(),
                                     REQ_UPDATE_TIANFU_SKILL,
                                     &char_os);
  return tsi;
}
int tianfu_skill_module::get_tian_fu_all_lvl(player_obj *player)
{
  if (player->tianfu_skill_list_.size()
      != tianfu_skill_cfg::instance()->get_tian_fu_amount())
    return 0;
  int all_lvl = 0;
  for (ilist_node<tianfu_skill_info *> *itor = player->tianfu_skill_list_.head();
       itor != NULL;
       itor = itor->next_)
    all_lvl = util::min(all_lvl, itor->value_->lvl_);
  return all_lvl;
}
