// -*- C++ -*-

//========================================================================
/**
 * Author   : brucexu
 * Date     : 2012-05-15 13:41
 */
//========================================================================

#ifndef IHASH_H_
#define IHASH_H_

#include <stddef.h>
//#include <string> 

template<class KEY, class VALUE>
class ihash;

/**
 * @class ihash_node
 *
 * @brief for construct ihash
 */
template<class KEY, class VALUE>
class ihash_node
{
  friend class ihash<KEY, VALUE>;
public:
  ihash_node()
  : key_(),
  value_(),
  next_(NULL)
  { }
  ihash_node(const KEY &k, const VALUE &v)
    : key_(k),
    value_(v),
    next_(NULL)
  { }
  void reset(const KEY &k, const VALUE &v)
  {
    this->key_   = k;
    this->value_ = v;
    this->next_  = NULL;
  }
protected:
  KEY key_;

  VALUE value_;

  ihash_node<KEY, VALUE> *next_;
};

/**
 * @class ihash
 *
 * @brief fast than stl|boost
 */
template<class KEY, class VALUE>
class ihash
{
public:
  ihash(unsigned int size)
    : size_(size),
    m_size_(0),
    free_node_list_(NULL)
  {
    this->size_ = this->get_prime(this->size_);
    this->table_ = new ihash_node<KEY, VALUE>*[this->size_];

    for (unsigned int i = 0; i < this->size_; ++i)
    {
      this->table_[i] = NULL;
      this->release(new ihash_node<KEY, VALUE>());
    }
  }

  ~ihash()
  {
    for (unsigned int i = 0; i < this->size_; ++i)
    {
      ihash_node<KEY, VALUE> *node = this->table_[i];
      while (node)
      {
        ihash_node<KEY, VALUE> *next = node->next_;
        delete node;
        node = next;
      }
    }
    delete[] this->table_;

    for (ihash_node<KEY, VALUE> *itor = this->free_node_list_; itor != NULL; )
    {
      ihash_node<KEY, VALUE> *p = itor;
      itor = itor->next_;
      delete p;
    }
  }

  void clear()
  {
    for (unsigned int i = 0; i < this->size_; ++i)
    {
      ihash_node<KEY, VALUE> *node = this->table_[i];
      if (node != NULL)
        this->unbind(node->key_);
    }
  }

  inline bool empty() const
  { return this->m_size_ == 0; }

  inline int bind(const KEY &k, const VALUE &v)
  {
    if (this->find(k)) return -1;

    ihash_node<KEY, VALUE> *node = this->alloc();
    node->reset(k, v);

    unsigned int idx = hash_i(k) % this->size_;
    node->next_ = this->table_[idx];
    this->table_[idx] = node;
    ++(this->m_size_);
    return 0;
  }

  inline int unbind(const KEY &k)
  {
    unsigned int idx = hash_i(k) % this->size_;
    for (ihash_node<KEY, VALUE> *node = this->table_[idx], **ahead = &this->table_[idx];
         node != NULL;
         ahead = &(node->next_), node = node->next_)
    {
      if (node->key_ == k)
      {
        *ahead = node->next_;
        this->release(node);
        --(this->m_size_);
        return 0;
      }
    }
    return -1;
  }

  inline bool find(const KEY &k)
  {
    unsigned int idx = hash_i(k) % this->size_;
    for (ihash_node<KEY, VALUE> *node = this->table_[idx];
         node != NULL;
         node = node->next_)
    {
      if (node->key_ == k)
        return true;
    }
    return false;
  }
  inline int find(const KEY &k, VALUE &v)
  {
    unsigned int idx = hash_i(k) % this->size_;
    for (ihash_node<KEY, VALUE> *node = this->table_[idx];
         node != NULL;
         node = node->next_)
    {
      if (node->key_ == k)
      {
        v = node->value_;
        return 0;
      }
    }
    return -1;
  }

private:
  ihash_node<KEY, VALUE> *alloc()
  {
    if (this->free_node_list_ == NULL)
      this->grow_up(4);
    ihash_node<KEY, VALUE> *p = this->free_node_list_;
    this->free_node_list_ = this->free_node_list_->next_;
    p->next_ = NULL;
    return p;
  }
  void release(ihash_node<KEY, VALUE> *n)
  {
    n->next_ = this->free_node_list_;
    this->free_node_list_ = n;
  }
  void grow_up(const int alloc_num)
  {
    for (int i = 0; i < alloc_num; ++i)
      this->release(new ihash_node<KEY, VALUE>());
  }

  int get_prime(const unsigned int n)
  {
    for (unsigned int i = 0; i < ihash<KEY, VALUE>::ihash_prime_list_size; ++i)
    {
      if (n <= ihash<KEY, VALUE>::ihash_prime_list[i])
        return ihash<KEY, VALUE>::ihash_prime_list[i];
    }
    return n;
  }

  unsigned int size_;
  unsigned int m_size_;

  //
  ihash_node<KEY, VALUE> **table_;
  ihash_node<KEY, VALUE> *free_node_list_;

  static unsigned int const ihash_prime_list[];
  static unsigned int const ihash_prime_list_size;
};
template<class KEY, class VALUE>
unsigned int const ihash<KEY, VALUE>::ihash_prime_list[] = {
  17,         29,         37,           53,           67,
  97,         131,        193,          257,          389,
  521,        769,        1031,         1543,         2053,
  3079,       6151,       12289,        24593,        49157,
  98317,      196613,     393241,       786433,       1572869, 
  3145739,    6291469,    12582917,     25165843,     50331653,
  50331653,   100663319,  201326611,    402653189,    805306457,
  1610612741
};
template<class KEY, class VALUE>
unsigned int const ihash<KEY, VALUE>::ihash_prime_list_size = 36;

template<class KEY>
inline unsigned int hash_i(const KEY &key)
{ return 0; }

template<>
inline unsigned int hash_i<int>(const int &key)
{ return (key & 0x7fffffff); }

#if 0
template<>
inline unsigned int hash_i<std::string>(const std::string &str)
{
  unsigned int hash = 0;
  const char *c = str.data();

  while (*c)
    hash += (hash << 5) + (*c++);

  return (hash & 0x7fffffff);
}
#endif

#endif
