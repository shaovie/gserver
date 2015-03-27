// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-15 14:42
 */
//========================================================================

#ifndef GUILD_SKILL_INFO_H_
#define GUILD_SKILL_INFO_H_

#include "singleton.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_guild_skill_info[] = "guild_skill";

/**
 * @class guild_skill_info
 *
 * @brief
 */
class guild_skill_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_LVL,
    IDX_END
  };
public:
  guild_skill_info() { this->reset(); }

  static const char *all_col() { return "char_id,cid,lvl"; }
  void reset()
  {
    this->char_id_        = 0;
    this->cid_            = 0;
    this->lvl_            = 1;
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values(%d,%d,%d)",
                      db_tb_guild_skill_info,
                      guild_skill_info::all_col(),
                      this->char_id_, this->cid_, this->lvl_);
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      guild_skill_info::all_col(),
                      db_tb_guild_skill_info,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set lvl=%d where char_id=%d and cid=%d",
                      db_tb_guild_skill_info,
                      this->lvl_,
                      this->char_id_, this->cid_);
  }

  short   lvl_;
  int     char_id_;
  int     cid_;
};
#endif // GUILD_SKILL_INFO_H_
