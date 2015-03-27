// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-06 13:58
 */
//========================================================================

#ifndef TASK_BIT_ARRAY_H_
#define TASK_BIT_ARRAY_H_

#include "istream.h"
#include "global_macros.h"

// Lib header
#include <string.h>

// Forward declarations

const char db_tb_task_bit_array[] = "task_bit_array";
/**
 * @class task_bit_array
 * 
 * @brief
 */
class task_bit_array
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_TRUNK_BIT_ID,
    IDX_BRANCH_BIT_ID,
    IDX_END
  };
  task_bit_array(int char_id = 0) { this->reset(); this->char_id_ = char_id; }
  
  void reset()
  {
    this->char_id_           = 0;
    this->len_[0]            = 0;
    this->len_[1]            = 0;
    ::memset(this->bit_[0],  0, sizeof(this->bit_[0]));
    ::memset(this->bit_[1],  0, sizeof(this->bit_[1]));
  }
  static const char *all_col()
  { return "char_id,trunk_bit,branch_bit"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      task_bit_array::all_col(),
                      db_tb_task_bit_array,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s)values(%d,'%s','%s')",
                      db_tb_task_bit_array,
                      task_bit_array::all_col(),
                      this->char_id_, this->bit_[0], this->bit_[1]);
  }
  void operator = (const task_bit_array &tba)
  {
    if (&tba == this) return ;

    this->char_id_           = tba.char_id_;
    this->len_[0]            = tba.len_[0];
    this->len_[1]            = tba.len_[1];
    ::memcpy(this->bit_[0],  tba.bit_[0], sizeof(this->bit_[0]));
    ::memcpy(this->bit_[1],  tba.bit_[1], sizeof(this->bit_[1]));
  }

  bool in_set(const int which, const int idx)
  { return this->bit_[which][idx] == '1'; }

  void set(const int which, const int idx)
  { 
    char *ptr = this->bit_[which] + this->len_[which];
    int empty_len = idx - this->len_[which] + 1;

    if (idx >= this->len_[which])
    {
      ::memset(ptr, '0', empty_len);
      this->len_[which] = idx + 1;
    }

    this->bit_[which][idx] = '1';
  }
public:
  int     char_id_;                   // 所属角色ID
  int     len_[2];                    // '1'的最大位置+1 not store db
  char    bit_[2][HALF_TASK_NUM + 1]; //
};
inline out_stream & operator << (out_stream &os, const task_bit_array *info)
{
  os << info->char_id_
    << info->len_[0] << info->len_[1]
    << stream_ostr(info->bit_[0], info->len_[0])
    << stream_ostr(info->bit_[1], info->len_[1]);
  return os;
}
inline in_stream & operator >> (in_stream &is, task_bit_array *info)
{
  stream_istr bit_0_si(info->bit_[0], HALF_TASK_NUM + 1);
  stream_istr bit_1_si(info->bit_[1], HALF_TASK_NUM + 1);
  is >> info->char_id_
    >> info->len_[0] >> info->len_[1]
    >> bit_0_si
    >> bit_1_si;
  return is;
}
#endif // TASK_BIT_ARRAY_H_

