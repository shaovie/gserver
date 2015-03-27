// -*- C++ -*-

//========================================================================
/**
 * Author   : brucexu
 * Date     : 2012-05-14 18:41
 */
//========================================================================

#ifndef ILIST_H_
#define ILIST_H_

#include <stddef.h>
#include <assert.h>

/**
 * @class ilist_node
 * 
 * @brief
 */
template<typename OBJ>
class ilist_node
{
public:
  ilist_node() : value_(), next_(NULL) { }
  ilist_node(const OBJ &v) : value_(v), next_(NULL) { }

  OBJ value_;
  ilist_node<OBJ>* next_;
};
/**
 * @class ilist
 * 
 * @brief simple list
 */
template<typename NODE>
class ilist
{
public:
  ilist() :
    size_(0),
    head_(NULL),
    tail_(NULL),
    free_obj_list_(NULL)
  { }

  ~ilist()
  {
    this->clear();
    for (ilist_node<NODE> *itor = this->free_obj_list_; itor != NULL; )
    {
      ilist_node<NODE> *p = itor;
      itor = itor->next_;
      delete p;
    }
  }
  void reset()
  { 
    this->size_ = 0;
    this->head_ = NULL;
    this->tail_ = NULL;
  }
  void clear()
  {
    ilist_node<NODE> *itor = this->head_;
    while (itor != NULL)
    {
      ilist_node<NODE> *n = itor->next_;
      this->release(itor);
      itor = n;
    }
    this->reset();
  }

  inline bool empty() const { return this->size_ == 0; }

  inline int size() const { return this->size_; }

  inline ilist_node<NODE> *head() const { return this->head_; }

  inline void push_front(const NODE &n)
  {
    ilist_node<NODE> *node = this->alloc();
    node->value_ = n;
    if (this->empty())
      this->tail_ = node;
    else
      node->next_ = this->head_;

    this->head_ = node;
    ++this->size_;
  }
  inline void push_back(const NODE &n)
  {
    ilist_node<NODE> *node = this->alloc();
    node->value_ = n;
    if (this->empty())
      this->head_ = node;
    else
      this->tail_->next_ = node;

    this->tail_ = node;
    ++this->size_;
  }
  inline NODE pop_front()
  {
    ilist_node<NODE> *node = this->head_;
    this->head_ = this->head_->next_;
    if (this->head_ == NULL)
      this->tail_ = NULL;
    node->next_ = NULL;
    --this->size_;
    NODE n = node->value_;
    this->release(node);
    return n;
  }
  bool find(const NODE &n)
  {
    ilist_node<NODE> *itor = this->head_;
    while (itor != NULL)
    {
      if (itor->value_ == n)
        return true;
      itor = itor->next_;
    }
    return false;
  }
  int remove(const NODE &n)
  {
    int ret = -1;
    while (this->head_ != NULL
           && this->head_->value_ == n)
    {
      ilist_node<NODE> *p = this->head_;
      this->head_ = this->head_->next_;
      this->release(p);

      if (this->head_ == NULL)
        this->tail_ = NULL;
      --this->size_;
      ret = 0;
    }
    if (this->head_ == NULL) return ret;

    ilist_node<NODE> *node = this->head_;
    while (node->next_ != NULL)
    {
      if (node->next_->value_ == n)
      {
        if (this->tail_ == node->next_)
          this->tail_= node;
        ilist_node<NODE> *p = node->next_;
        node->next_ = node->next_->next_;
        this->release(p);
        --this->size_;
        ret = 0;
      }else
        node = node->next_;
    }
    return ret;
  }
private:
  ilist(const ilist &);
  ilist &operator=(const ilist &);

  inline ilist_node<NODE> *alloc()
  {
    if (this->free_obj_list_ == NULL)
      this->grow_up(1);
    ilist_node<NODE> *itor = this->free_obj_list_;
    this->free_obj_list_ = this->free_obj_list_->next_;
    itor->next_ = NULL;
    return itor;
  }

  inline void release(ilist_node<NODE> *p)
  {
    p->next_ = this->free_obj_list_;
    this->free_obj_list_ = p;
  }

  void grow_up(const int alloc_num)
  {
    for (int i = 0; i < alloc_num; ++i)
      this->release(new ilist_node<NODE>());
  }
private:
  int size_;
  ilist_node<NODE> *head_;
  ilist_node<NODE> *tail_;
  ilist_node<NODE> *free_obj_list_;
};

#endif // ILIST_H_

