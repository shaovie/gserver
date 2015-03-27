inline int monster_obj::can_be_attacked(char_obj *attacker)
{
  if (this->status_ != ST_ENTER_SCENE)
    return ERR_CAN_NOT_ATTACK_TARGET_2;
  if (this->do_status_ == DO_BACKING)
    return ERR_CAN_NOT_ATTACK_TARGET_2;
  return super::can_be_attacked(attacker);
}
