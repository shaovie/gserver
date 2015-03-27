// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-21 09:17
 */
//========================================================================

#ifndef DROPPED_ITEM_MGR_H_
#define DROPPED_ITEM_MGR_H_

#include "singleton.h"

// Forward declarations
class time_value;
class dropped_item;
class dropped_item_mgr_impl;

/**
 * @class dropped_item_mgr
 * 
 * @brief
 */
class dropped_item_mgr : public singleton<dropped_item_mgr>
{
  friend class singleton<dropped_item_mgr>;
public:
  int assign_dropped_item_id();
  void do_timeout();

  //=
  void insert(const int, dropped_item *);
  dropped_item *find(const int );
  void remove(const int);
private:
  dropped_item_mgr();
  dropped_item_mgr(const dropped_item_mgr &);
  dropped_item_mgr& operator= (const dropped_item_mgr &);

  dropped_item_mgr_impl *impl_;
};

#endif // DROPPED_ITEM_MGR_H_

