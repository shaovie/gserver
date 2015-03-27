#include "char_obj.h"
#include "fighter_mgr.h"
#include "scene_mgr.h"
#include "client.h"
#include "message.h"
#include "error.h"
#include "def.h"
#include "mblock_pool.h"
#include "global_param_cfg.h"
#include "dropped_item.h"
#include "dropped_item_mgr.h"
#include "global_param_cfg.h"
#include "tianfu_skill_module.h"
#include "skill_info.h"
#include "buff_module.h"
#include "buff_obj.h"
#include "sys_log.h"
#include "fa_bao_module.h"

// Lib header
#include "mblock.h"
#include "macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("char");
static ilog_obj *e_log = err_log::instance()->get_ilog("char");

char_obj::char_obj() :
  fa_bao_attr_(NULL),
  buff_time_map_(NULL)
{
  ::memset(this->buff_effect_cnt_, 0, sizeof(this->buff_effect_cnt_));
  buff_module::init(this);
}
char_obj::~char_obj()
{
  buff_module::destroy(this);
  tianfu_skill_module::destroy(this);
  fa_bao_module::destroy(this);
}
int char_obj::do_enter_scene()
{
  if (scene_unit::do_enter_scene() != 0) return -1;
  fighter_mgr::instance()->insert(this->id(), this);
  return 0;
}
int char_obj::do_exit_scene()
{
  fighter_mgr::instance()->remove(this->id());
  scene_unit::do_exit_scene();
  return 0;
}
void char_obj::broadcast_position()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_position_msg(mb);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_speed()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_speed_msg(mb);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_hp_mp()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_hp_mp_msg(mb);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_use_skill(const int skill_cid,
                                   const short skill_lvl,
                                   const int target_id,
                                   const short x,
                                   const short y)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_use_skill(mb,
                                     skill_cid,
                                     skill_lvl,
                                     target_id,
                                     x,
                                     y);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_be_hurt_effect(const int skill_cid,
                                        const int hurt,
                                        const int tip,
                                        const int attacker_id)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_be_hurt_effect_msg(mb,
                                              skill_cid,
                                              hurt,
                                              tip,
                                              attacker_id);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_unit_life_status(const char st)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_unit_life_status(mb, st);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_flash_move(const short target_x,
                                    const short target_y)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_flash_move(mb,
                                      target_x,
                                      target_y);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_ji_tui(const int skill_cid,
                                const short target_x,
                                const short target_y)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_ji_tui(mb,
                                  skill_cid,
                                  target_x,
                                  target_y);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_tianfu_skill(const int skill_cid)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_tianfu_skill(mb, skill_cid);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   true,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
int char_obj::do_move(const short x, const short y, const char dir)
{
  if (scene_mgr::instance()->change_coord(this,
                                          this->scene_id_,
                                          this->coord_x_,
                                          this->coord_y_,
                                          x,
                                          y) != 0)
    return -1;

  this->dir_     = dir;
  this->coord_x_ = x;
  this->coord_y_ = y;
  return 0;
}
int char_obj::do_modify_energy(const int value, const int type)
{
  if (value == 0) return 0;

  int value_mod  = value;
  int *value_ptr = NULL;
  int value_max  = 0;

  //
  if (type == MODIFY_HP)
  {
    value_ptr = &(this->obj_attr_.hp_);
    value_max = this->total_hp();
  }else if (type == MODIFY_MP)
  {
    value_ptr = &(this->obj_attr_.mp_);
    value_max = this->total_mp();
  }else
    return 0;
  //
  if (*value_ptr <= 0 && value > 0)
    return 0;

  // calculate reduce value
  if (*value_ptr <= value_mod)
    value_mod = *value_ptr;

  // calculate increase value
  if (*value_ptr - value_mod > value_max)
    value_mod = *value_ptr - value_max;

  // modify current energy
  *value_ptr -= value_mod;

  return value_mod;
}
int char_obj::do_calc_hurt(int &attacked_tip, char_obj *defender)
{
  int hurt = this->do_calc_hurt_by_attr(attacked_tip, defender);
  if (hurt > 0)
  {
    hurt = this->do_calc_hurt_by_other_effect_for_attacker(hurt);
    if (hurt > 0)
      hurt = defender->do_calc_hurt_by_other_effect_for_defender(attacked_tip, hurt);
  }
  return hurt > 0 ? hurt : 1;
}
int char_obj::do_calc_hurt_by_attr(int &attacked_tip, char_obj *defender)
{
  //= 计算出攻击者等级系数和防御者等级系数
  double attacker_lvl = this->lvl();
  double defender_lvl = defender->lvl();
  double attacker_lvl_coe = attacker_lvl / (attacker_lvl + defender_lvl);
  double defender_lvl_coe = defender_lvl / (defender_lvl + attacker_lvl);

  //= 计算闪避
  int shan_bi = (int)(10000.0 \
                      * defender->obj_attr_.shan_bi() \
                      / (defender->obj_attr_.shan_bi() + this->obj_attr_.ming_zhong() + 1.0) \
                      * defender_lvl_coe * defender_lvl_coe);
  if (shan_bi > 5000) shan_bi = 5000;
  if (BIT_ENABLED(this->unit_type(), scene_unit::MONSTER))
    shan_bi = global_param_cfg::mst_shanbi_per;

  int shan_bi_r = rand() % 10000 + 1;
  if (shan_bi_r < shan_bi)
  {
    attacked_tip = SK_TIP_SHAN_BI;
    return 0;
  }

  //= 计算伤害值
  double attacker_gong_ji = this->obj_attr_.gong_ji(); 
  double defender_fang_yu = defender->obj_attr_.fang_yu(); 
  double hurt = attacker_gong_ji / (attacker_gong_ji + defender_fang_yu) * attacker_gong_ji;
  int hurt_coe_r = rand() % global_param_cfg::hurt_base_area + global_param_cfg::hurt_base_val;
  hurt = hurt * hurt_coe_r / 100.0;

  //= 计算职业相克
  if (this->can_ke_zhi(defender))
    hurt = hurt * (1.0 + global_param_cfg::xiangke_param / 1000.0);
  else if (this->can_be_ke_zhi(defender))
    hurt = hurt * (1.0 - global_param_cfg::xiangke_param / 1000.0);

  //= 计算暴击率与暴击伤害
  double attacker_bao_ji = this->obj_attr_.bao_ji();
  double defender_kang_bao = defender->obj_attr_.kang_bao();
  int bao_ji = (int)(10000.0 \
                     * attacker_bao_ji / (attacker_bao_ji + defender_kang_bao + 1.0) \
                     * attacker_lvl_coe * attacker_lvl_coe);
  if (bao_ji > 5000) bao_ji = 5000;
  if (BIT_ENABLED(defender->unit_type(), scene_unit::MONSTER))
    bao_ji = global_param_cfg::mst_baoji_per;
  int bao_ji_r = rand() % 10000 + 1;
  if (bao_ji_r < bao_ji)
  {
    attacked_tip = SK_TIP_BAO_JI;
    int bao_ji_val = (int)(attacker_bao_ji / (attacker_bao_ji + defender_kang_bao + 1.0)
                           * attacker_bao_ji);
    hurt = hurt * 1.5 + bao_ji_val;
  }

  //= 计算伤免
  double defender_shang_mian = defender->obj_attr_.shang_mian();
  double shang_mian_coe = defender_shang_mian / (defender_shang_mian + hurt + 1.0) \
                          * defender_lvl_coe * defender_lvl_coe;
  if (shang_mian_coe > 0.5) shang_mian_coe = 0.5;
  hurt = hurt * (1.0 - shang_mian_coe);

  return int(hurt);
}
int char_obj::do_calc_hurt_by_other_effect_for_attacker(const int hurt)
{
  int real_hurt = fa_bao_module::do_calc_hurt_for_attacker(this, hurt);
  return real_hurt;
}
int char_obj::do_calc_hurt_by_other_effect_for_defender(int &attacked_tip, const int hurt)
{
  int real_hurt = fa_bao_module::do_calc_hurt_for_defender(this, hurt);
  if (this->has_buff(BF_SK_XU_RUO))
  {
    buff_obj *bo = this->get_buff_by_effect(BF_SK_XU_RUO);
    if (bo != NULL)
      real_hurt += real_hurt * bo->value_ / 100;
  }
  if (this->has_buff(BF_SK_TIE_JIA))
  {
    buff_obj *bo = this->get_buff_by_effect(BF_SK_TIE_JIA);
    if (bo != NULL)
      real_hurt = real_hurt * bo->value_ / 100;
  }
  if (this->has_buff(BF_SK_HU_DUN))
  {
    buff_obj *bo = this->get_buff_by_effect(BF_SK_HU_DUN);
    if (bo != NULL && bo->value_ > 0)
    {
      bo->left_value_ -= real_hurt;
      attacked_tip = SK_TIP_HU_DUN;
    }
  }
  return real_hurt;
}
int char_obj::do_be_hurt(char_obj *attacker,
                         const time_value &now,
                         const int hurt,
                         const int hate_coe,
                         const int skill_cid,
                         const int skill_hurt_delay)
{
  if (hurt <= 0) return 0;

#if 0 // 前端为了处理伤害延时
  if (this->do_reduce_hp(hurt) != 0)
    this->broadcast_hp_mp();
#else
  this->do_reduce_hp(hurt);
#endif

  if (this->has_buff(BF_SK_FAN_TAN))
  {
    buff_obj *bo = this->get_buff_by_effect(BF_SK_FAN_TAN);
    int fan_tan_hurt = 0;
    if (bo != NULL)
       fan_tan_hurt = hurt * bo->value_ / 100;
    attacker->do_reduce_hp(fan_tan_hurt);
    attacker->broadcast_be_hurt_effect(skill_cid, fan_tan_hurt, SK_TIP_FAN_TAN, this->id_);
  }
  int real_hate_val = (int)::ceil((hurt * global_param_cfg::hate_val_coe / 1000.0)
                                  * (hate_coe / 1000.0));
  this->on_be_hurt(attacker, now, real_hate_val, skill_hurt_delay);
  return 0;
}
void char_obj::on_be_attacked(char_obj *attacker)
{
  (void)attacker;
}
void char_obj::on_be_hurt(char_obj *hurter,
                          const time_value &,
                          const int /*hate_val*/,
                          const int )
{
  this->do_check_hp(hurter);
}
void char_obj::do_check_hp(char_obj *hurter)
{
  if (this->obj_attr_.hp_ > 0) return ;
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
  *mb << hurter->id() << hurter->cid();
  this->post_aev(AEV_BE_KILLED, mb);
  this->add_status(OBJ_DEAD);
}
void char_obj::do_drop_items_on_scene(ilist<dropped_item *> &drop_list,
                                      const int drop_area_radius)
{
  if (drop_list.empty()) return ;
  coord_t empty_pos_set[128];
  int ret = scene_mgr::instance()->get_scene_empty_pos_list(this,
                                                            this->scene_id_,
                                                            sizeof(empty_pos_set)/sizeof(empty_pos_set[0]),
                                                            drop_area_radius,
                                                            empty_pos_set);
  while (!drop_list.empty())
  {
    dropped_item *di = drop_list.pop_front();
    bool ok = false;
    for (int i = 0; i < ret; ++i)
    {
      int idx = rand() % ret;
      if (empty_pos_set[idx].x_ == 0) continue;
      di->reset_pos(this->scene_id_,
                    this->scene_cid_,
                    empty_pos_set[idx].x_,
                    empty_pos_set[idx].y_);
      empty_pos_set[idx].x_ = 0;
      di->do_enter_scene();
      dropped_item_mgr::instance()->insert(di->id(), di);
      ok = true;
      break;
    }
    if (!ok)
      dropped_item_pool::instance()->release(di);
  }
}
