// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-05 18:22
 */
//========================================================================

#ifndef SESSION_MGR_H_
#define SESSION_MGR_H_

#include "singleton.h"

// Forward declarations
class world_svc;
class session_mgr_impl;

/**
 * @class session_mgr
 * 
 * @brief
 */
class session_mgr : public singleton<session_mgr>
{
  friend class singleton<session_mgr>;
public:
  int alloc_session_id();

  void insert(const int , world_svc *);

  world_svc *find(const int );

  world_svc *random();

  void remove(const int );
private:
  session_mgr();
  session_mgr(const session_mgr &);
  session_mgr& operator= (const session_mgr&);

  session_mgr_impl *impl_;
};

#endif // SESSION_MGR_H_

