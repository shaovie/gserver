// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-08-05 15:06
 */
//========================================================================

#ifndef WATER_TREE_INFO_H_
#define WATER_TREE_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>

// Forward declarations

const char db_tb_water_tree[] = "water_tree";

/**
 * @class water_tree_info
 * 
 * @brief
 */
class water_tree_info
{
public:
  enum
  {
    IDX_LVL = 0,
    IDX_CHENG_ZHANG,
    IDX_END
  };
  water_tree_info() { reset(); }

  void reset()
  {
    this->lvl_          = 0;
    this->cheng_zhang_  = 0;
  }

  static const char *all_col()
  { return "lvl,cheng_zhang"; }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set lvl=%d,cheng_zhang=%d",
                      db_tb_water_tree,
                      this->lvl_, this->cheng_zhang_);
  }

  int lvl_;           // 永恒之树等级
  int cheng_zhang_;   // 永恒之树成长
};

#endif  // WATER_TREE_INFO_H_

