// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-15 10:46
 */
//========================================================================

#ifndef DXC_INFO_H_
#define DXC_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "obj_pool.h"
#include "global_macros.h"

// Forward declarations

const char db_tb_dxc_info[] = "dxc_info";

/**
 * @class dxc_info
 *
 * @brief
 */
class dxc_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_SCENE_CID,
    IDX_GRADE,
    IDX_END
  };

  dxc_info() { this->reset(); }

  void reset()
  {
    this->char_id_   = 0;
    this->scene_cid_ = 0;
    this->grade_     = 0;
  }
  void operator = (const dxc_info &di)
  {
    if (&di == this) return ;

    this->char_id_   = di.char_id_;
    this->scene_cid_ = di.scene_cid_;
    this->grade_     = di.grade_;
  }
  static const char *all_col()
  { return "char_id,scene_cid,grade"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      dxc_info::all_col(),
                      db_tb_dxc_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,%d"
                      ")",
                      db_tb_dxc_info, dxc_info::all_col(),
                      this->char_id_, this->scene_cid_, this->grade_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "grade=%d"
                      " where char_id=%d and scene_cid=%d",
                      db_tb_dxc_info, this->grade_,
                      this->char_id_, this->scene_cid_);
  }

  char grade_;
  int  char_id_;
  int  scene_cid_;
};
#endif // DXC_INFO_H_

