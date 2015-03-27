// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-04-11 11:38
 */
//========================================================================

#ifndef CHAR_ATTR_H_
#define CHAR_ATTR_H_

// Lib header
#include <string.h>
#include "def.h"

// Forward declarations

const char db_tb_char_attr[] = "char_attr";

/**
 * @class char_attr
 *
 * @brief
 */
class char_attr
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_HP, IDX_MP,
    IDX_GONG_JI, IDX_FANG_YU,
    IDX_MING_ZHONG, IDX_SHAN_BI,
    IDX_BAO_JI, IDX_KANG_BAO,
    IDX_SHANG_MIAN,
    IDX_ZHU_WU, IDX_FU_WU,
    IDX_END
  };
  char_attr() { this->reset(); }

  void reset()
  {
    this->char_id_ = 0;
    ::memset(this->attr_, 0, sizeof(this->attr_));
    this->zhu_wu_  = 0;
    this->fu_wu_   = 0;
  }
  static const char *all_col()
  {
    return "char_id,hp,mp,gong_ji,fang_yu,"
      "ming_zhong,shan_bi,bao_ji,kang_bao,shang_mian,zhu_wu,fu_wu";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where char_id=%d",
                      char_attr::all_col(),
                      db_tb_char_attr,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values("
                      "%d,"
                      "%d,%d,%d,%d,"
                      "%d,%d,%d,%d,"
                      "%d,%d,%d"
                      ")",
                      db_tb_char_attr,
                      char_attr::all_col(),
                      this->char_id_,
                      this->attr_[ATTR_T_HP], this->attr_[ATTR_T_MP],
                      this->attr_[ATTR_T_GONG_JI], this->attr_[ATTR_T_FANG_YU],
                      this->attr_[ATTR_T_MING_ZHONG], this->attr_[ATTR_T_SHAN_BI],
                      this->attr_[ATTR_T_BAO_JI], this->attr_[ATTR_T_KANG_BAO],
                      this->attr_[ATTR_T_SHANG_MIAN],
                      this->zhu_wu_, this->fu_wu_);
  }
public:
  int char_id_;               // 玩家ID
  int attr_[ATTR_T_ITEM_CNT]; // 属性值
  int zhu_wu_;
  int fu_wu_;
};

#endif // CHAR_ATTR_H_

