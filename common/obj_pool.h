// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-14 18:12
 */
//========================================================================

#ifndef OBJ_POOL_H_
#define OBJ_POOL_H_

#include <stddef.h>

/**
 * @class obj_pool_std_allocator
 * 
 * @brief
 */
template<typename OBJ>
class obj_pool_std_allocator
{
public:
  OBJ *alloc() { return new OBJ(); }
  void release(OBJ *p) { delete p; }
};
/**
 * @class obj_pool
 * 
 * @brief
 */
template<typename OBJ, typename ALLOCATOR>
class obj_pool
{
  template<typename T>
  class _obj_node
  {
  public:
    _obj_node() : obj_(NULL), next_(NULL) { }
    
    T *obj_;
    _obj_node<T> *next_;
  };
public:
  obj_pool() :
    free_obj_list_(NULL),
    free_obj_node_list_(NULL)
  { }

  OBJ *alloc()
  {
    if (this->free_obj_list_ == NULL)
      this->release(this->allocator_.alloc());
    
    _obj_node<OBJ> *p = this->free_obj_list_;
    OBJ *o = p->obj_;
    this->free_obj_list_ = this->free_obj_list_->next_;
    this->release_node(p);
    return o;
  }

  inline void release(OBJ *p)
  { 
    if (p == NULL) return ;

    _obj_node<OBJ> *n = this->alloc_node();
    n->obj_  = p;
    n->next_ = this->free_obj_list_;
    this->free_obj_list_ = n;
  }

  inline void flush()
  {
    for (_obj_node<OBJ> *itor = this->free_obj_list_; itor != NULL; )
    {
      _obj_node<OBJ> *p = itor;
      itor = itor->next_;
      this->allocator_.release(p->obj_);
      this->release_node(p);
    }
    this->free_obj_list_ = NULL;
    for (_obj_node<OBJ> *itor = this->free_obj_node_list_; itor != NULL; )
    {
      _obj_node<OBJ> *p = itor;
      itor = itor->next_;
      delete p;
    }
    this->free_obj_node_list_ = NULL;
  }
private:
  _obj_node<OBJ> *alloc_node()
  {
    if (this->free_obj_node_list_ == NULL)
      this->release_node(new _obj_node<OBJ>());
    _obj_node<OBJ> *p = this->free_obj_node_list_;
    this->free_obj_node_list_ = this->free_obj_node_list_->next_;
    return p;
  }
  void release_node(_obj_node<OBJ> *p)
  {
    if (p == NULL) return ;
    p->next_ = this->free_obj_node_list_;
    this->free_obj_node_list_ = p;
  }
private:
  _obj_node<OBJ> *free_obj_list_;
  _obj_node<OBJ> *free_obj_node_list_;

  ALLOCATOR allocator_;
};

#endif // OBJ_POOL_H_

