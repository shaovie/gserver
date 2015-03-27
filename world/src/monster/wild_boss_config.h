// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2013-03-06 17:18
 */
//========================================================================

#ifndef WILD_BOSS_CONFIG_H_
#define WILD_BOSS_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define WILD_BOSS_CFG_PATH                      "wild_boss.json"

// Forward declarations
class wild_boss_config_impl;

/**
 * @class wild_boss_config
 * 
 * @brief
 */
class wild_boss_config : public singleton<wild_boss_config>
{
  friend class singleton<wild_boss_config>;
public:
  class wild_boss
  {
  public:
    wild_boss() :
      index_(0),
      monster_cid_(0),
      scene_cid_(0),
      interval_(0)
    { }
  public:
    int index_;
    int monster_cid_;
    int scene_cid_;
    int interval_;
    ilist<coord_t> coord_;
  };
public:
  int reload_config(const char *cfg_root);
  int load_config(const char *cfg_root);

  ilist_node<wild_boss_config::wild_boss*>* get_head();
private:
  wild_boss_config();

  wild_boss_config_impl *impl_;
};

#endif // WILD_BOSS_CONFIG_H_
