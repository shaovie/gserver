// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-05-07 19:29
 */
//========================================================================

#ifndef RANK_LIST_H_
#define RANK_LIST_H_

#define SDLIST_FIRST_IDX 1

// Lib header
#include <tr1/unordered_map>

/**
 * @class sdlist_node
 *
 * @brief
 */
template<typename OBJ>
class sdlist_node
{
public:
  sdlist_node(const OBJ &v) :
    idx_(0),
    value_(v),
    next_(NULL),
    prec_(NULL)
  { }
public:
  int idx_;
  OBJ value_;
  sdlist_node<OBJ>* next_;
  sdlist_node<OBJ>* prec_;
};

/**
 * @class sdlist
 *
 * @brief sort doubly linked list
 */
template<typename OBJ>
class sdlist
{
public:
  sdlist() :
    size_(0),
    head_(NULL),
    tail_(NULL)
  { }

  ~sdlist()
  {
    sdlist_node<OBJ> *itor = this->head_;
    while (itor != NULL)
    {
      sdlist_node<OBJ> *n = itor;
      itor = itor->next_;;
      delete n;
    }
  }
public:
  inline bool empty() const { return this->size_ == 0; }
  inline int size() const { return this->size_; }
  inline sdlist_node<OBJ> *head() const { return this->head_; }
  sdlist_node<OBJ> *insert(const OBJ &o)
  {
    sdlist_node<OBJ> *node = new sdlist_node<OBJ>(o);

    if (this->empty())
    {
      this->tail_ = node;
      this->head_ = node;
      node->idx_ = SDLIST_FIRST_IDX;
    }else if (!(node->value_ > this->tail_->value_)) // tail insert
    {
      this->tail_->next_ = node;
      node->prec_ = this->tail_;
      node->idx_ = this->tail_->idx_ + 1;
      this->tail_ = node;
    }else // forward insert
    {
      sdlist_node<OBJ> *itor = this->tail_;
      for(; itor != NULL; itor = itor->prec_)
      {
        if (!(node->value_ > itor->value_))
          break;
        ++itor->idx_;
      }
      if (itor != NULL)
      {
        node->prec_ = itor;
        node->next_ = itor->next_;
        node->idx_ = itor->idx_ + 1;
        itor->next_->prec_ = node;
        itor->next_ = node;
      }else // head insert
      {
        node->next_ = this->head_;
        node->idx_ = SDLIST_FIRST_IDX;
        this->head_->prec_ = node;
        this->head_ = node;
      }
    }

    ++this->size_;
    return node;
  }
  void shift_forward(sdlist_node<OBJ> *node)
  {
    if (this->empty()
        || node == this->head_
        || !(node->value_ > node->prec_->value_))
      return ;

    sdlist_node<OBJ> *itor = node->prec_;
    for (; itor != NULL; itor = itor->prec_)
    {
      if (!(node->value_ > itor->value_))
        break;
      ++itor->idx_;
    }

    if (node->next_ != NULL) node->next_->prec_ = node->prec_;
    node->prec_->next_ = node->next_;
    if (node == this->tail_) this->tail_ = node->prec_;
    if (itor != NULL)
    {
      node->prec_ = itor;
      node->next_ = itor->next_;
      node->idx_ = itor->idx_ + 1;
      itor->next_->prec_ = node;
      itor->next_ = node;
    }else
    {
      node->prec_ = NULL;
      node->next_ = this->head_;
      node->idx_ = this->head_->idx_ - 1;
      this->head_->prec_ = node;
      this->head_ = node;
    }
  }
  void shift_backward(sdlist_node<OBJ> *node)
  {
    if (this->empty()
        || node == this->tail_
        || node->value_ > node->next_->value_)
      return ;

    sdlist_node<OBJ> *itor = node->next_;
    for (; itor != NULL; itor = itor->next_)
    {
      if (node->value_ > itor->value_)
        break;
      --itor->idx_;
    }

    if (node->prec_ != NULL) node->prec_->next_ = node->next_;
    node->next_->prec_ = node->prec_;
    if (node == this->head_) this->head_ = node->next_;
    if (itor != NULL)
    {
      node->next_ = itor;
      node->prec_ = itor->prec_;
      node->idx_ = itor->prec_->idx_ - 1;
      itor->prec_->next_ = node;
      itor->prec_ = node;
    }else
    {
      node->next_ = NULL;
      node->prec_ = this->tail_;
      node->idx_ = this->tail_->idx_ + 1;
      this->tail_->next_ = node;
      this->tail_ = node;
    }
  }
  inline void swap(sdlist_node<OBJ> *node1, sdlist_node<OBJ> *node2)
  {
    if (this->empty()
        || node1 == NULL || node2 == NULL
        || node1 == node2)
      return ;

    if (node1->next_ == node2)
    {
      if (node1->prec_ != NULL) node1->prec_->next_ = node2;
      if (node2->next_ != NULL) node2->next_->prec_ = node1;
      node1->next_ = node2->next_;
      node2->next_ = node1;
      node2->prec_ = node1->prec_;
      node1->prec_ = node2;

    }else if (node2->next_ == node1)
    {
      if (node1->next_ != NULL) node1->next_->prec_ = node2;
      if (node2->prec_ != NULL) node2->prec_->next_ = node1;
      node2->next_ = node1->next_;
      node1->next_ = node2;
      node1->prec_ = node2->prec_;
      node2->prec_ = node1;
    }else
    {
      if (node2->next_ != NULL) node2->next_->prec_ = node1;
      if (node2->prec_ != NULL) node2->prec_->next_ = node1;
      if (node1->next_ != NULL) node1->next_->prec_ = node2;
      if (node1->prec_ != NULL) node1->prec_->next_ = node2;

      sdlist_node<OBJ> *next = node1->next_;
      sdlist_node<OBJ> *prev = node1->prec_;
      node1->next_ = node2->next_;
      node1->prec_ = node2->prec_;
      node2->next_ = next;
      node2->prec_ = prev;
    }

    if (node1 == this->head_)
      this->head_ = node2;
    else if (node2 == this->head_)
      this->head_ = node1;
    if (node1 == this->tail_)
      this->tail_ = node2;
    else if (node2 == this->tail_)
      this->tail_ = node1;

    const int idx = node1->idx_;
    node1->idx_ = node2->idx_;
    node2->idx_ = idx;
  }
private:
  inline void sort_back(sdlist_node<OBJ> *node)
  {
    for (sdlist_node<OBJ> *itor = node->next_;
         itor != NULL;
         itor = itor->next_)
      itor->idx_ = itor->prec_->idx_ + 1;
  }
private:
  int size_;
  sdlist_node<OBJ> *head_;
  sdlist_node<OBJ> *tail_;
};

/**
 * @class rank_list
 *
 * @brief sort && index list
 * @brief VALUE > && = && ==
 */
template<typename KEY, typename CMP>
class rank_list
{
public:
  class sdllist_obj
  {
  public:
    sdllist_obj(const KEY &o, const CMP &c) :
      c_(c),
      o_(o)
    {}
  public:
    CMP c_;
    KEY o_;
  public:
    inline bool operator> (const sdllist_obj &v2) const
    { return this->c_ > v2.c_; }
  };
public:
  typedef std::tr1::unordered_map<KEY, sdlist_node<sdllist_obj> *> node_map_t;
  typedef typename std::tr1::unordered_map<KEY, sdlist_node<sdllist_obj> *>::iterator node_map_itor;
public:
  void insert(const KEY &o)
  {
    sdlist_node<sdllist_obj> *node = this->sort_list_.insert(sdllist_obj(o, 0));
    if (node == NULL) return ;
    this->node_map_.insert(std::make_pair(o, node));
  }
  void insert(const KEY &o, const CMP &c)
  {
    sdlist_node<sdllist_obj> *node = this->sort_list_.insert(sdllist_obj(o, c));
    if (node == NULL) return ;
    this->node_map_.insert(std::make_pair(o, node));
  }
  void update(const KEY &k, const CMP &c)
  {
    node_map_itor itor = this->node_map_.find(k);
    if (itor == this->node_map_.end()) return ;
    CMP old_c = itor->second->value_.c_;
    if (old_c == c) return ;

    itor->second->value_.c_ = c;
    if (c > old_c)
      this->sort_list_.shift_forward(itor->second);
    else
      this->sort_list_.shift_backward(itor->second);
  }
  int index(const KEY &k)
  {
    node_map_itor itor = this->node_map_.find(k);
    if (itor == this->node_map_.end()) return 0;
    return itor->second->idx_;
  }
  void swap(const KEY &k1, const KEY &k2)
  {
    node_map_itor itor1 = this->node_map_.find(k1);
    if (itor1 == this->node_map_.end()) return ;
    node_map_itor itor2 = this->node_map_.find(k2);
    if (itor2 == this->node_map_.end()) return ;
    this->sort_list_.swap(itor1->second, itor2->second);
  }
private:
  sdlist<sdllist_obj> sort_list_;
  node_map_t node_map_;
};
#endif // RANK_LIST_H_
