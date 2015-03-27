// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2013-03-06 18:47
 */
//========================================================================

#ifndef WILD_BOSS_REFRESH_H_
#define WILD_BOSS_REFRESH_H_

// Lib header
#include "singleton.h"

// Forward declarations
class wild_boss_refresh_impl;
class out_stream;

/**
 * @class wild_boss_refresh
 * 
 * @brief
 */
class wild_boss_refresh : public singleton<wild_boss_refresh>
{
  friend class singleton<wild_boss_refresh>;
public:
  class wild_boss
  {
  public:
    wild_boss() :
      monster_cid_(0),
      monster_id_(0),
      next_appear_time_(0)
    { }
  public:
    int monster_cid_;
    int monster_id_;
    int next_appear_time_;
  };
public:
  int do_timeout(const int now);
  int fetch_wild_boss_info(out_stream &os);

private:
  wild_boss_refresh();

  wild_boss_refresh_impl *impl_;
};

#endif // WILD_BOSS_REFRESH_H_
