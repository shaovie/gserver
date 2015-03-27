inline int movable_monster_obj::can_move(const short x, const short y)
{
  if (this->move_speed() == 0)
    return ERR_CAN_NOT_MOVE;
  return super::can_move(x, y);
}
inline int movable_monster_obj::can_be_moved()
{
  if (this->move_speed() == 0)
    return ERR_CAN_NOT_BE_MOVED;
  return super::can_be_moved();
}
inline void movable_monster_obj::do_move_i(const time_value &now)
{
  coord_t next_pos = this->path_.pop_front();
  this->do_move_i(now, next_pos.x_, next_pos.y_);
}
inline void movable_monster_obj::do_move_i(const time_value &now,
                                           const short x,
                                           const short y)
{
  char dir = util::calc_next_dir(this->coord_x_,
                                 this->coord_y_,
                                 x,
                                 y);
  long int move_time = util::calc_move_time(dir, this->move_speed());
  this->reach_pos_time_ = now + time_value(0, move_time * 1000);

  if (this->do_move(x, y, dir) == 0)
    this->broadcast_position();
}
inline void movable_monster_obj::to_patrol()
{
  this->do_status_ = DO_PATROLING;
  this->path_.clear();
  this->do_calc_next_patrol_time();
  if (this->move_speed() != this->patrol_speed_)
    this->move_speed(this->patrol_speed_);
}
inline void movable_monster_obj::do_calc_next_patrol_time()
{
  int sec = this->patrol_interval_;
  if (sec <= 0) sec = 40;
  this->next_patrol_time_ = time_value(time_util::now + rand() % sec,
                                       rand() % 1000 * 1000);
}
inline int movable_monster_obj::do_flash_move(const short x, const short y, const char dir)
{
  if (this->do_move(x, y, dir) != 0) return -1;

  this->broadcast_position();
  return 0;
}
inline void movable_monster_obj::move_speed(const short v)
{
  this->prev_move_speed_ = this->move_speed_;
  this->move_speed_ = v;
  this->broadcast_speed();
}
