inline int char_obj::can_attack(char_obj *target)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CHAR_IS_DEAD;
  if (this->group() != 0 && this->group() == target->group())
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  return 0;
}
inline int char_obj::can_be_attacked(char_obj *attacker)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CHAR_IS_DEAD;
  if (this->group() != 0 && this->group() == attacker->group())
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  return 0;
}
inline int char_obj::can_move(const short x, const short y)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CHAR_IS_DEAD;
  if (this->has_buff(BF_SK_DING_SHEN)
      && (this->coord_x_ != x
          || this->coord_y_ != y)
     )
    return ERR_CAN_NOT_MOVE;
  if (this->has_buff(BF_SK_XUAN_YUN)
      || this->has_buff(BF_SK_SHANDIAN_FENGBAO))
    return ERR_CAN_NOT_MOVE;
  return 0;
}
inline int char_obj::can_be_moved()
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CHAR_IS_DEAD;
  if (this->has_buff(BF_SK_DING_SHEN)
      || this->has_buff(BF_SK_XUAN_YUN)
      || this->has_buff(BF_SK_XUAN_FENG)
      || this->has_buff(BF_SK_SHANDIAN_FENGBAO))
    return ERR_CAN_NOT_MOVE;
  return 0;
}
inline int char_obj::can_be_hurt(char_obj * /*attacker*/)
{
#if 0
  if (this->has_buff(buff_module::BF_SK_WUDI))
    return -1;
#endif
  return 0;
}
inline int char_obj::is_proper_attack_target(char_obj *defender)
{
  int ret = this->can_attack(defender);
  if (ret != 0) return ret;
  ret = defender->can_be_attacked(this);
  if (ret != 0) return ret;
  return 0;
}
inline int char_obj::is_proper_cure_target(char_obj *defender)
{
  int ret = this->can_cure(defender);
  if (ret != 0) return ret;
  ret = defender->can_be_cured(this);
  if (ret != 0) return ret;
  return 0;
}
inline void char_obj::do_build_snap_hp_mp_info(out_stream &os)
{
  os << T_HP_MP
    << this->obj_attr_.total_hp()
    << this->obj_attr_.hp_
    << this->obj_attr_.total_mp()
    << this->obj_attr_.mp_;
}
inline void char_obj::do_build_broadcast_position_msg(mblock &mb)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << this->dir_
    << this->coord_x_
    << this->coord_y_;
  ph->set(0, NTF_BROADCAST_OTHER_CHAR_MOVE, 0, mb.length());
}
inline void char_obj::do_build_broadcast_speed_msg(mblock &mb)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << this->move_speed();
  ph->set(0, NTF_BROADCAST_OTHER_CHAR_SPEED, 0, mb.length());
}
inline void char_obj::do_build_broadcast_hp_mp_msg(mblock &mb)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << this->obj_attr_.total_hp()
    << this->obj_attr_.hp_
    << this->obj_attr_.total_mp()
    << this->obj_attr_.mp_;
  ph->set(0, NTF_BROADCAST_HP_MP, 0, mb.length());
}
inline void char_obj::do_build_broadcast_use_skill(mblock &mb,
                                                   const int skill_cid,
                                                   const short skill_lvl,
                                                   const int target_id,
                                                   const short x,
                                                   const short y)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ 
    << skill_cid
    << skill_lvl
    << target_id
    << x << y;
  ph->set(0, NTF_BROADCAST_OTHER_USE_SKILL, 0, mb.length());
}
inline void char_obj::do_build_broadcast_be_hurt_effect_msg(mblock &mb,
                                                            const int skill_cid,
                                                            const int hurt,
                                                            const int tip,
                                                            const int attacker_id)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ 
    << skill_cid
    << hurt
    << (char)tip
    << attacker_id;
  ph->set(0, NTF_BROADCAST_BE_HURT_EFFECT, 0, mb.length());
}
inline void char_obj::do_build_broadcast_unit_life_status(mblock &mb,
                                                          const char st)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << st;
  ph->set(0, NTF_BROADCAST_UNIT_LIFE_STATUS, 0, mb.length());
}
inline void char_obj::do_build_broadcast_flash_move(mblock &mb,
                                                    const short target_x,
                                                    const short target_y)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << target_x 
    << target_y;
  ph->set(0, NTF_BROADCAST_FLASH_MOVE, 0, mb.length());
}
inline void char_obj::do_build_broadcast_ji_tui(mblock &mb,
                                                const int skill_cid,
                                                const short target_x,
                                                const short target_y)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << skill_cid
    << target_x 
    << target_y;
  ph->set(0, NTF_BROADCAST_JI_TUI, 0, mb.length());
}
inline void char_obj::do_build_broadcast_tianfu_skill(mblock &mb,
                                                      const int skill_cid)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << skill_cid;
  ph->set(0, NTF_BROADCAST_TIANFU_SKILL, 0, mb.length());
}
inline void char_obj::do_build_broadcast_add_buff_msg(mblock &mb,
                                                      const int buff_id)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << buff_id;
  ph->set(0, NTF_BROADCAST_ADD_BUFF, 0, mb.length()) ;
}
inline void char_obj::do_build_broadcast_del_buff_msg(mblock &mb,
                                                      const int buff_id)
{
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << buff_id;
  ph->set(0, NTF_BROADCAST_DEL_BUFF, 0, mb.length()) ;
}
inline int char_obj::do_add_hp(const int value)
{ return this->do_modify_energy(-value, MODIFY_HP); }
inline int char_obj::do_add_mp(const int value)
{ return this->do_modify_energy(-value, MODIFY_MP); }
inline int char_obj::do_reduce_hp(const int value)
{ return this->do_modify_energy(value, MODIFY_HP); }
inline int char_obj::do_reduce_mp(const int value)
{ return this->do_modify_energy(value, MODIFY_MP); }
inline bool char_obj::has_buff(const buff_effect_id_t buff_effect_id)
{ return this->buff_effect_cnt_[buff_effect_id] > 0; }
