// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-19 11:20
 */
//========================================================================

#ifndef WORLD_SVC_MGR_H_
#define WORLD_SVC_MGR_H_

// Lib header
#include "singleton.h"

// Forward declarations
class world_svc;
class out_stream;
class world_svc_mgr_impl;

/**
 * @class world_svc_mgr
 * 
 * @brief
 */
class world_svc_mgr : public singleton<world_svc_mgr>
{
  friend class singleton<world_svc_mgr>;
public:
  world_svc *find(const char *, const int );
  world_svc *random(const char *);
  void insert(const char *, const int , world_svc *);
  void remove(const char *, const int );

  int world_svc_cnt();

  void dump(const int now);
private:
  world_svc_mgr();

  world_svc_mgr_impl *impl_;
};

#endif // WORLD_SVC_MGR_H_
