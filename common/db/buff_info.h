// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-24 18:27
 */
//========================================================================

#ifndef BUFF_INFO_H_
#define BUFF_INFO_H_

#include "istream.h"
#include "global_macros.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_buff[] = "buff";

/**
 * @class buff_info
 * 
 * @brief
 */
class buff_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0, IDX_DATA,
    IDX_END
  };
  buff_info() { this->reset(); }

  void reset()
  {
    this->char_id_          = 0;
    ::memset(this->data_, 0, sizeof(this->data_));
  }
  void operator = (const buff_info &v)
  {
    if (&v == this) return ;

    this->char_id_ = v.char_id_;
    ::memcpy(this->data_, v.data_, sizeof(this->data_));
  }

  static const char *all_col() { return "char_id,data"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where char_id=%d",
                      buff_info::all_col(),
                      db_tb_buff,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values(%d,'%s')",
                      db_tb_buff,
                      buff_info::all_col(),
                      this->char_id_, this->data_);
  }

  int  char_id_;
  char data_[MAX_BUFF_DATA_LEN + 1];
};
inline out_stream & operator << (out_stream &os, const buff_info *info)
{
  os << info->char_id_
    << stream_ostr(info->data_, ::strlen(info->data_));
  return os;
}
inline in_stream & operator >> (in_stream &is, buff_info *info)
{
  stream_istr si(info->data_, sizeof(info->data_));
  is >> info->char_id_ >> si;
  return is;
}
#endif // BUFF_INFO_H_

