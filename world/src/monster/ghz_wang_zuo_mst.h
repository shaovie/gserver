// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-19 15:48
 */
//========================================================================

#ifndef GHZ_WANG_ZUO_MST_H_
#define GHZ_WANG_ZUO_MST_H_

#include "monster_template.h"

// Forward declarations
class monster_cfg_obj;

/**
 * @class ghz_wang_zuo_mst
 * 
 * @brief
 */
class ghz_wang_zuo_mst : public fighting_unmovable_monster
{
  typedef fighting_unmovable_monster super;
public:
  ghz_wang_zuo_mst();
  virtual ~ghz_wang_zuo_mst() { }

  virtual int do_load_config(monster_cfg_obj *cfg);
  virtual void on_activated();
  virtual int  can_attack(char_obj * /*target*/);
  virtual int  can_be_attacked(char_obj * /*attacker*/);
  virtual void do_dead(const int);
private:
  int guild_id_;
};

#endif // GHZ_WANG_ZUO_MST_H_

