// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-09 12:39
 */
//========================================================================

#ifndef PROXY_MGR_H_
#define PROXY_MGR_H_

// Lib header
#include "guard.h"
#include "singleton.h"
#include "thread_mutex.h"

#include "array_t.h"
#include "proxy_obj.h"
#include "db_async_store.h"

// Forward declarations
class proxy_obj;
class proxy_mgr_impl;

#define MAX_PROXY_OBJ_NUM   1024

/**
 * @class proxy_mgr
 * 
 * @brief
 */
class proxy_mgr : public singleton<proxy_mgr>
{
  friend class singleton<proxy_mgr>;
public:
  int alloc_id()
  {
    if (this->proxy_id_list_.empty())
      return -1;
    return this->proxy_id_list_.pop_front();
  }
  void release_id(const int id)
  {
    if (id < 1 || id >= MAX_PROXY_OBJ_NUM)
      return ;
    this->proxy_id_list_.push_back(id);
  }
  void insert(const int id, proxy_obj *obj)
  {
    guard<thread_mutex> g(this->proxy_map_mtx_);
    this->proxy_map_.insert(id, obj);
  }
  proxy_obj *find(const int id)
  {
    guard<thread_mutex> g(this->proxy_map_mtx_);
    return this->proxy_map_.find(id);
  }
  void remove(const int id)
  {
    guard<thread_mutex> g(this->proxy_map_mtx_);
    this->proxy_map_.remove(id);
  }

  //=
  int post_result(const int id, mblock *mb)
  {
    guard<thread_mutex> g(this->proxy_map_mtx_);
    proxy_obj *po = this->proxy_map_.find(id);
    if (po == NULL) return -1;
    po->post_result(mb);
    return 0;
  }
private:
  array_t<proxy_obj *> proxy_map_;
  thread_mutex proxy_map_mtx_;

  ilist<int> proxy_id_list_;
private:
  proxy_mgr() : proxy_map_(MAX_PROXY_OBJ_NUM)
  {
    for (int i = 1; i < MAX_PROXY_OBJ_NUM; ++i)
      this->proxy_id_list_.push_back(i);
  }
  //=
  proxy_mgr(const proxy_mgr &);
  proxy_mgr& operator= (const proxy_mgr &);
};

#endif // PROXY_MGR_H_

