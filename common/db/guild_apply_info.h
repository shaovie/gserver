// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-24 12:42
 */
//========================================================================

#ifndef GUILD_APPLY_INFO_H_
#define GUILD_APPLY_INFO_H_

// Lib header
#include <string.h>
#include <stdio.h>

#include "istream.h"

const char db_tb_guild_apply_info[] = "guild_apply";

// Forward declarations
/**
 * @class guild_apply_info
 *
 * @brief 公会成员
 */
class guild_apply_info
{
public:
  enum
  {
    IDX_GUILD_ID,
    IDX_CHAR_ID,
    IDX_APPLY_TIME,
    IDX_END
  };
  guild_apply_info() { reset(); }
public:
  void reset()
  {
    this->guild_id_   = 0;
    this->char_id_    = 0;
    this->apply_time_ = 0;
  }
public:
  static const char *all_col()
  { return "guild_id,char_id,apply_time"; }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,%d"
                      ")",
                      db_tb_guild_apply_info,
                      guild_apply_info::all_col(),
                      this->guild_id_, this->char_id_, this->apply_time_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where guild_id=%d and char_id=%d",
                      db_tb_guild_apply_info,
                      this->guild_id_, this->char_id_);
  }
  int delete_guild_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where guild_id=%d",
                      db_tb_guild_apply_info,
                      this->guild_id_);
  }
public:
  int guild_id_;   // 公会id
  int char_id_;    // 玩家id
  int apply_time_; // 申请时间
};
#endif // GUILD_APPLY_INFO_H_
