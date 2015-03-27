// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-25 14:48
 */
//========================================================================

#ifndef BAOWU_MALL_INFO_H_
#define BAOWU_MALL_INFO_H_

#include "global_macros.h"

// Lib header
#include <string.h>
#include "singleton.h"

// Forward declarations

const char db_tb_baowu_mall_info[] = "baowu_mall";

/**
 * @class baowu_mall_info
 * 
 * @brief
 */
class baowu_mall_info
{
  friend class obj_pool_std_allocator<baowu_mall_info>;
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_REFRESH_TIME,
    IDX_ITEM_INFO,
    IDX_END
  };
  baowu_mall_info() { reset(); }
  ~baowu_mall_info() { }

  static const char *all_col()
  { return "char_id,refresh_time,item_info"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      baowu_mall_info::all_col(),
                      db_tb_baowu_mall_info,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    char str[128] = {0};
    int len = 0;
    for (int i = 0; i < BAOWU_MALL_ITEM_COUNT; i++)
    {
      len += ::snprintf(str + len, sizeof(str) - len,
                        "%d:%d,",
                        this->item_info_[i][0],
                        this->item_info_[i][1]);
    }
    if (len > 0)
      str[len - 1] = '\0';
    else
      str[len] = '\0';

    return ::snprintf(bf, bf_len,
                      "replace into %s(%s) values("
                      "%d,%d,'%s'"
                      ")",
                      db_tb_baowu_mall_info,
                      baowu_mall_info::all_col(),
                      this->char_id_, this->refresh_time_, str);
  }

  void reset()
  {
    this->char_id_          = 0;
    this->refresh_time_     = 0;
    ::memset(this->item_info_, 0, sizeof(this->item_info_));
  }

  int char_id_;                     // 角色id
  int refresh_time_;                // 上次刷新时间
  int item_info_[BAOWU_MALL_ITEM_COUNT][2];   // 刷新物品信息
};

#endif  // BAOWU_MALL_INFO_H_
