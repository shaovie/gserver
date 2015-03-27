#include "equip_module.h"
#include "player_obj.h"
#include "item_config.h"
#include "package_module.h"
#include "equip_score_cfg.h"
#include "all_char_info.h"
#include "item_extra_info_opt.h"
#include "lvl_param_cfg.h"
#include "suit_config.h"
#include "behavior_id.h"
#include "sys_log.h"
#include "message.h"
#include "error.h"
#include "strength_addition_cfg.h"
#include "attr_module.h"

// Lib header
#include <json/json.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("equip");
static ilog_obj *e_log = err_log::instance()->get_ilog("equip");

int player_obj::clt_takeon_equip(const char *msg, const int len)
{
  int equip_id = 0;

  in_stream is(msg, len);
  is >> equip_id;

  item_obj *equip_in_pkg = package_module::find_item(this, equip_id);
  if (equip_in_pkg == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  const equip_cfg_obj *eco = item_config::instance()->get_equip(equip_in_pkg->cid_);
  if (eco == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (equip_in_pkg->pkg_ != PKG_PACKAGE
      || eco->career_ != this->career()
      || eco->use_lvl_ > this->lvl())
    return this->send_respond_err(NTF_OPERATE_RESULT,
                                  ERR_CAN_NOT_TAKEUP_THIS_EQUIP);

  item_obj *equip_takeon = package_module::find_item_by_part(this, eco->part_);
  if (equip_takeon != NULL)
    package_module::do_move_item(this, equip_takeon, PKG_PACKAGE); 
  package_module::do_move_item(this, equip_in_pkg, PKG_EQUIP); 
  if (equip_in_pkg->bind_ == UNBIND_TYPE)
  {
    equip_in_pkg->bind_ = BIND_TYPE;
    int item_update_size = 0;
    g_item_update_info[item_update_size++] = package_module::UPD_BIND;
    package_module::on_item_update(this,
                                   UPD_ITEM,
                                   equip_in_pkg,
                                   g_item_update_info,
                                   item_update_size,
                                   0);
  }

  if (eco->part_ == PART_ZHU_WU
      || eco->part_ == PART_FU_WU)
  {
    char_brief_info::on_char_equip_update(this->id_, eco->part_, equip_in_pkg->cid_);
    this->broadcast_equip_fino();
  }

  this->do_calc_attr_affected_by_equip();
  this->on_attr_update(ZHAN_LI_EQUIP_TAKEON, equip_in_pkg->cid_);
  attr_module::on_all_qh_update(this);
  attr_module::on_all_equip_color_update(this);
  return 0;
}
int player_obj::clt_takedown_equip(const char *msg, const int len)
{
  int equip_id  = 0;

  in_stream is(msg, len);
  is >> equip_id;

  item_obj *equip = package_module::find_item(this, equip_id);
  if (equip == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  const equip_cfg_obj* eco = item_config::instance()->get_equip(equip->cid_);
  if (eco == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (package_module::would_be_full(this,
                                    PKG_PACKAGE,
                                    equip->cid_,
                                    equip->amount_,
                                    equip->bind_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  package_module::do_move_item(this, equip, PKG_PACKAGE); 

  if (eco->part_ == PART_ZHU_WU
      || eco->part_ == PART_FU_WU)
  {
    char_brief_info::on_char_equip_update(this->id_, eco->part_, 0);
    this->broadcast_equip_fino();
  }

  this->do_calc_attr_affected_by_equip();
  this->on_attr_update(ZHAN_LI_EQUIP_TAKEDOWN, equip->cid_);
  attr_module::on_all_qh_update(this);
  attr_module::on_all_equip_color_update(this);
  return 0;
}
int equip_module::do_calc_equip_score(item_obj *equip)
{
  const equip_cfg_obj* eco = item_config::instance()->get_equip(equip->cid_);
  if (eco == NULL) return 0;
  equip_score_coe *esc = equip_score_cfg::instance()->get_equip_score_coe(eco->career_);
  if (esc == NULL) return 0;

  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip, extra_json);

  int qh_lvl = 0;
  obj_attr::reset_attr_v_add();
  equip_module::do_calc_attr_affected_by_equip_qh_lvl(equip, eco, extra_json, qh_lvl);
  equip_module::do_calc_attr_affected_by_equip_xi_lian(extra_json);

  double equip_score = 0;
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    equip_score += obj_attr::attr_v_add[i] * esc->attr_coe_[i];
  return (int)::ceil(equip_score / LVL_PARAM_COE_CHU_SHU);
}
void equip_module::do_calc_attr_affected_by_equip_qh_lvl(item_obj *equip,
                                                         const equip_cfg_obj *eco,
                                                         Json::Value &extra_json,
                                                         int &qh_lvl)
{
  qh_lvl = item_extra_info_opt::get_qh_lvl(extra_json);
  double coe = 1.0;
  if (qh_lvl > 0)
  {
    double attr_param = lvl_param_cfg::instance()->attr_param(qh_lvl);
    coe = attr_param / LVL_PARAM_COE_CHU_SHU;
  }
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
  {
    if (eco->attr_val_[i] == 0) continue;
    double v = eco->attr_val_[i];
    v = eco->attr_val_[i] * equip->attr_float_coe_ / 100.0; // 基础值
    obj_attr::attr_v_add[i] += (int)::ceil(v * coe);
  }
}
void equip_module::do_calc_attr_affected_by_equip_xi_lian(Json::Value &extra_json)
{
  int cur_xl[MAX_XI_LIAN_ATTR_NUM][4] = {{0}};
  item_extra_info_opt::get_cur_xl(cur_xl, extra_json);

  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    if (cur_xl[i][1] == 0) break;
    if (cur_xl[i][0] >= ATTR_T_ITEM_CNT) continue;
    obj_attr::attr_v_add[cur_xl[i][0]] += cur_xl[i][2];
  }
}
void equip_module::do_calc_attr_affected_by_equip_suit(player_obj *player)
{
  static std::map<int/*suit_id*/, int/*count*/> s_suit_count_map; 

  s_suit_count_map.clear();

  for (int part = PART_ZHU_WU; part < PART_END; ++part)
  {
    item_obj *equip = package_module::find_item_by_part(player, part);
    if (equip == NULL) continue;
    const equip_cfg_obj *eco = item_config::instance()->get_equip(equip->cid_); 
    if (eco == NULL) continue;

    if (eco->suit_id_ != 0)
    {
      std::map<int, int>::iterator iter = s_suit_count_map.find(eco->suit_id_);
      if (iter == s_suit_count_map.end())
        s_suit_count_map[eco->suit_id_] = 1;
      else
        ++(iter->second);
    }
  }
  suit_attr_info *sai = NULL;
  for (std::map<int, int>::iterator iter = s_suit_count_map.begin();
       iter != s_suit_count_map.end();
       ++iter)
  {
    int suit_id = iter->first;
    int suit_count = iter->second;
    if (suit_count < 1) continue;
    sai = suit_config::instance()->get_suit_attr_info(suit_id, suit_count);
    if (sai == NULL)
    {
      --suit_count;
      sai = suit_config::instance()->get_suit_attr_info(suit_id, suit_count);
      if (sai == NULL) continue;
    }
    obj_attr::attr_v_add[ATTR_T_HP]         += sai->attr_val_[ATTR_T_HP];
    obj_attr::attr_v_add[ATTR_T_GONG_JI]    += sai->attr_val_[ATTR_T_GONG_JI];
    obj_attr::attr_v_add[ATTR_T_FANG_YU]    += sai->attr_val_[ATTR_T_FANG_YU];
    obj_attr::attr_v_add[ATTR_T_MING_ZHONG] += sai->attr_val_[ATTR_T_MING_ZHONG];
    obj_attr::attr_v_add[ATTR_T_SHAN_BI]    += sai->attr_val_[ATTR_T_SHAN_BI];
    obj_attr::attr_v_add[ATTR_T_BAO_JI]     += sai->attr_val_[ATTR_T_BAO_JI];
    obj_attr::attr_v_add[ATTR_T_KANG_BAO]   += sai->attr_val_[ATTR_T_KANG_BAO];
    obj_attr::attr_v_add[ATTR_T_SHANG_MIAN] += sai->attr_val_[ATTR_T_SHANG_MIAN];
  }
}
void equip_module::do_calc_attr_affected_by_all_qh(const int all_qh_lvl)
{
  sa_attr *saa = strength_addition_cfg::instance()->get_sa_attr(all_qh_lvl);
  if (saa == NULL) return;

  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    obj_attr::attr_v_add[i] += saa->attr_val_[i];
}
