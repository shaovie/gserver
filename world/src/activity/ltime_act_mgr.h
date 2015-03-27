// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-15 21:48
 */
//========================================================================

#ifndef LTIME_ACT_MGR_H_
#define LTIME_ACT_MGR_H_

// Lib header
#include "singleton.h"

// Forward declarations
class player_obj;
class ltime_act_obj;
class ltime_act_mgr_impl;

/**
 * @class ltime_act_mgr
 * 
 * @brief
 */
class ltime_act_mgr : public singleton<ltime_act_mgr>
{
  friend class singleton<ltime_act_mgr>;
public:
  void run(const int now);

  void insert(const int act_id, ltime_act_obj *lao);
  bool is_opened(const int act_id);
  ltime_act_obj *find(const int act_id);

  void on_enter_game(player_obj *);
private:
  ltime_act_mgr();

  ltime_act_mgr_impl *impl_;
};

#endif // LTIME_ACT_MGR_H_

