inline int base_attr::total(const int t)
{
  if (t == CALC_ALL_ATTR) return this->total_all();
  if (t == CALC_ZHAN_LI) return this->total_zhan_li();
  return 0;
}
inline int base_attr::total_all()
{
  int tv = 0;
  for (int i = 0; i < ADD_T_ITEM_CNT; ++i)
    tv += this->attr_value_[i];
  return tv;
}
inline int base_attr::total_zhan_li()
{
  int tv = 0;
  for (int i = 0; i < ADD_T_ITEM_CNT; ++i)
  {
    if (i != ADD_T_BUFF)
      tv += this->attr_value_[i];
  }
  return tv;
}
inline void obj_attr::clear_value(const add_t t)
{
  for (int i = 0; i < ATTR_T_ITEM_CNT; ++i)
    this->acc_add_v_[i].clear(t);
}
inline void obj_attr::clear_rate(const add_t t)
{
  for (int i = 0; i < ATTR_T_ITEM_CNT; ++i)
    this->acc_add_r_[i].clear(t);
}
inline void obj_attr::adjust_hp_mp(const int old_hp,
                                   const int old_total_hp,
                                   const int old_mp,
                                   const int old_total_mp)
{
  if (old_total_hp > 0)
  {
    double hp_r = old_hp * 100.0 / old_total_hp;
    if (hp_r > 0.0)
      this->hp_ = (int)(this->total_hp() * hp_r / 100.0);
  }
  if (old_total_mp > 0)
  {
    double mp_r = old_mp * 100.0 / old_total_mp;
    if (mp_r > 0.0)
      this->mp_ = (int)(this->total_mp() * mp_r / 100.0);
  }
}
