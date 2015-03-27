// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-04-09 15:02
 */
//========================================================================

#ifndef RANK_DATA_H_
#define RANK_DATA_H_

// Lib header
#include <stdint.h>

// Forward declarations
class out_stream;

//------------------------------------------------------------------
// structure for each rank
// must supply
//   copy constructor
//   operator ==   判断玩家是否已在排行榜中
//   operator >    用于排序
//   build_info
class rank_data
{
public:
  virtual ~rank_data(){}
  virtual void build_info(out_stream &os);
  virtual int id() { return this->char_id_; }

  int char_id_;
  int value_;
};

class lvl_rank_data : public rank_data
{
public:
  bool operator == (const lvl_rank_data &data) const
  { return (this->char_id_ == data.char_id_); }
  bool operator > (const lvl_rank_data &data) const
  {
    if (this->value_ == data.value_)
    {
      if (this->exp_ == data.exp_)
        return this->char_id_ < data.char_id_;
      return (this->exp_ > data.exp_);
    }
    return this->value_ > data.value_;
  }

  int64_t exp_;
};

class zhanli_rank_data : public rank_data
{
public:
  bool operator == (const zhanli_rank_data &data) const
  { return (this->char_id_ == data.char_id_); }
  bool operator > (const zhanli_rank_data &data) const
  {
    if (this->value_ == data.value_)
      return (this->char_id_ < data.char_id_);
    return this->value_ > data.value_;
  }
};

class mstar_rank_data : public rank_data
{
public:
  virtual void build_info(out_stream &os);
  bool operator == (const mstar_rank_data &data) const
  { return (this->char_id_ == data.char_id_); }
  bool operator > (const mstar_rank_data &data) const
  {
    if (this->value_ == data.value_)
      return (this->char_id_ < data.char_id_);
    return this->value_ > data.value_;
  }
};

#endif // RANK_DATA_H_
