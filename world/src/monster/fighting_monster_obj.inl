inline void fighting_monster_obj::to_attack()
{
  this->do_status_ = DO_ATTACKING;
}
inline void fighting_monster_obj::to_chase()
{
  this->do_status_ = DO_CHASING;
  if (this->move_speed() != this->chase_speed_)
    this->move_speed(this->chase_speed_);
}
inline void fighting_monster_obj::do_off_war(const time_value &now)
{
  this->to_back(now);
}
inline int fighting_monster_obj::can_use_skill_i(const time_value &now)
{
  if ((now - this->last_use_skill_time_).msec() < this->common_cd_)
    return ERR_SKILL_CD_LIMIT;
  if (now < this->reach_pos_time_
      || now < this->be_hurt_done_time_
      || now < this->attack_done_time_)
    return ERR_CAN_NOT_USE_SKILL;
  return 0;
}
inline void fighting_monster_obj::on_use_skill_ok(skill_info *si,
                                                  const skill_detail *,
                                                  const int skill_hurt_delay,
                                                  const time_value &now)
{
  if (si != NULL)
  {
    si->use_time_ = (int)now.sec();
    si->use_usec_ = (int)now.usec();
  }

  int delay = skill_hurt_delay + 500/*被攻击者受伤时的动作播放时间，包括人+怪*/;
  this->attack_done_time_ = time_value(now.sec(), now.usec() + delay * 1000);
}
inline void fighting_monster_obj::on_chase_failed()
{
  this->to_chase();
}
