// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-24 18:27
 */
//========================================================================

#ifndef SYS_SETTINGS_H_
#define SYS_SETTINGS_H_

#include "istream.h"
#include "global_macros.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_sys_settings[] = "sys_settings";

/**
 * @class sys_settings
 * 
 * @brief
 */
class sys_settings
{
public:
  enum
  {
    IDX_CHAR_ID = 0, IDX_DATA,
    IDX_END
  };
  sys_settings() { this->reset(); }

  void reset()
  {
    this->char_id_ = 0;
    ::memset(this->data_, 0, sizeof(this->data_));
  }
  void operator = (const sys_settings &v)
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
                      sys_settings::all_col(),
                      db_tb_sys_settings,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values(%d,'%s')",
                      db_tb_sys_settings,
                      sys_settings::all_col(),
                      this->char_id_, this->data_);
  }

  int  char_id_;
  char data_[MAX_SYS_SETTINGS_LEN + 1];
};
inline out_stream & operator << (out_stream &os, const sys_settings *info)
{
  os << info->char_id_
    << stream_ostr(info->data_, ::strlen(info->data_));
  return os;
}
inline in_stream & operator >> (in_stream &is, sys_settings *info)
{
  stream_istr d_si(info->data_, sizeof(info->data_));
  is >> info->char_id_ >> d_si;
  return is;
}
#endif // SYS_SETTINGS_H_

