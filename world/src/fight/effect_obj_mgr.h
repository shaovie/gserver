// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-12-29 18:38
 */
//========================================================================

#ifndef EFFECT_OBJ_MGR_H_
#define EFFECT_OBJ_MGR_H_

#include "singleton.h"

// Forward declarations
class time_value;
class effect_obj;
class effect_obj_mgr_impl;

/**
 * @class effect_obj_mgr
 * 
 * @brief
 */
class effect_obj_mgr : public singleton<effect_obj_mgr>
{
  friend class singleton<effect_obj_mgr>;
public:
  void do_timeout(const time_value &now);

  int assign_effect_obj_id();
  void insert(effect_obj *seo);

private:
  effect_obj_mgr();

  effect_obj_mgr_impl *impl_;
};

#endif // EFFECT_OBJ_MGR_H_

