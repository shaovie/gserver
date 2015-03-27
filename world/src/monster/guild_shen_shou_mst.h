// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-04 12:38
 */
//========================================================================

#ifndef GUILD_SHEN_SHOU_MST_H_
#define GUILD_SHEN_SHOU_MST_H_

#include "fighting_monster_obj.h"

// Forward declarations

/**
 * @class guild_shen_shou_mst
 * 
 * @brief
 */
class guild_shen_shou_mst : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  guild_shen_shou_mst() { }
  virtual ~guild_shen_shou_mst() { }

  virtual void do_dead(const int) { super::do_dead(0); }
};

#endif // GUILD_SHEN_SHOU_MST_H_

