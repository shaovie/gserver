// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-19 16:22
 */
//========================================================================

#ifndef GHZ_SHOU_WEI_MST_H_
#define GHZ_SHOU_WEI_MST_H_

#include "monster_template.h"

// Forward declarations
class monster_cfg_obj;

/**
 * @class ghz_shou_wei_mst
 * 
 * @brief
 */
class ghz_shou_wei_mst : public fighting_unmovable_monster
{
  typedef fighting_unmovable_monster super;
public:
  ghz_shou_wei_mst();
  virtual ~ghz_shou_wei_mst() { }

  virtual int  do_load_config(monster_cfg_obj *cfg);
  virtual int  can_attack(char_obj * /*target*/);
  virtual int  can_be_attacked(char_obj * /*attacker*/);
  virtual void do_dead(const int);
private:
  int guild_id_;
};

#endif // GHZ_SHOU_WEI_MST_H_

