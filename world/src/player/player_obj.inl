inline bool player_obj::can_ke_zhi(char_obj *defender)
{
  if (defender->career() == CAREER_ZHONG_LI)
    return false;
  if (this->char_info_->career_ == CAREER_LI_LIANG
      && defender->career() == CAREER_MIN_JIE)
    return true;
  if (this->char_info_->career_ == CAREER_MIN_JIE
      && defender->career() == CAREER_ZHI_LI)
    return true;
  if (this->char_info_->career_ == CAREER_ZHI_LI
      && defender->career() == CAREER_LI_LIANG)
    return true;
  return false;
}
inline bool player_obj::can_be_ke_zhi(char_obj *defender)
{
  if (defender->career() == CAREER_ZHONG_LI)
    return false;
  if (this->char_info_->career_ == CAREER_LI_LIANG
      && defender->career() == CAREER_ZHI_LI)
    return true;
  if (this->char_info_->career_ == CAREER_MIN_JIE
      && defender->career() == CAREER_LI_LIANG)
    return true;
  if (this->char_info_->career_ == CAREER_ZHI_LI
      && defender->career() == CAREER_MIN_JIE)
    return true;
  return false;
}
inline void player_obj::move_speed(const short v)
{
  this->prev_move_speed_ = this->move_speed_;
  this->move_speed_ = v;
  this->broadcast_speed();
}
