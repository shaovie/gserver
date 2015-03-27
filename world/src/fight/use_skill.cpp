#include "char_obj.h"
#include "sys_log.h"
#include "skill_info.h"
#include "skill_config.h"
#include "error.h"
#include "fighter_mgr.h"
#include "buff_module.h"
#include "monster_mgr.h"
#include "spawn_monster.h"
#include "scene_config.h"
#include "scene_mgr.h"
#include "mblock_pool.h"
#include "monster_obj.h"
#include "effect_obj.h"
#include "effect_obj_mgr.h"

// Lib header
#include "macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("fight");
static ilog_obj *e_log = err_log::instance()->get_ilog("fight");
DIR_STEP(s_dir_step);

int char_obj::can_use_skill(const skill_detail *sd,
                            const skill_info *si,
                            const int block_radius,
                            const time_value &now,
                            const coord_t &target_pos)
{
  if (this->has_buff(BF_SK_DING_SHEN))
  {
    if (this->career() == CAREER_LI_LIANG)
    {
      if (si->cid_ != LI_LIANG_COMMON_SKILL_1
          && si->cid_ != LI_LIANG_COMMON_SKILL_2
          && si->cid_ != LI_LIANG_COMMON_SKILL_3)
        return ERR_CAN_NOT_USE_SKILL;
    }else if (this->career() == CAREER_MIN_JIE)
    {
      if (si->cid_ != MIN_JIE_COMMON_SKILL_1
          && si->cid_ != MIN_JIE_COMMON_SKILL_2
          && si->cid_ != MIN_JIE_COMMON_SKILL_3
          && si->cid_ != MIN_JIE_BIAN_SHEN_COMMON_SKILL)
        return ERR_CAN_NOT_USE_SKILL;
    }else if (this->career() == CAREER_ZHONG_LI)
    {
      if (si->cid_ != ZHI_LI_COMMON_SKILL_1 
          && si->cid_ != ZHI_LI_COMMON_SKILL_2
          && si->cid_ != ZHI_LI_COMMON_SKILL_3)
        return ERR_CAN_NOT_USE_SKILL;
    }
  }
  if (this->has_buff(BF_SK_XUAN_YUN)
      || this->has_buff(BF_SK_XUAN_FENG)
      || this->has_buff(BF_SK_SHANDIAN_FENGBAO))
    return ERR_CAN_NOT_USE_SKILL;

  if (sd->distance_ > 0
      && target_pos.x_ != 0
      && target_pos.y_ != 0
      && util::is_out_of_distance(this->coord_x_,
                                  this->coord_y_,
                                  target_pos.x_,
                                  target_pos.y_,
                                  sd->distance_ + this->block_radius_ + block_radius))
    return ERR_SKILL_OUT_OF_DISTANCE;

  if (sd->cd_ > 0)
  {
    if ((now - time_value(si->use_time_, si->use_usec_)).msec() < (long)sd->cd_)
      return ERR_SKILL_CD_LIMIT;
  }
  return 0;
}
int char_obj::do_use_skill(const int target_id,
                           const time_value &now,
                           const short target_x,
                           const short target_y,
                           const int skill_cid,
                           const int skill_hurt_delay,
                           const skill_detail *sd)
{
  int ret = this->dispatch_skill(target_id,
                                 now,
                                 target_x,
                                 target_y,
                                 skill_cid,
                                 skill_hurt_delay,
                                 sd);
  if (ret == 0)
    this->last_use_skill_time_ = now;
  return ret;
}
int char_obj::dispatch_skill(const int target_id,
                             const time_value &now,
                             const short target_x,
                             const short target_y,
                             const int skill_cid,
                             const int skill_hurt_delay,
                             const skill_detail *sd)

{
  int ret = 0;
  switch (skill_cid)
  {
  case 32000001: // 怪物    普通攻击
  case 32000002: // 怪物    普通攻击
  case 32000003: // 怪物    普通攻击
  case 32000004: // 怪物    普通攻击
  case 32000005: // 怪物    普通攻击
  case 32000006: // 怪物    普通攻击
  case 32000007: // 怪物    普通攻击
  case 32000008: // 怪物    普通攻击
  case 32000009: // 怪物    普通攻击
  case 32000010: // 怪物    普通攻击
  case 32000011: // 怪物    怪物攻击
  case 32000012: // 怪物    怪物攻击
  case 32000013: // 怪物    普通攻击
  case 32000014: // 怪物    普通攻击
    // 造成百分比伤害，附加固定额外伤害值，并附加BUF
    ret = this->sk_percent_hurt_and_add_fixed_hurt(target_id,
                                                   now,
                                                   skill_cid,
                                                   skill_hurt_delay,
                                                   sd,
                                                   target_x,
                                                   target_y);
    break;
  case 31110001: // 力量型  普通攻击1
  case 31110002: // 力量型  普通攻击2
  case 31110003: // 力量型  普通攻击3
  case 31210001: // 敏捷型  普通攻击1
  case 31210002: // 敏捷型  普通攻击2
  case 31210003: // 敏捷型  普通攻击3
  case 31310001: // 智力型  普通攻击1
  case 31310002: // 智力型  普通攻击2
  case 31310003: // 智力型  普通技能3
    // 对一定范围半径内的所有目标造成百分比伤害，附加固定额外伤害值, 并附加BUF
    ret = this->sk_area_percent_hurt_and_add_fixed_hurt(now,
                                                        skill_cid,
                                                        skill_hurt_delay,
                                                        sd,
                                                        target_id,
                                                        target_x,
                                                        target_y,
                                                        true);
    break;

  case 31110011: // 力量型  战斗怒吼
  case 32100005: // 怪物    死亡脉冲
  case 32100009: // 怪物    冰霜震击
    // 对一定范围半径内的所有目标造成百分比伤害，附加固定额外伤害值, 并附加BUF
    ret = this->sk_area_percent_hurt_and_add_fixed_hurt(now,
                                                        skill_cid,
                                                        skill_hurt_delay,
                                                        sd,
                                                        0,
                                                        this->coord_x_,
                                                        this->coord_y_,
                                                        false);
    break;
  case 31210004: // 敏捷型  变身普通攻击
    // 对一定范围半径内的所有目标造成百分比伤害，附加固定额外伤害值, 并附加BUF
    ret = this->sk_area_percent_hurt_and_add_fixed_hurt(now,
                                                        skill_cid,
                                                        skill_hurt_delay,
                                                        sd,
                                                        target_id,
                                                        target_x,
                                                        target_y,
                                                        false);
    break;
  case 31110013: // 力量型  剑刃风暴
  case 31210013: // 敏捷型  烈焰火环
  case 31310021: // 智力型  寒冰护体
  case 32100004: // 怪物    蝗虫群
  case 32100011: // 怪物    狂暴
  case 32100012: // 怪物    铁甲
  case 32100013: // 怪物    回春
  case 32100014: // 怪物    反弹
    ret = this->sk_insert_buff_to_self(now, skill_cid, sd);
    break;
  case 32100001: // 怪物    击晕
  case 32100002: // 怪物    减速
    ret = this->sk_insert_buff_to_target(now, skill_cid, sd, target_id);
    break;
  case 31310012: // 智力型  冰锥术
  case 31210011: // 敏捷型  破空之刃
    ret = this->sk_1_4_view_area_percent_hurt_and_add_fixed_hurt(now,
                                                                 skill_cid,
                                                                 skill_hurt_delay,
                                                                 sd);
    break;
  case 31110012: // 力量型  战神冲锋
  case 31210012: // 敏捷型  疾风猎刃
  case 31310011: // 智力型  龙破斩
  case 32100017: // 怪物    混沌陨石
  case 32100003: // 怪物    地刺
  case 32100006: // 怪物    腐蚀蜂群
  case 32100008: // 怪物    超声冲击波
    // 对直线范围内的所有目标造成百分比伤害，附加固定额外伤害值
    ret = this->sk_line_area_percent_hurt_and_add_fixed_hurt(now,
                                                             skill_cid,
                                                             skill_hurt_delay,
                                                             sd);
    break;
  case 31110021: // 力量型  镜像分身
    ret = this->sk_jing_xiang_fen_shen(skill_cid, skill_hurt_delay, sd);
    break;
  case 31210021: // 敏捷型  恶魔变身
    ret = this->sk_area_percent_hurt_and_add_fixed_hurt_and_insert_buff_to_self(now,
                                                                                skill_cid,
                                                                                skill_hurt_delay,
                                                                                sd,
                                                                                this->coord_x_,
                                                                                this->coord_y_);
    break;
  case 31310013: // 智力型  暴风雪
  case 32100007: // 怪物    闪电风暴
    ret = this->sk_place_a_last_hurt_effect_and_insert_buf_to_self(now, skill_cid, sd);
    break;
  case 32100019: // 怪物    爆发
    ret = this->sk_area_total_hp_percent_hurt(now,
                                              skill_cid,
                                              skill_hurt_delay,
                                              sd,
                                              this->coord_x_,
                                              this->coord_y_);
    break;
  default:
    e_log->wning("unknwon skill %d in %s", skill_cid, __func__);
    break;
  }
  return ret;
}
int char_obj::sk_percent_hurt_and_add_fixed_hurt(const int target_id,
                                                 const time_value &now,
                                                 const int skill_cid,
                                                 const int skill_hurt_delay,
                                                 const skill_detail *sd,
                                                 const short target_x,
                                                 const short target_y)
{
  if (target_id == 0) // 没有目标也可以释放
  {
    this->broadcast_use_skill(skill_cid, sd->cur_lvl_, target_id, target_x, target_y);
    return 0;
  }
  if (this->id_ == target_id) return ERR_SKILL_TARGET_ILLEGAL;
  char_obj *target = fighter_mgr::instance()->find(target_id);
  if (target == NULL) return ERR_SKILL_TARGET_NOT_EXIST;

  int ret = this->is_proper_attack_target(target);
  if (ret != 0) return ret;

  int real_hurt = 0;
  if (target->can_be_hurt(this) == 0)
  {
    int tip = 0;
    real_hurt = this->do_calc_hurt(tip, target);
    if (tip != SK_TIP_SHAN_BI && tip != SK_TIP_HU_DUN)
    {
      if (sd->hurt_percent_ > 0)
        real_hurt = real_hurt * sd->hurt_percent_ / 100;
      real_hurt += sd->add_fixed_hurt_;
      target->do_be_hurt(this, now, real_hurt, sd->hate_coe_, skill_cid, skill_hurt_delay);

      this->sk_do_insert_buff(target,
                              skill_cid,
                              real_hurt,
                              now,
                              sd->buff_info_,
                              SK_BUFF_TARGET_SINGLE,
                              0);
    }
    target->broadcast_be_hurt_effect(skill_cid, real_hurt, tip, this->id_);
  }
  target->do_ji_tui(this, now, sd->back_dis_, skill_cid);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, target_id, target->coord_x(), target->coord_y());
  target->on_be_attacked(this);
  this->on_attack_somebody(target_id, target->unit_type(), now, skill_cid, real_hurt);
  return 0;
}
int char_obj::sk_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                      const int skill_cid,
                                                      const int skill_hurt_delay,
                                                      const skill_detail *sd,
                                                      const int target_id,
                                                      const short target_x,
                                                      const short target_y,
                                                      const bool with_tian_fu)
{
  if (target_x <= 0 && target_y <= 0)
    return ERR_SKILL_TARGET_ILLEGAL;

  int target_set[64] = {0}; // should be OPTIMIZED
  int ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                       sizeof(target_set)/sizeof(target_set[0]),
                                                       this->attack_target(),
                                                       OBJ_DEAD,
                                                       target_x,
                                                       target_y,
                                                       sd->param_1_,
                                                       target_set);
  int sum_hurt = 0;
  int attacked_target_id = target_id;
  this->sk_do_area_percent_hurt_and_add_fixed_hurt(now,
                                                   ret,
                                                   target_set,
                                                   skill_cid,
                                                   skill_hurt_delay,
                                                   sd,
                                                   with_tian_fu,
                                                   attacked_target_id,
                                                   sum_hurt);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, target_x, target_y);
  this->on_attack_somebody(attacked_target_id, 0, now, skill_cid, sum_hurt);
  return 0;
}
void char_obj::sk_do_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                          const int target_cnt,
                                                          const int *target_set,
                                                          const int skill_cid,
                                                          const int skill_hurt_delay,
                                                          const skill_detail *sd,
                                                          const bool with_tian_fu,
                                                          int &attacked_target_id,
                                                          int &sum_hurt)
{
  for (int i = 0; i < target_cnt; ++i)
  {
    if (this->id_ == target_set[i]) continue;
    char_obj *target = fighter_mgr::instance()->find(target_set[i]);
    if (target == NULL
        || this->is_proper_attack_target(target) != 0)
      continue;

    if (target->can_be_hurt(this) == 0)
    {
      int tip = 0;
      int real_hurt = this->do_calc_hurt(tip, target);
      if (tip != SK_TIP_SHAN_BI && tip != SK_TIP_HU_DUN)
      {
        if (sd->hurt_percent_ > 0)
          real_hurt = real_hurt * sd->hurt_percent_ / 100;
        real_hurt += sd->add_fixed_hurt_;
        sum_hurt += real_hurt;
        target->do_be_hurt(this, now, real_hurt, sd->hate_coe_, skill_cid, skill_hurt_delay);

        this->sk_do_insert_buff(target,
                                skill_cid,
                                real_hurt,
                                now,
                                sd->buff_info_,
                                SK_BUFF_TARGET_AREA,
                                0);
        if (with_tian_fu)
          this->do_tianfu_effect(target, now);
      }
      if (attacked_target_id == 0)
        attacked_target_id = target_set[i];
      target->broadcast_be_hurt_effect(skill_cid, real_hurt, tip, this->id_);
    }
    target->do_ji_tui(this, now, sd->back_dis_, skill_cid);
    target->on_be_attacked(this);
  }
}
int char_obj::sk_insert_buff_to_self(const time_value &now,
                                     const int skill_cid,
                                     const skill_detail *sd)
{
  this->sk_do_insert_buff(this,
                          skill_cid,
                          0,
                          now,
                          sd->buff_info_,
                          SK_BUFF_TARGET_SELF,
                          0);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, 0, 0);
  return 0;
}
int char_obj::sk_insert_buff_to_target(const time_value &now,
                                       const int skill_cid,
                                       const skill_detail *sd,
                                       const int target_id)
{
  if (this->id_ == target_id) return ERR_SKILL_TARGET_ILLEGAL;
  char_obj *target = fighter_mgr::instance()->find(target_id);
  if (target == NULL) return ERR_SKILL_TARGET_NOT_EXIST;
  int ret = this->is_proper_attack_target(target);
  if (ret != 0) return ret;

  this->sk_do_insert_buff(target,
                          skill_cid,
                          0,
                          now,
                          sd->buff_info_,
                          SK_BUFF_TARGET_SINGLE,
                          0);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, 0, 0);
  target->on_be_attacked(this);
  this->on_attack_somebody(target_id, target->unit_type(), now, skill_cid, 0);
  return 0;
}
int char_obj::sk_jing_xiang_fen_shen(const int skill_cid,
                                     const int skill_hurt_delay,
                                     const skill_detail *sd)
{
  if (sd->param_1_ > 3) return ERR_CONFIG_NOT_EXIST;

  coord_t pos_list[3];
  const int dist = 3;
  if (sd->param_1_ == 1)
  {
    pos_list[0].x_ = this->coord_x_ - dist;
    pos_list[0].y_ = this->coord_y_;
  }else if (sd->param_1_ == 2)
  {
    pos_list[0].x_ = this->coord_x_ - dist;
    pos_list[0].y_ = this->coord_y_;
    pos_list[1].x_ = this->coord_x_ + dist;
    pos_list[1].y_ = this->coord_y_;
  }else if (sd->param_1_ == 3)
  {
    pos_list[0].x_ = this->coord_x_ - dist;
    pos_list[0].y_ = this->coord_y_;
    pos_list[1].x_ = this->coord_x_ + dist;
    pos_list[1].y_ = this->coord_y_;
    pos_list[2].x_ = this->coord_x_;
    pos_list[2].y_ = this->coord_y_ + dist;
  }
  for (int i = 0; i < sd->param_1_; ++i)
  {
    if (!scene_config::instance()->can_move(this->scene_cid_,
                                            pos_list[i].x_,
                                            pos_list[i].y_))
    {
      pos_list[i] = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                             this->coord_x_,
                                                             this->coord_y_,
                                                             dist + 1);
    }
    int mst_id = spawn_monster::spawn_one(sd->param_3_,
                                          skill_hurt_delay,
                                          this->scene_id_,
                                          this->scene_cid_,
                                          this->dir_,
                                          pos_list[i].x_,
                                          pos_list[i].y_);
    if (mst_id != -1)
    {
      monster_obj *mo = monster_mgr::instance()->find(mst_id);
      if (mo != NULL)
      {
        mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*12);
        *mb << this->id()
          << this->pk_mode()
          << this->team_id()
          << this->guild_id()
          << this->lvl()
          << this->total_hp()
          << this->obj_attr_.gong_ji() * sd->param_2_ / 100
          << this->obj_attr_.fang_yu()
          << this->obj_attr_.ming_zhong()
          << this->obj_attr_.shan_bi()
          << this->obj_attr_.bao_ji()
          << this->obj_attr_.kang_bao()
          << this->obj_attr_.shang_mian();
        mo->post_aev(AEV_ZHUZAI_FEN_SHEN_INFO, mb);

        mblock *mb2 = mblock_pool::instance()->alloc(sizeof(int));
        *mb2 << mst_id;
        this->post_aev(AEV_ZHUZAI_FEN_SHEN_INFO, mb2);
      }
    }
  }
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, 0, 0);
  return 0;
}
int char_obj::sk_line_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                           const int skill_cid,
                                                           const int skill_hurt_delay,
                                                           const skill_detail *sd)
{
  short target_x = this->coord_x_;
  short target_y = this->coord_y_;
  for (int i = 0; i < sd->param_1_; ++i)
  {
    target_x += s_dir_step[(int)this->dir_][0];
    target_y += s_dir_step[(int)this->dir_][1];
    if (!scene_config::instance()->can_move(this->scene_cid_,
                                            target_x,
                                            target_y))
      break;
  }

  int target_set[64] = {0}; // should be OPTIMIZED
  int ret = scene_mgr::instance()->get_line_area_obj_set(this->scene_id_,
                                                         sizeof(target_set)/sizeof(target_set[0]),
                                                         this->attack_target(),
                                                         OBJ_DEAD,
                                                         this->coord_x_,
                                                         this->coord_y_,
                                                         this->dir_,
                                                         target_x,
                                                         target_y,
                                                         sd->param_2_,
                                                         target_set);
  int sum_hurt = 0;
  int attacked_target_id = 0;
  this->sk_do_area_percent_hurt_and_add_fixed_hurt(now,
                                                   ret,
                                                   target_set,
                                                   skill_cid,
                                                   skill_hurt_delay,
                                                   sd,
                                                   false,
                                                   attacked_target_id,
                                                   sum_hurt);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, target_x, target_y);
  this->on_attack_somebody(attacked_target_id, 0, now, skill_cid, sum_hurt);
  return 0;
}
int char_obj::sk_1_4_view_area_percent_hurt_and_add_fixed_hurt(const time_value &now,
                                                               const int skill_cid,
                                                               const int skill_hurt_delay,
                                                               const skill_detail *sd)
{
  int target_set[64] = {0}; // should be OPTIMIZED
  int ret = scene_mgr::instance()->get_1_4_view_obj_set(this->scene_id_,
                                                        sizeof(target_set)/sizeof(target_set[0]),
                                                        this->attack_target(),
                                                        OBJ_DEAD,
                                                        this->coord_x_,
                                                        this->coord_y_,
                                                        this->dir_,
                                                        sd->param_1_,
                                                        target_set);
  int sum_hurt = 0;
  int attacked_target_id = 0;
  this->sk_do_area_percent_hurt_and_add_fixed_hurt(now,
                                                   ret,
                                                   target_set,
                                                   skill_cid,
                                                   skill_hurt_delay,
                                                   sd,
                                                   false,
                                                   attacked_target_id,
                                                   sum_hurt);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, 0, 0);
  this->on_attack_somebody(attacked_target_id, 0, now, skill_cid, sum_hurt);
  return 0;
}
int char_obj::sk_area_percent_hurt_and_add_fixed_hurt_and_insert_buff_to_self(const time_value &now,
                                                                              const int skill_cid,
                                                                              const int skill_hurt_delay,
                                                                              const skill_detail *sd,
                                                                              const short target_x,
                                                                              const short target_y)
{
  int target_set[64] = {0}; // should be OPTIMIZED
  int ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                       sizeof(target_set)/sizeof(target_set[0]),
                                                       this->attack_target(),
                                                       OBJ_DEAD,
                                                       target_x,
                                                       target_y,
                                                       sd->param_1_,
                                                       target_set);
  int sum_hurt = 0;
  int attacked_target_id = 0;
  this->sk_do_area_percent_hurt_and_add_fixed_hurt(now,
                                                   ret,
                                                   target_set,
                                                   skill_cid,
                                                   skill_hurt_delay,
                                                   sd,
                                                   false,
                                                   attacked_target_id,
                                                   sum_hurt);
  this->sk_do_insert_buff(this,
                          skill_cid,
                          0,
                          now,
                          sd->buff_info_,
                          SK_BUFF_TARGET_SELF,
                          0);
  if (skill_cid == 31210021) // 恶魔变身
    buff_module::do_remove_all_debuff(this);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, target_x, target_y);
  this->on_attack_somebody(attacked_target_id, 0, now, skill_cid, sum_hurt);
  return 0;
}
int char_obj::sk_place_a_last_hurt_effect_and_insert_buf_to_self(const time_value &now,
                                                                 const int skill_cid,
                                                                 const skill_detail *sd)
{
  sk_last_hurt_effect_obj *seo = sk_last_hurt_effect_obj_alloctor::instance()->alloc();

  seo->set(now,
           this->id(),
           skill_cid,
           sd->cur_lvl_,
           this->scene_cid_,
           this->scene_id_,
           this->coord_x_,
           this->coord_y_,
           sd->param_3_);
  if (seo->do_enter_scene() != 0)
  {
    seo->release();
    return 0;
  }
  effect_obj_mgr::instance()->insert(seo);
  this->sk_do_insert_buff(this,
                          skill_cid,
                          0,
                          now,
                          sd->buff_info_,
                          SK_BUFF_TARGET_SELF,
                          0);
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, this->coord_x_, this->coord_y_);
  return 0;
}
int char_obj::sk_area_total_hp_percent_hurt(const time_value &now,
                                            const int skill_cid,
                                            const int skill_hurt_delay,
                                            const skill_detail *sd,
                                            const short target_x,
                                            const short target_y)
{
  int target_set[64] = {0}; // should be OPTIMIZED
  int ret = scene_mgr::instance()->get_scene_unit_list(this->scene_id_,
                                                       sizeof(target_set)/sizeof(target_set[0]),
                                                       this->attack_target(),
                                                       OBJ_DEAD,
                                                       target_x,
                                                       target_y,
                                                       sd->param_1_,
                                                       target_set);
  int sum_hurt = 0;
  int attacked_target_id = 0;
  for (int i = 0; i < ret; ++i)
  {
    if (this->id_ == target_set[i]) continue;
    char_obj *target = fighter_mgr::instance()->find(target_set[i]);
    if (target == NULL
        || this->is_proper_attack_target(target) != 0)
      continue;

    if (target->can_be_hurt(this) == 0)
    {
      int tip = 0;
      int real_hurt = target->total_hp() * sd->param_2_ / 100;
      target->do_be_hurt(this, now, real_hurt, sd->hate_coe_, skill_cid, skill_hurt_delay);
      if (attacked_target_id == 0)
        attacked_target_id = target_set[i];
      target->broadcast_be_hurt_effect(skill_cid, real_hurt, tip, this->id_);
    }
    target->do_ji_tui(this, now, sd->back_dis_, skill_cid);
    target->on_be_attacked(this);
  }
  this->broadcast_use_skill(skill_cid, sd->cur_lvl_, 0, target_x, target_y);
  this->on_attack_somebody(attacked_target_id, 0, now, skill_cid, sum_hurt);
  return 0;
}
