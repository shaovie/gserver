// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-06 10:51
 */
//========================================================================

#ifndef IHEAP_H_
#define IHEAP_H_

#include <string.h>

// Forward declarations

#define IHEAP_ROOT           1
#define IHEAP_PARENT(n)     ((n) / 2)
#define IHEAP_LCHILD(n)     (2 * (n))
#define IHEAP_RCHILD(n)     (2 * (n) + 1)

/**
 * @class iheap
 * 
 * @brief simple min binary heap code
 */
template <typename T>
class iheap
{
public:
  iheap(const int size = 64) :
    cur_size_(0),
    max_size_(size),
    node_(NULL)
  {
    this->node_ = new T*[this->max_size_];

    for (int i = 0; i < this->max_size_; ++i)
      this->node_[i] = NULL;
  }
  ~iheap()
  {
    if (this->node_)
      delete[] this->node_;
    this->node_ = NULL;
  }

  inline int size()   { return this->cur_size_; }
  inline bool empty() { return this->cur_size_ == 0; }
  inline void reset() { this->cur_size_ = 0; }

  inline int push(T* node)
  {
    if (this->cur_size_ == this->max_size_ - 1)
      this->double_size();

    int pos = ++this->cur_size_;

    while (IHEAP_PARENT(pos) >= IHEAP_ROOT
           && *node < *this->node_[IHEAP_PARENT(pos)])
    {
      this->node_[pos] = this->node_[IHEAP_PARENT(pos)];
      pos = IHEAP_PARENT(pos);
    }
    this->node_[pos] = node;
    return 0;
  }
  inline T* pop()
  {
    if (this->empty()) return NULL;

    T* top = this->node_[IHEAP_ROOT];
    T* end = this->node_[this->cur_size_];
    --this->cur_size_;

    int pos = IHEAP_ROOT;
    int lchild = IHEAP_LCHILD(pos);
    int rchild = IHEAP_RCHILD(pos);
    while (1)
    {
      if (rchild <= this->cur_size_
          && *this->node_[rchild] < *end
          && *this->node_[rchild] < *this->node_[lchild])
      {
        this->node_[pos] = this->node_[rchild];
        pos = rchild;
      }else if (lchild <= this->cur_size_
               && *this->node_[lchild] < *end)
      {
        this->node_[pos] = this->node_[lchild];
        pos = lchild;
      }else
        break;
      
      lchild = IHEAP_LCHILD(pos);
      rchild = IHEAP_RCHILD(pos);
    }
    this->node_[pos] = end;
    return top;
  }
private:
  inline void double_size()
  {
    T** t = new T*[this->max_size_ * 2];
    memmove((char*)t, (char*)this->node_, sizeof(T*) * this->max_size_);
    this->max_size_ *= 2;
    delete[] this->node_;
    this->node_ = t;
  }
private:
  int cur_size_;
  int max_size_;

  T** node_;
};

#endif // IHEAP_H_

