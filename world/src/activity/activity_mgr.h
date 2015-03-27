// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-05 14:52
 */
//========================================================================

#ifndef ACTIVITY_MGR_H_
#define ACTIVITY_MGR_H_

// Lib header
#include "singleton.h"

// Forward declarations
class activity_obj;
class activity_mgr_impl;

/**
 * @class activity_mgr
 *
 * @brief
 */
class activity_mgr : public singleton<activity_mgr>
{
  friend class singleton<activity_mgr>;
public:
  int init();

  void run(const int now);

  bool is_opened(const int act_id);
  activity_obj *find(const int act_id);
private:
  activity_mgr();

  activity_mgr_impl *impl_;
};

#endif // ACTIVITY_MGR_H_

