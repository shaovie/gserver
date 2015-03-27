// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-14 22:58
 */
//========================================================================

#ifndef FIGHTER_MGR_H_
#define FIGHTER_MGR_H_

#include "singleton.h"

// Forward declarations
class char_obj;
class fighter_mgr_impl;

/**
 * @class fighter_mgr
 * 
 * @brief
 */
class fighter_mgr : public singleton<fighter_mgr>
{
  friend class singleton<fighter_mgr>;
public:
  void insert(const int char_id, char_obj *);

  char_obj *find(const int char_id);

  void remove(const int char_id);
private:
  fighter_mgr();
  fighter_mgr(const fighter_mgr &);
  fighter_mgr& operator= (const fighter_mgr &);

  fighter_mgr_impl *impl_;
};

#endif // FIGHTER_MGR_H_

