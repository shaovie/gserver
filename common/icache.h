// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-04-09 16:24
 */
//========================================================================

#ifndef ICACHE_H_
#define ICACHE_H_

// Lib header
#include "thread_mutex.h"
#include <tr1/unordered_map>

#include "obj_pool.h"

#define CACHE_OBJ_SIZE  20480

// Forward declarations
template<typename KEY, typename VALUE>
class cache_obj;
template<typename KEY, typename VALUE>
class cobj_list;

/**
 * @class lru cache
 * 
 * @brief
 */
template<typename KEY, typename VALUE, typename ALLOCATOR>
class icache
{
  typedef std::tr1::unordered_map<KEY, cache_obj<KEY, VALUE> *> obj_hash_map_t;
  typedef typename std::tr1::unordered_map<KEY, cache_obj<KEY, VALUE> *>::iterator obj_hash_map_itor;
public:
  icache(const int max_objs) :
    max_objs_(max_objs),
    obj_map_(max_objs)
  { }
  VALUE *find(const KEY &k)
  {
    guard<thread_mutex> g(this->obj_map_mtx_);
    obj_hash_map_itor itor = this->obj_map_.find(k);
    if (itor == this->obj_map_.end())
      return NULL;
    this->cobj_list_.adjust(itor->second);
    return itor->second->value_;
  }
  int insert(const KEY &k, VALUE *v)
  {
    guard<thread_mutex> g(this->obj_map_mtx_);
    obj_hash_map_itor itor = this->obj_map_.find(k);
    if (itor == this->obj_map_.end())
    {
      this->drop_one();
      VALUE *p = this->allocator_.alloc();
      *p = *v;
      this->insert_i(k, p);
    }else
    {
      *(itor->second->value_) = *v;
      this->cobj_list_.adjust(itor->second);
    }
    return 0;
  }
  void remove(const KEY &k)
  {
    guard<thread_mutex> g(this->obj_map_mtx_);
    this->remove_i(k);
  }
  int size() const
  { 
    guard<thread_mutex> g(this->obj_map_mtx_);
    return this->obj_map_.size();
  }
private:
  void drop_one()
  {
    if (this->obj_map_.size() >= this->max_objs_)
    {
      cache_obj<KEY, VALUE> *co = NULL;
      if (this->cobj_list_.drop(co) == 0)
      {
        this->obj_map_.erase(co->key_);
        this->allocator_.release(co->value_);
        this->cobj_pool_.release(co);
      }
    }
  }
  void insert_i(const KEY &k, VALUE *v)
  {
    cache_obj<KEY, VALUE> *co = this->cobj_pool_.alloc();
    co->reset(k, v);
    this->obj_map_.insert(std::make_pair(k, co));
    this->cobj_list_.insert(co);
  }
  void remove_i(const KEY &k)
  {
    obj_hash_map_itor itor = this->obj_map_.find(k);
    if (itor == this->obj_map_.end()) return ;

    this->cobj_list_.remove(itor->second);

    delete itor->second->value_;
    this->cobj_pool_.release(itor->second);
    this->obj_map_.erase(itor);
  }
private:
  size_t max_objs_;

  obj_hash_map_t obj_map_;
  ALLOCATOR allocator_;
  obj_pool<cache_obj<KEY, VALUE>, obj_pool_std_allocator<cache_obj<KEY, VALUE> > > cobj_pool_;
  cobj_list<KEY, VALUE> cobj_list_;
  thread_mutex obj_map_mtx_;
};
/**
 * @class cache_obj
 * 
 * @brief
 */
template<typename KEY, typename VALUE>
class cache_obj
{
public:
  cache_obj() :
    key_(),
    value_(NULL),
    prev_(NULL),
    next_(NULL)
  { }

  cache_obj(const KEY &k, VALUE *v) :
    key_(k),
    value_(v),
    prev_(NULL),
    next_(NULL)
  { }

  void reset(const KEY &k, VALUE *v)
  {
    this->key_   = k;
    this->value_ = v;
    this->next_  = NULL;
    this->prev_  = NULL;
  }
  void next(cache_obj *co) { this->next_ = co; }
  cache_obj *next() { return this->next_; }
public:
  KEY  key_;
  VALUE *value_;
  cache_obj<KEY, VALUE> *prev_;
  cache_obj<KEY, VALUE> *next_;
};
/**
 * @class cobj_list
 * 
 * @brief
 */
template<typename KEY, typename VALUE>
class cobj_list
{
public:
  cobj_list() : head_(NULL), tail_(NULL) { }

  void insert(cache_obj<KEY, VALUE> *co)
  {
    if (this->tail_ == NULL)
      this->head_ = this->tail_ = co;
    else
    {
      co->prev_ = this->tail_;
      this->tail_->next_ = co;
      this->tail_ = co;
    }
  }
  int drop(cache_obj<KEY, VALUE> *&cobj)
  {
    if (this->head_ == NULL) return -1;
    cobj = this->head_;
    this->remove(cobj);
    return 0;
  }
  void remove(cache_obj<KEY, VALUE> *co)
  {
    if (co->next_ != NULL && co->prev_ != NULL)
    {
      co->prev_->next_ = co->next_;
      co->next_->prev_ = co->prev_;
    }else if (co->next_ != NULL) // is head
    {
      this->head_ = co->next_;
      this->head_->prev_ = NULL;
    }else if (co->prev_ != NULL) // is tail
    {
      this->tail_ = co->prev_;
      this->tail_->next_ = NULL;
    }else // just one
    {
      this->head_ = NULL;
      this->tail_ = NULL;
    }
    co->prev_ = co->next_ = NULL;
  }
  void adjust(cache_obj<KEY, VALUE> *co)
  {
    this->remove(co);
    this->insert(co);
  }
private:
  cache_obj<KEY, VALUE> *head_;
  cache_obj<KEY, VALUE> *tail_;
};

template<typename VALUE>
class value_allocator
{
public:
  value_allocator() { }

  VALUE *alloc() { return this->pool_.alloc(); }
  void release(VALUE *p) { this->pool_.release(p); }
private:
  obj_pool<VALUE, obj_pool_std_allocator<VALUE> > pool_;
};

#endif // ICACHE_H_

