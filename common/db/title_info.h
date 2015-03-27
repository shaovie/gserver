// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-25 11:21
 */
//========================================================================

#ifndef TITLE_INFO_H_
#define TITLE_INFO_H_

#include "global_macros.h"

// Lib header
#include <stdio.h>
#include "singleton.h"

// Forward declarations

const char db_tb_title_info[] = "title";

/**
 * @class title_info
 *
 * @brief
 */
class title_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_TITLE_CID,
    IDX_END
  };

  title_info() { this->reset(); }
  ~title_info() { }

  static const char *all_col()
  { return "char_id,title_cid"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      title_info::all_col(),
                      db_tb_title_info,
                      this->char_id_);
  }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) values("
                      "%d,%d"
                      ")",
                      db_tb_title_info,
                      title_info::all_col(),
                      this->char_id_, this->title_cid_);
  }

  void reset()
  {
    this->char_id_    = 0;
    this->title_cid_  = 0;
  }

  short title_cid_;         // 称号cid
  int   char_id_;           // 角色id
};

#endif  // TITLE_INFO_H_
