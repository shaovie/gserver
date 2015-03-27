// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-24 12:42
 */
//========================================================================

#ifndef GUILD_MEMBER_INFO_H_
#define GUILD_MEMBER_INFO_H_

// Lib header
#include <string.h>
#include <stdio.h>

#include "istream.h"

const char db_tb_guild_member_info[] = "guild_member";

// Forward declarations
/**
 * @class guild_member_info
 *
 * @brief 公会成员
 */
class guild_member_info
{
public:
  enum // 职位
  {
    POS_NULL = 0,
    POS_HZ   = 1, // 会长
    POS_FH   = 2, // 副会长
    POS_YL   = 3, // 元老
    POS_HY   = 4, // 会员
    POS_END,
  };
public:
  enum
  {
    IDX_CHAR_ID,
    IDX_GUILD_ID,
    IDX_CONTRIB,
    IDX_TOTAL_CONTRIB,
    IDX_POSITION,
    IDX_END
  };
  guild_member_info() { reset(); }
public:
  void reset()
  {
    this->char_id_       = 0;
    this->guild_id_      = 0;
    this->position_      = 0;
    this->contrib_       = 0;
    this->total_contrib_ = 0;
  }
public:
  static const char *all_col()
  { return "char_id,guild_id,contrib,total_contrib,position"; }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,"
                      "%d,%d,"
                      "%d"
                      ")",
                      db_tb_guild_member_info,
                      guild_member_info::all_col(),
                      this->char_id_, this->guild_id_,
                      this->contrib_, this->total_contrib_,
                      this->position_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where char_id=%d",
                      db_tb_guild_member_info,
                      this->char_id_);
  }
  int delete_guild_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where guild_id=%d",
                      db_tb_guild_member_info,
                      this->guild_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "contrib=%d,total_contrib=%d,position=%d"
                      " where char_id=%d",
                      db_tb_guild_member_info,
                      this->contrib_, this->total_contrib_, this->position_,
                      this->char_id_);
  }
public:
  char position_;       // 职位
  int  char_id_;        // 玩家id
  int  guild_id_;       // 公会id
  int  contrib_;        // 当前贡献
  int  total_contrib_;  // 历史贡献
};
#endif // GUILD_MEMBER_INFO_H_
