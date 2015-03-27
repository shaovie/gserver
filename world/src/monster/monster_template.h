// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-01-02 18:49
 */
//========================================================================

#ifndef MONSTER_TEMPLATE_H_
#define MONSTER_TEMPLATE_H_

#include "fighting_monster_obj.h"

// Forward declarations

/**
 * @class unmovable_monster_obj
 *
 * @brief
 */
class unmovable_monster_obj : public monster_obj
{
  typedef monster_obj super;
public:
  unmovable_monster_obj() { }
  virtual ~unmovable_monster_obj() { }

  virtual short move_speed() { return 0; }
  virtual int can_move(const short, const short) { return ERR_CAN_NOT_MOVE; }
  virtual int can_be_moved() { return ERR_CAN_NOT_BE_MOVED; }
};
/**
 * @class fighting_unmovable_monster
 *
 * @brief
 */
class fighting_unmovable_monster : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  virtual short move_speed() { return 0; }
  virtual int can_move(const short, const short) { return ERR_CAN_NOT_MOVE; }
  virtual int can_be_moved() { return ERR_CAN_NOT_BE_MOVED; }
  virtual void on_chase_failed() { this->to_back(time_value::gettimeofday()); }
};

#endif // MONSTER_TEMPLATE_H_

