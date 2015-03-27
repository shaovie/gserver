#include "buff_obj.h"
#include "buff_config.h"
#include "error.h"
#include "player_obj.h"
#include "char_obj.h"
#include "buff_info.h"
#include "fighter_mgr.h"
#include "sys_log.h"
#include "scene_mgr.h"

// Lib header
#include "macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("bf");
static ilog_obj *e_log = err_log::instance()->get_ilog("bf");

void buff_obj::init(char_obj *owner,
                    const int bf_id,
                    const buff_effect_id_t effect_id,
                    const int src_id,
                    const int buff_rel_cid,
                    const int left_value,
                    const int left_time,
                    const int interval,
                    const int value,
                    const time_value &now,
                    const int bits,
                    const int param)
{
  this->dj_cnt_         = 1;
  this->owner_          = owner;
  this->last_update_time_ = now;
  this->effect_         = effect_id;
  this->value_          = value;
  this->interval_       = interval;
  this->src_id_         = src_id;
  this->buff_rel_cid_        = buff_rel_cid;

  this->id_             = bf_id;
  this->left_value_     = left_value;
  this->left_time_      = left_time;
  this->bits_           = bits;
  this->any_param_      = param;
  this->param_1_        = 0;
  this->param_2_        = 0;
  this->param_3_        = 0;
  buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(bf_id);
  if (bco == NULL) return ;
  this->param_1_        = bco->param_1_;
  this->param_2_        = bco->param_2_;
  this->param_3_        = bco->param_3_;
  if (bco->died_clear_)
    SET_BITS(this->bits_, BF_BIT_DEAD_CLR);
  if (bco->offline_clear_)
    SET_BITS(this->bits_, BF_BIT_OFFLINE_CLR);
  if (bco->show_type_ == buff_config::SHOW_TO_SELF)
    SET_BITS(this->bits_, BF_BIT_SHOW_TO_SELF);
  else if (bco->show_type_ == buff_config::SHOW_TO_ALL)
    SET_BITS(this->bits_, BF_BIT_SHOW_TO_SELF|BF_BIT_SHOW_TO_OTHER);
}
//------------------- only_timing_buff ---------------------------
int only_timing_buff::do_dj(const int /*left_value*/, const int left_time)
{
  buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(this->id_);
  if (bco == NULL) return ERR_CONFIG_NOT_EXIST;

  if (this->dj_cnt_ >= bco->dj_cnt_) return ERR_CONFIG_NOT_EXIST;

  this->left_time_  += left_time / (this->dj_cnt_ * 2);
  ++(this->dj_cnt_);
  return 0;
}
int only_timing_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void only_timing_buff::release()
{ only_timing_buff_alloctor::instance()->release(this); }
//------------------- last area hurt buff ---------------------------
int last_area_hurt_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;

  if ((now - this->last_action_time_).msec() > (long)this->interval_)
  {
    this->last_action_time_ = now;

    int target_set[32] = {0};
    int ret = scene_mgr::instance()->get_scene_unit_list(this->owner_->scene_id(),
                                                         sizeof(target_set)/sizeof(target_set[0]),
                                                         this->owner_->attack_target(),
                                                         OBJ_DEAD,
                                                         this->owner_->coord_x(),
                                                         this->owner_->coord_y(),
                                                         this->param_3_,
                                                         target_set);
    if (ret <= 0) return 0;
    int sum_hurt = 0;
    for (int i = 0; i < ret; ++i)
    {
      if (this->owner_->id() == target_set[i]) continue;
      char_obj *target = fighter_mgr::instance()->find(target_set[i]);
      if (target == NULL
          || this->owner_->is_proper_attack_target(target) != 0)
        continue;

      if (target->can_be_hurt(this->owner_) == 0)
      {
        int tip = 0;
        int real_hurt = this->owner_->do_calc_hurt(tip, target);
        if (tip != SK_TIP_SHAN_BI && tip != SK_TIP_HU_DUN)
        {
          if (this->param_1_ > 0)
            real_hurt = real_hurt * this->param_1_ / 100;
          real_hurt += this->param_2_;
          sum_hurt += real_hurt;
          target->do_be_hurt(this->owner_, now, real_hurt, 0, this->buff_rel_cid_, 500);
        }
        target->broadcast_be_hurt_effect(this->buff_rel_cid_, real_hurt, tip, this->owner_->id());
      }
      target->on_be_attacked(this->owner_);
    }
    this->owner_->on_attack_somebody(0, 0, now, this->buff_rel_cid_, sum_hurt);
  }
  return 0;
}
void last_area_hurt_buff::release()
{ last_area_hurt_buff_alloctor::instance()->release(this); }
//------------------- last percent hurt buff ---------------------------
int last_percent_hurt_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;

  if ((now - this->last_action_time_).msec() > (long)this->interval_)
  {
    this->last_action_time_ = now;

    char_obj *src = fighter_mgr::instance()->find(this->src_id_);
    if (src == NULL) return -1;
    
    int real_hurt = this->any_param_ * this->value_ / 100;
    this->owner_->do_be_hurt(src, now, real_hurt, 0, this->buff_rel_cid_, 500);
    this->owner_->broadcast_be_hurt_effect(this->buff_rel_cid_, real_hurt, 0, src->id());
    this->owner_->on_be_attacked(src);
    src->on_attack_somebody(0, 0, now, this->buff_rel_cid_, real_hurt);
  }
  return 0;
}
void last_percent_hurt_buff::release()
{ last_percent_hurt_buff_alloctor::instance()->release(this); }
//------------------- jian su buff ---------------------------
void jian_su_buff::remove()
{
  if (BIT_ENABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
    this->do_update_attr(false);
  super::remove();
}
void jian_su_buff::do_update_attr(const bool be_init)
{
  int value = (int)(this->owner_->move_speed() * this->value_ / 100.0);
  if (!be_init)
    value = this->owner_->prev_move_speed();
  this->owner_->move_speed(value);
}
int jian_su_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  if (BIT_DISABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
  {
    SET_BITS(this->bits_, buff_obj::BF_BIT_ACTIVED);
    this->do_update_attr(true);
  }

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void jian_su_buff::release()
{ jian_su_buff_alloctor::instance()->release(this); }
//------------------- add attr percent buff ---------------------------
void add_all_attr_r_buff::remove()
{
  if (BIT_ENABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
    this->do_update_attr(false);
  super::remove();
}
void add_all_attr_r_buff::do_update_attr(const bool be_init)
{
  int value = this->value_;
  if (!be_init) value = -this->value_;

  obj_attr::reset_attr_r_add();
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    obj_attr::attr_r_add[i] += value;

  this->owner_->get_obj_attr().acc_r_attr(obj_attr::attr_r_add, ADD_T_BUFF);
  this->owner_->on_attr_update(0, 0);
}
int add_all_attr_r_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  if (BIT_DISABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
  {
    SET_BITS(this->bits_, buff_obj::BF_BIT_ACTIVED);
    this->do_update_attr(true);
  }

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void add_all_attr_r_buff::release()
{ add_all_attr_r_buff_alloctor::instance()->release(this); }
//------------------- add one attr percent buff ---------------------------
void add_attr_r_buff::remove()
{
  if (BIT_ENABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
    this->do_update_attr(false);
  super::remove();
}
void add_attr_r_buff::do_update_attr(const bool be_init)
{
  int value = this->value_;
  if (!be_init) value = -this->value_;

  obj_attr::reset_attr_r_add();
  obj_attr::attr_r_add[this->any_param_] += value;

  this->owner_->get_obj_attr().acc_r_attr(obj_attr::attr_r_add, ADD_T_BUFF);
  this->owner_->on_attr_update(0, 0);
}
int add_attr_r_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  if (BIT_DISABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
  {
    SET_BITS(this->bits_, buff_obj::BF_BIT_ACTIVED);
    this->do_update_attr(true);
  }

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void add_attr_r_buff::release()
{ add_attr_r_buff_alloctor::instance()->release(this); }
//------------------------------------------------------------------
void bian_shen_buff::remove()
{
  if (BIT_ENABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
    this->do_update_attr(false);
  super::remove();
}
void bian_shen_buff::do_update_attr(const bool be_init)
{
  int value = this->value_;
  if (!be_init) value = -this->value_;

  obj_attr::reset_attr_r_add();
  obj_attr::attr_r_add[ATTR_T_GONG_JI] += value;
  obj_attr::attr_r_add[ATTR_T_BAO_JI] += value;
  obj_attr::attr_r_add[ATTR_T_MING_ZHONG] += value;

  this->owner_->get_obj_attr().acc_r_attr(obj_attr::attr_r_add, ADD_T_BUFF);
  this->owner_->on_attr_update(0, 0);
}
int bian_shen_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  if (BIT_DISABLED(this->bits_, buff_obj::BF_BIT_ACTIVED))
  {
    SET_BITS(this->bits_, buff_obj::BF_BIT_ACTIVED);
    this->do_update_attr(true);
  }

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void bian_shen_buff::release()
{ bian_shen_buff_alloctor::instance()->release(this); }
//------------------- last add hp buff ---------------------------
int last_add_hp_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;

  if ((now - this->last_action_time_).msec() > (long)this->interval_)
  {
    this->last_action_time_ = now;

    this->owner_->do_add_hp(this->owner_->total_hp() * this->value_ / 100);
    this->owner_->broadcast_hp_mp();
  }
  return 0;
}
void last_add_hp_buff::release()
{ last_add_hp_buff_alloctor::instance()->release(this); }
//----------------------- consume_value_buff -----------------------
int consume_value_buff::run(const time_value &now)
{
  if (this->left_time_ <= 0) return -1;
  if (this->left_value_ <= 0) return -1;

  int diff = (int)(now - this->last_update_time_).msec();
  this->last_update_time_ = now;
  this->left_time_ -= diff;
  return 0;
}
void consume_value_buff::release()
{ consume_value_buff_alloctor::instance()->release(this); }
