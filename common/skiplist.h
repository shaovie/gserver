// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-05-06 16:21
 */
//========================================================================

#ifndef SKIPLIST_H_
#define SKIPLIST_H_

#include <stdlib.h>
#define SL_MAXLEVEL  8

template <typename VALUE>
class skiplist_node
{
public:
  skiplist_node(const int level, const VALUE &v) :
    prev_(NULL),
    value_(v)
  {
    int my_level = (level < 1 ? 1 : level);
    this->lvl_next_ = new skiplist_node<VALUE> *[my_level];
    this->lvl_span_ = new int[my_level];
    for (int i = 0; i < my_level; ++i)
    {
      this->lvl_next_[i] = NULL;
      this->lvl_span_[i] = 0;
    }
  }
  ~skiplist_node()
  {
    delete []this->lvl_next_;
    delete []this->lvl_span_;
  }

  skiplist_node<VALUE> *next() { return this->lvl_next_[0]; }
  skiplist_node<VALUE> *prev() { return this->prev_; }

  skiplist_node<VALUE> *prev_;
  skiplist_node<VALUE> **lvl_next_;
  int *lvl_span_;

  VALUE value_;
};

template <typename VALUE>
class skiplist
{
public:
  skiplist() :
    max_level_(1),
    length_(0)
  {
    this->head_ = new skiplist_node<VALUE>(SL_MAXLEVEL, VALUE());
    this->tail_ = this->head_;
  }
  ~skiplist()
  {
    this->clear();
    delete this->head_;
  }

  int random_level()
  {
    int level = 1;
    while (level < SL_MAXLEVEL
           && (::random() & 0xFFFF) < (0.25 * 0xFFFF))
      level += 1;
    return level;
  }
  void clear()
  {
    skiplist_node<VALUE> *curr = this->tail_;
    while (curr != this->head_)
    {
      this->tail_ = this->tail_->prev_;
      delete curr;
      curr = this->tail_;
    }
    for (int i = this->max_level_ - 1; i >= 0; --i)
      this->head_->lvl_next_[i] = NULL;
    this->max_level_ = 1;
    this->length_ = 0;
  }
  void insert(const VALUE &value)
  {
    skiplist_node<VALUE> *update[SL_MAXLEVEL] = {0};
    int rank[SL_MAXLEVEL] = {0};

    int level = this->random_level();
    skiplist_node<VALUE> *node = new skiplist_node<VALUE>(level, value);
    if (level > this->max_level_)
      this->max_level_ = level;

    int curr_rank = 0;
    skiplist_node<VALUE> *curr = this->head_;
    for (int i = this->max_level_ - 1; i >= 0; --i)
    {
      while (curr->lvl_next_[i] != NULL
             && !(node->value_ > curr->lvl_next_[i]->value_))
      {
        curr = curr->lvl_next_[i];
        curr_rank += curr->lvl_span_[i];
      }
      update[i] = curr;
      rank[i] = curr_rank;
    }
    for (int i = 0; i < level; ++i)
    {
      node->lvl_next_[i] = update[i]->lvl_next_[i];
      update[i]->lvl_next_[i] = node;

      node->lvl_span_[i] = rank[0] - rank[i] + 1;
      if (node->lvl_next_[i] != NULL)
        node->lvl_next_[i]->lvl_span_[i] -= node->lvl_span_[i] - 1;
    }
    for (int i = level; i < this->max_level_; ++i)
    {
      if (update[i]->lvl_next_[i] != NULL)
        ++update[i]->lvl_next_[i]->lvl_span_[i];
    }

    node->prev_ = curr;
    if (node->lvl_next_[0] != NULL)
      node->lvl_next_[0]->prev_ = node;
    else
      this->tail_ = node;
    ++this->length_;
  }
  void drop(const VALUE &value)
  {
    skiplist_node<VALUE> *curr = this->head_;
    skiplist_node<VALUE> *node = NULL;
    for (int i = this->max_level_ - 1; i >= 0; --i)
    {
      while ((node != NULL && curr->lvl_next_[i] != node)
             || (curr->lvl_next_[i] != NULL
                 && curr->lvl_next_[i]->value_ > value))
        curr = curr->lvl_next_[i];

      if (node != NULL && curr->lvl_next_[i] == node)
      {
        curr->lvl_next_[i] = node->lvl_next_[i];
        if (curr->lvl_next_[i] != NULL)
          curr->lvl_next_[i]->lvl_span_[i] += node->lvl_span_[i] - 1;
      }else
      {
        // 为防止 operator > 中参加比较的数据不包含 operator == 中的唯一数值
        // 而其他值都相等
        skiplist_node<VALUE> *temp = curr;
        while (temp->lvl_next_[i] != NULL)
        {
          if (value > temp->lvl_next_[i]->value_)
            break;
          else if (temp->lvl_next_[i]->value_ == value)
          {
            curr = temp;
            node = curr->lvl_next_[i];
            curr->lvl_next_[i] = node->lvl_next_[i];
            if (curr->lvl_next_[i] != NULL)
              curr->lvl_next_[i]->lvl_span_[i] += node->lvl_span_[i] - 1;
            break;
          }
          temp = temp->lvl_next_[i];
        }
      }
    }

    if (node != NULL)
    {
      if (curr->lvl_next_[0] != NULL)
        curr->lvl_next_[0]->prev_ = curr;
      else
        this->tail_ = curr;
      while (this->max_level_ > 1
             && this->head_->lvl_next_[0] == NULL)
        --this->max_level_;
      --this->length_;
      delete node;
    }
  }
  skiplist_node<VALUE>* find(const VALUE &value)
  {
    skiplist_node<VALUE> *curr = this->head_;
    for (int i = this->max_level_ - 1; i >= 0; --i)
    {
      while (curr->lvl_next_[i] != NULL
             && curr->lvl_next_[i]->value_ > value)
        curr = curr->lvl_next_[i];

      // 为防止 operator > 中参加比较的数据不包含 operator == 中的唯一数值
      // 而其他值都相等
      skiplist_node<VALUE> *temp = curr;
      while (temp->lvl_next_[i] != NULL)
      {
        if (value > temp->lvl_next_[i]->value_)
          break;
        else if (temp->lvl_next_[i]->value_ == value)
          return temp->lvl_next_[i];

        temp = temp->lvl_next_[i];
      }
    }
    return NULL;
  }
  int get_rank(const VALUE &value)
  {
    int rank = 0;
    skiplist_node<VALUE> *curr = this->head_;
    for (int i = this->max_level_ - 1; i >= 0; --i)
    {
      while (curr->lvl_next_[i] != NULL
             && curr->lvl_next_[i]->value_ > value)
      {
        curr = curr->lvl_next_[i];
        rank += curr->lvl_span_[i];
      }

      // 为防止 operator > 中参加比较的数据不包含 operator == 中的唯一数值
      // 而其他值都相等
      skiplist_node<VALUE> *temp = curr;
      while (temp->lvl_next_[i] != NULL)
      {
        if (value > temp->lvl_next_[i]->value_)
          break;
        else if (temp->lvl_next_[i]->value_ == value)
          return rank + temp->lvl_next_[i]->lvl_span_[i];

        temp = temp->lvl_next_[i];
        rank += temp->lvl_span_[i];
      }
    }
    return -1;
  }

  skiplist_node<VALUE>* begin() { return this->head_->next(); }
  skiplist_node<VALUE>* end()   { return this->tail_->next(); }

  int max_level() const { return this->max_level_; }
  int length() const    { return this->length_;    }
private:
  int max_level_;
  int length_;

  skiplist_node<VALUE> *head_;
  skiplist_node<VALUE> *tail_;
};
#endif//SKIPLIST_H_
