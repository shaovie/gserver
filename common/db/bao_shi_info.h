// -*- C++ -*-

//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-24 16:19
 */
//========================================================================

#ifndef BAO_SHI_INFO_H_
#define BAO_SHI_INFO_H_

#include "istream.h"
#include "global_macros.h"

// Lib header

const char db_tb_bao_shi_info[] = "bao_shi";

/**
 * @class bao_shi_info
 *
 * @brief
 */
class bao_shi_info
{
public:
  enum
  {
    IDX_CHAR_ID,
    IDX_POS,
    IDX_BS_IDX,
    IDX_BS_LVL,
    IDX_UP_CNT,
    IDX_BS_EXP,
    IDX_END
  };
  bao_shi_info() { reset(); }
public:
  void reset()
  {
    this->char_id_ = 0;
    this->pos_     = 0;
    this->bs_idx_  = 0;
    this->bs_lvl_  = 0;
    this->up_cnt_  = 0;
    this->bs_exp_  = 0;
  }
public:
  static const char *all_col()
  { return "char_id,pos,bs_idx,bs_lvl,up_cnt,bs_exp"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      bao_shi_info::all_col(),
                      db_tb_bao_shi_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,"
                      "%d,%d,"
                      "%d,%d"
                      ")",
                      db_tb_bao_shi_info,
                      bao_shi_info::all_col(),
                      this->char_id_, this->pos_,
                      this->bs_idx_, this->bs_lvl_,
                      this->up_cnt_,
                      this->bs_exp_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "bs_lvl=%d,up_cnt=%d,bs_exp=%d"
                      " where char_id=%d and pos=%d and bs_idx=%d",
                      db_tb_bao_shi_info,
                      this->bs_lvl_, this->bs_exp_,
                      this->up_cnt_,
                      this->char_id_, this->pos_, this->bs_idx_);
  }
public:
  char  pos_;     // 部位
  char  bs_idx_;  // 第几个
  short bs_lvl_;  // 宝石等级
  short up_cnt_;  // 当前等级提升次数
  int   char_id_; // 玩家ID
  int   bs_exp_;  // 当前等级经验
};
#endif  //BAO_SHI_INFO_H_
