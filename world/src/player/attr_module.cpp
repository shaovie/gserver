#include "attr_module.h"
#include "player_obj.h"
#include "package_module.h"
#include "global_param_cfg.h"
#include "sys_log.h"
#include "basic_attr_cfg.h"
#include "lvl_param_cfg.h"
#include "zhan_li_calc_cfg.h"
#include "passive_skill_cfg.h"
#include "passive_skill_info.h"
#include "item_extra_info_opt.h"
#include "equip_module.h"
#include "istream.h"
#include "message.h"
#include "all_char_info.h"
#include "clsid.h"
#include "time_util.h"
#include "guild_config.h"
#include "guild_skill_info.h"
#include "cheng_jiu_module.h"
#include "bao_shi_cfg.h"
#include "bao_shi_info.h"
#include "title_cfg.h"
#include "skill_module.h"
#include "skill_info.h"
#include "behavior_log.h"
#include "fa_bao_config.h"
#include "fa_bao_module.h"
#include "kai_fu_act_obj.h"

// Lib header
#include <cmath>
#include <json/json.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

void player_obj::do_calc_attr_affected_by_basic()
{
  basic_attr_cfg_info *baci = basic_attr_cfg::instance()->basic_attr(this->career());
  if (baci == NULL) return ;

  this->obj_attr_.clear_value(ADD_T_BASIC);

  double attr_param = lvl_param_cfg::instance()->attr_param(this->lvl());
  double coe = attr_param / LVL_PARAM_COE_CHU_SHU;
  obj_attr::reset_attr_v_add();
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    obj_attr::attr_v_add[i] = (int)::ceil(baci->attr_val_[i] * coe);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);
}
void player_obj::do_calc_attr_affected_by_equip()
{
  this->obj_attr_.clear_value(ADD_T_EQUIP);
  obj_attr::reset_attr_v_add();
  int all_qh_lvl = 0;
  int equip_cnt = 0;
  for (int part = PART_ZHU_WU; part < PART_END; ++part)
  {
    item_obj *equip = package_module::find_item_by_part(this, part);
    if (equip == NULL) continue;
    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip->cid_);
    if (eco == NULL) continue ;

    Json::Value extra_json;
    item_extra_info_opt::decode_extra_info(equip, extra_json);

    int qh_lvl = 0;
    equip_module::do_calc_attr_affected_by_equip_qh_lvl(equip, eco, extra_json, qh_lvl);
    equip_module::do_calc_attr_affected_by_equip_xi_lian(extra_json);
    if (qh_lvl == -1) qh_lvl = 0;
    if (equip_cnt == 0)
      all_qh_lvl = qh_lvl;
    else
      all_qh_lvl = all_qh_lvl > qh_lvl ? qh_lvl : all_qh_lvl;
    equip_cnt++;
  }
  if (equip_cnt == PART_END - 1)
    equip_module::do_calc_attr_affected_by_all_qh(all_qh_lvl);
  equip_module::do_calc_attr_affected_by_equip_suit(this);
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_EQUIP);
}
short attr_module::get_equip_all_qh_lvl(player_obj *player)
{
  int all_qh_lvl = 0;
  int equip_cnt = 0;
  for (int part = PART_ZHU_WU; part < PART_END; ++part)
  {
    item_obj *equip = package_module::find_item_by_part(player, part);
    if (equip == NULL) continue;
    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip->cid_);
    if (eco == NULL) continue ;

    Json::Value extra_json;
    item_extra_info_opt::decode_extra_info(equip, extra_json);

    int qh_lvl = item_extra_info_opt::get_qh_lvl(extra_json);
    if (qh_lvl == -1) qh_lvl = 0;
    if (equip_cnt == 0)
      all_qh_lvl = qh_lvl;
    else
      all_qh_lvl = all_qh_lvl > qh_lvl ? qh_lvl : all_qh_lvl;
    equip_cnt++;
  }
  if (equip_cnt == PART_END - 1)
    return (short)all_qh_lvl;
  return 0;
}
void attr_module::on_all_qh_update(player_obj *player)
{
  short all_qh_lvl = attr_module::get_equip_all_qh_lvl(player);
  if (player->all_qh_lvl() != all_qh_lvl)
  {
    player->all_qh_lvl(all_qh_lvl);
    player->broadcast_all_qh_lvl();
    kai_fu_act_obj::on_all_qh_lvl_update(player, all_qh_lvl);
  }
}
int attr_module::get_equip_all_color(player_obj *player)
{
  int all_color = 0;
  int equip_cnt = 0;
  for (int part = PART_ZHU_WU; part < PART_END; ++part)
  {
    item_obj *equip = package_module::find_item_by_part(player, part);
    if (equip == NULL) continue;
    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip->cid_);
    if (eco == NULL) continue ;

    if (equip_cnt == 0)
      all_color = eco->color_;
    else
      all_color = all_color > eco->color_ ? eco->color_ : all_color;
    equip_cnt++;
  }
  if (equip_cnt == PART_END - 1)
    return all_color;
  return 0;
}
void attr_module::on_all_equip_color_update(player_obj *player)
{
  int all_color = attr_module::get_equip_all_color(player);
  kai_fu_act_obj::on_all_equip_color_update(player, all_color);
}
void player_obj::broadcast_all_qh_lvl()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << this->all_qh_lvl_;
  ph->set(0, NTF_BROADCAST_ALL_QH_LVL, 0, mb.length());
  this->do_broadcast(&mb, true);
}
void player_obj::do_calc_attr_affected_by_guild_skill()
{
  this->obj_attr_.clear_value(ADD_T_GUILD_SKILL);
  obj_attr::reset_attr_v_add();
  for (ilist_node<guild_skill_info *> *itor = this->guild_skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    const guild_skill_info *info = itor->value_;
    const int id = guild_skill_config::instance()->effect_id(info->cid_);
    const int val = guild_skill_config::instance()->effect_val(info->cid_, info->lvl_);
    obj_attr::attr_v_add[id] += val;
  }
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_GUILD_SKILL);
}
void player_obj::do_calc_attr_affected_by_passive_skill()
{
  this->obj_attr_.clear_value(ADD_T_PASSIVE_SKILL);
  obj_attr::reset_attr_v_add();
  ilist_node<passive_skill_info *> *l_itor = this->passive_skill_list_.head();
  for (; l_itor != NULL; l_itor = l_itor->next_)
  {
    const passive_skill_cfg_obj *psco =
      passive_skill_cfg::instance()->get_skill(l_itor->value_->cid_);
    if (psco == NULL) continue;
    double attr_param = lvl_param_cfg::instance()->attr_param(l_itor->value_->lvl_);
    obj_attr::attr_v_add[psco->effect_id_] += (int)::ceil(psco->attr_val_ * attr_param / LVL_PARAM_COE_CHU_SHU);
  }
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_PASSIVE_SKILL);
}
void player_obj::do_calc_attr_affected_by_bao_shi()
{
  this->obj_attr_.clear_value(ADD_T_BAO_SHI);
  obj_attr::reset_attr_v_add();
  for (ilist_node<bao_shi_info *> *itor = this->bao_shi_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    const bao_shi_info *info = itor->value_;
    const int id = bao_shi_eff_cfg::instance()->pos_eff_id(info->pos_);
    const int val = bao_shi_eff_cfg::instance()->eff_init_val(info->pos_);
    const double coe = lvl_param_cfg::instance()->attr_param(info->bs_lvl_) / LVL_PARAM_COE_CHU_SHU;
    obj_attr::attr_v_add[id] += int(val * coe);
  }
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BAO_SHI);
}
void player_obj::do_calc_attr_affected_by_title()
{
  this->obj_attr_.clear_value(ADD_T_TITLE);
  obj_attr::reset_attr_v_add();
  if (this->cur_title() != 0)
  {
    for (ilist_node<short> *iter = this->title_list_.head();
         iter != NULL;
         iter = iter->next_)
    {
      title_cfg_obj *tco = title_cfg::instance()->get_title_cfg_obj(iter->value_);
      if (tco == NULL) continue;

      for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
        obj_attr::attr_v_add[i] += tco->attr_[i];
    }
  }
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_TITLE);
}
void player_obj::do_calc_attr_affected_by_fa_bao()
{
  this->obj_attr_.clear_rate(ADD_T_FA_BAO);
  obj_attr::reset_attr_r_add();
  fa_bao_dj_cfg_obj *fbdco = fa_bao_dj_cfg::instance()->get_dj_info(this->char_extra_info_->fa_bao_dj_);
  if (fbdco == NULL) return ;

  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    obj_attr::attr_r_add[i] += fbdco->attr_rate_[i];

  int fa_bao_info[FA_BAO_PART_CNT + 1] = {0};
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  fa_bao_module::parse_fa_bao_value(this->fa_bao(), fa_bao_info);
  for (int i = 1; i <= FA_BAO_PART_CNT; ++i)
  {
    if (fa_bao_info[i] != 0)
    {
      fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao(fbdco->part_fa_bao_cid_[i]);
      if (fbco != NULL)
      {
        for (int j = ATTR_T_HP; j < ATTR_T_ITEM_CNT; ++j)
          obj_attr::attr_r_add[j] += fbco->attr_rate_[j];
      }
    }
  }

  this->obj_attr_.acc_r_attr(obj_attr::attr_r_add, ADD_T_FA_BAO);
}
int attr_module::do_calc_zhan_li(player_obj *player)
{
  int zhan_li = 0;
  zhan_li_calc_coe *zlcc = zhan_li_calc_cfg::instance()->get_zhan_li_calc_coe(player->career());
  if (zlcc != NULL)
  {
    for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
      zhan_li += (int)::ceil(obj_attr::attr_v_add[i] * zlcc->attr_coe_[i] / LVL_PARAM_COE_CHU_SHU);
  }
  
  //= skill 
  double total_skill_lvl = 0;
  skill_map_itor itor = player->skill_map_.begin();
  for (; itor != player->skill_map_.end(); ++itor)
  {
    int *init_skill_list = skill_module::get_common_skills(player->career());
    bool find = false;
    for (int i = 0; i < 64/*magic number*/; ++i)
    {
      if (init_skill_list[i] == 0) break;

      if (itor->second->cid_ == init_skill_list[i])
      {
        find = true;
        break;
      }
    }
    if (!find)
      total_skill_lvl += itor->second->lvl_;
  }
  zhan_li = (int)((double)zhan_li * (1.0 + total_skill_lvl * global_param_cfg::skill_fight_score_coe / 10000.0));

  return zhan_li;
}
void player_obj::on_attr_update(const int bt, const int src)
{
  obj_attr::reset_attr_v_add();
  this->obj_attr_.calc_zhan_li(obj_attr::attr_v_add);
  int zhan_li = attr_module::do_calc_zhan_li(this);
  if (this->char_info_->zhan_li_ != zhan_li)
  {
    if (bt != 0)
      behavior_log::instance()->store(BL_ZHAN_LI,
                                      time_util::now,
                                      "%d|%d|%d|%d|%d",
                                      this->id(),
                                      this->char_info_->zhan_li_,
                                      zhan_li,
                                      bt,
                                      src);
    this->char_info_->zhan_li_ = zhan_li;
    this->db_save_char_info();
    char_brief_info::on_char_zhan_li_update(this->id_, zhan_li);
    cheng_jiu_module::on_zhan_li_update(this, zhan_li);
    kai_fu_act_obj::on_zhan_li_update(this);
  }

  this->broadcast_hp_mp();
  this->do_notify_attr_to_clt();
}
void player_obj::do_notify_attr_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->obj_attr_.gong_ji()
    << this->obj_attr_.fang_yu()
    << this->obj_attr_.ming_zhong()
    << this->obj_attr_.shan_bi()
    << this->obj_attr_.bao_ji()
    << this->obj_attr_.kang_bao()
    << this->obj_attr_.shang_mian()
    << this->char_info_->zhan_li_;
  this->do_delivery(NTF_CHAR_ATTR_UPDATE, &mb);
}
