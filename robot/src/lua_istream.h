// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2013-12-17 15:22
 */
//========================================================================

#ifndef LUA_ISTREAM_H_
#define LUA_ISTREAM_H_

//= Lib header
#include "istream.h"
#include "client.h"

// Forward declarations
//
/**
 * @class lua_in_stream
 * 
 * @brief
 */
class lua_in_stream
{
public:
  static const char *rd_ptr(in_stream &is)
  { return is.rd_ptr(); }

  static char read_char(in_stream &is)
  { char value = 0; is >> value; return value; }
  static short read_short(in_stream &is)
  { short value = 0; is >> value; return value; }
  static int read_int(in_stream &is)
  { int value = 0; is >> value; return value; }
  static double read_double(in_stream &is)
  { double value = 0; is >> value; return value; }
  static int64_t read_int64_t(in_stream &is)
  { int64_t value = 0; is >> value; return value; }
  static char *read_string(in_stream &is, const int len)
  {
    static char bf[2048] = {0};
    stream_istr si(bf, len);
    is >> si;
    return bf;
  }
};
/**
 * @class lua_out_stream
 * 
 * @brief
 */
class lua_out_stream
{
public:
  static out_stream new_out_stream()
  {
    out_stream os(client::send_buf, client::send_buf_len);
    return os;
  }
  static void write_char(out_stream &os, const char value)
  { os << value; }
  static void write_short(out_stream &os, const short value)
  { os << value; }
  static void write_int(out_stream &os, const int value)
  { os << value; }
  static void write_double(out_stream &os, const double value)
  { os << value; }
  static void write_int64_t(out_stream &os, const int64_t value)
  { os << value; }
  static void write_string(out_stream &os, const char *bf)
  { stream_ostr so(bf, ::strlen(bf)); os << so; }
};

#endif // LUA_ISTREAM_H_

