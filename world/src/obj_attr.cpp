#include "obj_attr.h"

// Lib header
#include <math.h>

int obj_attr::attr_r_add[ATTR_T_ITEM_CNT];
int obj_attr::attr_v_add[ATTR_T_ITEM_CNT];

obj_attr::obj_attr() :
  hp_(0),
  mp_(0)
{
  for (int i = 0; i < ATTR_T_ITEM_CNT; ++i)
  {
    this->acc_add_v_[i].init();
    this->acc_add_r_[i].init();
    this->total_value_[i] = 0;
  }
}
void obj_attr::acc_v_attr(const int attr_add[ATTR_T_ITEM_CNT], const add_t t)
{
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    this->acc_add_v_[i].add(t, attr_add[i]);

  int old_hp = this->hp_;
  int old_total_hp = this->total_hp();
  int old_mp = this->mp_;
  int old_total_mp = this->total_mp();

  this->calc_attr(this->total_value_, CALC_ALL_ATTR);
  this->adjust_hp_mp(old_hp, old_total_hp, old_mp, old_total_mp);
}
void obj_attr::acc_r_attr(const int attr_add[ATTR_T_ITEM_CNT], const add_t t)
{
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
    this->acc_add_r_[i].add(t, attr_add[i]);

  int old_hp = this->hp_;
  int old_total_hp = this->total_hp();
  int old_mp = this->mp_;
  int old_total_mp = this->total_mp();

  this->calc_attr(this->total_value_, CALC_ALL_ATTR);
  this->adjust_hp_mp(old_hp, old_total_hp, old_mp, old_total_mp);
}
void obj_attr::calc_attr(int *result_value, const int ct)
{
  for (int i = ATTR_T_HP; i < ATTR_T_ITEM_CNT; ++i)
  {
    result_value[i] = (int)::ceil((double)this->acc_add_v_[i].total(ct) \
                                  * (double)this->acc_add_r_[i].total(ct) \
                                  / 10000.0);
  }
}

