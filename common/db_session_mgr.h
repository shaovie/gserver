// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-05 18:22
 */
//========================================================================

#ifndef DB_SESSION_MGR_H_
#define DB_SESSION_MGR_H_

#include "singleton.h"

// Forward declarations
class player_obj;
class db_session_mgr_impl;

/**
 * @class db_session_mgr
 * 
 * @brief
 */
class db_session_mgr : public singleton<db_session_mgr>
{
  friend class singleton<db_session_mgr>;
public:
  int alloc_session_id();

  void insert(const int , player_obj *);

  player_obj *find(const int );

  void remove(const int );
private:
  db_session_mgr();
  db_session_mgr(const db_session_mgr &);
  db_session_mgr& operator= (const db_session_mgr&);

  db_session_mgr_impl *impl_;
};

#endif // DB_SESSION_MGR_H_

