// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 02:24
 */
//========================================================================

#ifndef MBLOCK_H_
#define MBLOCK_H_

#include <cstdio>
#include <cstring>
#include <cassert>
#include <stdint.h>

/**
 * @class mblock
 *
 * @brief 
 */
class mblock
{
public:
  mblock(const int size) :
    release_data_(true),
    mb_size_(size),
    mb_type_(0),
    rd_ptr_(0),
    wr_ptr_(0),
    mb_data_(new char[size]),
    prev_(NULL),
    next_(NULL)
  { }

  mblock(char *data_ptr, const int size) :
    release_data_(false),
    mb_size_(size),
    mb_type_(0),
    rd_ptr_(0),
    wr_ptr_(0),
    mb_data_(data_ptr),
    prev_(NULL),
    next_(NULL)
  { }

  ~mblock()
  { if (this->release_data_) assert(0); }

  inline char *data(void) const { return this->mb_data_; }

  inline int size(void) const { return this->mb_size_; }

  inline int length(void) const { return this->wr_ptr_ - this->rd_ptr_; }

  inline int space(void) const
  { return this->mb_size_ > 0 ? this->mb_size_ - this->wr_ptr_ : 0; }

  inline void copy(const char *buf, int len)
  {
    if (len <= this->space())
    {
      std::memcpy(this->wr_ptr(), buf, len);
      this->wr_ptr(len);
    }
  }

  inline int data_type(void) const { return this->mb_type_; }
  inline void data_type(const int type) { this->mb_type_ = type; }

  inline mblock *next(void) const { return this->next_; }
  inline void next(mblock *mb)    { this->next_ = mb; }

  inline mblock *prev(void) const { return this->prev_; }
  inline void prev(mblock *mb)    { this->prev_ = mb; }

  inline char *rd_ptr(void) const { return this->mb_data_ + this->rd_ptr_; }
  inline void rd_ptr(const int n) { this->rd_ptr_ += n; }

  inline char *wr_ptr(void) const { return this->mb_data_ + this->wr_ptr_; }
  inline void wr_ptr(const int n) { this->wr_ptr_ += n; }

  inline void reset(void) { this->rd_ptr_ = this->wr_ptr_ = 0; }

  inline void release(void)
  {
    for (mblock *mt = NULL, *mb = this; mb != NULL; )
    {
      mt = mb; 
      mb = mb->next(); 
      mt->clean();
      delete mt;
    }
  }

  inline void clean(void)
  {
    if (this->mb_data_ && this->release_data_)
    {
      ::delete []this->mb_data_;
      this->mb_data_ = NULL;
      this->mb_size_ = 0;
      this->release_data_ = false;
    }
  }

  // set '\0' to the special pos.
  // if <pos> is -1, the pos equal mblock::wr_ptr().
  inline void set_eof(const int pos = -1)
  {
    if (pos == -1)
      this->mb_data_[this->wr_ptr_] = '\0';
    else
      this->mb_data_[pos] = '\0';
  }

  // numeric operators
#define MBLOCK_READ_OPERATOR(type)                    \
  inline mblock & operator >> (type &v)               \
  {                                                   \
    if (this->length() >= (int)sizeof(type))          \
    {                                                 \
      std::memcpy(&v, this->rd_ptr(), sizeof(type));  \
      this->rd_ptr(sizeof(type));                     \
    }                                                 \
    return *this;                                     \
  }

#define MBLOCK_WRITE_OPERATOR(type)                   \
  inline mblock & operator << (const type &v)         \
  {                                                   \
    if (this->space() >= (int)sizeof(type))           \
    {                                                 \
      std::memcpy(this->wr_ptr(), &v, sizeof(type));  \
      this->wr_ptr(sizeof(type));                     \
    }                                                 \
    return *this;                                     \
  }

  MBLOCK_READ_OPERATOR(char);
  MBLOCK_READ_OPERATOR(short);
  MBLOCK_READ_OPERATOR(int);
  MBLOCK_READ_OPERATOR(int64_t);
  MBLOCK_READ_OPERATOR(float);
  MBLOCK_READ_OPERATOR(double);

  MBLOCK_WRITE_OPERATOR(char);
  MBLOCK_WRITE_OPERATOR(short);
  MBLOCK_WRITE_OPERATOR(int);
  MBLOCK_WRITE_OPERATOR(int64_t);
  MBLOCK_WRITE_OPERATOR(float);
  MBLOCK_WRITE_OPERATOR(double);
#undef MBLOCK_READ_OPERATOR
#undef MBLOCK_WRITE_OPERATOR
protected:
  bool  release_data_;
  
  int	mb_size_;
  int mb_type_;
  int rd_ptr_;
  int wr_ptr_;

  char *mb_data_;
  mblock *prev_;
  mblock *next_;
private:
  // = Disallow these operations
  mblock &operator =(const mblock & );

  mblock(const mblock &);
};

#endif // MBLOCK_H_

