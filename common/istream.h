// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-17 16:56
 */
//========================================================================

#ifndef I_STREAM_H_
#define I_STREAM_H_

//= Lib header
#include "mblock.h"
#include <cstring>
#include <stdint.h>
#include <limits.h>

#include "def.h"

// Forward declarations
//
/**
 * @class proto_str
 *
 * @brief
 */
class stream_istr
{
  friend class in_stream;
public:
  stream_istr(char *bf, const unsigned int len)
    : str_len_(0), bf_len_(len), bf_(bf)
  { }

  char *str() const  { return this->bf_; }
  unsigned int bf_len()  const { return this->bf_len_;  }
  unsigned int str_len() const { return this->str_len_; }

private:
  unsigned int       str_len_;
  const unsigned int bf_len_;
  char              *bf_;
};
class stream_ostr
{
  friend class out_stream;
public:
  stream_ostr(const char *bf, const unsigned int len)
    : str_len_(len), bf_(bf)
  { }

  const char *str() const  { return this->bf_; }
  unsigned int str_len() const { return this->str_len_; }

private:
  unsigned int       str_len_;
  const char        *bf_;
};
/**
 * @class in_stream
 *
 * @brief
 */
class in_stream
{
public:
  in_stream(const char *bf, const unsigned int len)
    : bf_len_(len), rd_ptr_(0), bf_(bf) { }

  //= attribute method
  inline const char *rd_ptr() const
  { return this->bf_ + this->rd_ptr_; }
  inline void rd_ptr(const unsigned int val)
  { this->rd_ptr_ += val; }
  inline void set_rd_ptr(const unsigned int val)
  { this->rd_ptr_ = val; }

  inline const char *get_ptr()
  { return this->bf_; }

  inline unsigned int length() const
  { return this->bf_len_ - this->rd_ptr_; }

  //= stream operator
# define SHORT_CODE(T) in_stream & operator >>(T &v)     \
  {                                                      \
    if (sizeof(v) <= this->length())                     \
    {                                                    \
      v = *((T *)(this->bf_ + this->rd_ptr_));           \
      this->rd_ptr_ += sizeof(v);                        \
    }                                                    \
    return *this;                                        \
  }

  SHORT_CODE(char);
  SHORT_CODE(short);
  SHORT_CODE(int);
  SHORT_CODE(double);
  SHORT_CODE(int64_t);
#undef SHORT_CODE
  in_stream & operator >>(stream_istr &v)
  {
    if (this->length() >= sizeof(short))
    {
      unsigned short s_len = *((short *)(this->rd_ptr()));
      this->rd_ptr(sizeof(short));
      if (s_len <= this->length() && s_len < v.bf_len())
      {
        this->copy_in(v.bf_, s_len);
        v.bf_[s_len] = '\0';
        v.str_len_   = s_len;
      }
    }
    return *this;
  }
  in_stream & operator >>(mblock *v)
  {
    if (this->length() > sizeof(short))
    {
      unsigned short s_len = *((short *)(this->rd_ptr()));
      this->rd_ptr(sizeof(short));
      if (s_len <= this->length() && s_len < v->space())
      {
        v->copy(this->rd_ptr(), s_len);
        this->rd_ptr(s_len);
      }
    }
    return *this;
  }
protected:
  void copy_in(char *p, int len)
  {
    ::memcpy(p, this->bf_ + this->rd_ptr_, len);
    this->rd_ptr_ += len;
  }
private:
  unsigned int bf_len_;
  unsigned int rd_ptr_;

  const char *bf_;
};
/**
 * @class out_stream
 *
 * @brief
 */
class out_stream
{
public:
  out_stream(char *bf, unsigned int len)
    : bf_len_(len), wr_ptr_(0), bf_(bf) { }

  void reset(char *bf, unsigned int len)
  {
    this->bf_len_ = len;
    this->wr_ptr_ = 0;
    this->bf_     = bf;
  }
  //= attribute method
  inline char *wr_ptr() const
  { return this->bf_ + this->wr_ptr_; }
  inline void wr_ptr(unsigned int val)
  { this->wr_ptr_ += val; }
  inline void set_wr_ptr(unsigned int val)
  { this->wr_ptr_ = val; }

  inline char *get_ptr()
  { return this->bf_; }

  inline unsigned int length()
  { return this->wr_ptr_; }

  inline unsigned int space()
  { return this->bf_len_ - this->wr_ptr_; }

  //= stream operator
# define SHORT_CODE(T) out_stream & operator <<(const T v)\
  {                                                       \
    if (sizeof(v) <= this->space())                       \
    {                                                     \
      *((T *)(this->bf_ + this->wr_ptr_)) = v;            \
      this->wr_ptr_ += sizeof(v);                         \
    }                                                     \
    return *this;                                         \
  }

  SHORT_CODE(char);
  SHORT_CODE(short);
  SHORT_CODE(int);
  SHORT_CODE(double);
  SHORT_CODE(int64_t);
#undef SHORT_CODE

  out_stream & operator <<(const stream_ostr &v)
  {
    //assert(v.str_len() < 32766);
    if (sizeof(short) + v.str_len() <= this->space())
    {
      this->copy_out((const char *)&(v.str_len_), sizeof(short));
      this->copy_out(v.str(), v.str_len());
    }
    return *this;
  }
  out_stream & operator <<(in_stream &v)
  {
    if (v.length() <= this->space())
    {
      int l = v.length();
      this->copy_out(v.rd_ptr(), l);
      v.rd_ptr(l);
    }
    return *this;
  }
  out_stream & operator <<(mblock *v)
  {
    if (sizeof(short) + v->length() <= this->space())
    {
      int l = v->length();
      this->copy_out((const char *)&l, sizeof(short));
      this->copy_out(v->rd_ptr(), l);
      v->rd_ptr(l);
    }
    return *this;
  }
  out_stream & operator <<(proto_head &ph)
  { return *this << ph.seq_ << ph.id_ << ph.result_ << ph.len_; }

protected:
  void copy_out(const char *p, unsigned int len)
  {
    ::memcpy(this->bf_ + this->wr_ptr_, p, len);
    this->wr_ptr_ += len;
  }
private:
  unsigned int bf_len_;
  unsigned int wr_ptr_;

  char *bf_;
};

#endif // I_STREAM_H_

