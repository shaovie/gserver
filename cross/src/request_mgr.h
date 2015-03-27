// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-07-08 15:22
 */
//========================================================================

#ifndef REQUEST_MGR_H_
#define REQUEST_MGR_H_

#include "singleton.h"

// Forward declarations
class world_svc;
class request_mgr_impl;

/**
 * @class request_mgr
 * 
 * @brief
 */
class request_mgr : public singleton<request_mgr>
{
  friend class singleton<request_mgr>;
public:
  int store(const world_svc * , const int );

  world_svc *get(const int , int &);
private:
  request_mgr();
  request_mgr(const request_mgr &);
  request_mgr& operator= (const request_mgr&);

  request_mgr_impl *impl_;
};

#endif // REQUEST_MGR_H_

