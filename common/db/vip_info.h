// -*- C++ -*-

//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-17 16:19
 */
//========================================================================

#ifndef VIP_INFO_H_
#define VIP_INFO_H_

#include "istream.h"
#include "global_macros.h"

// Lib header
#include <stdio.h>
#include <string.h>

const char db_tb_vip_info[] = "vip";

/**
 * @class vip_info
 *
 * @brief
 */
class vip_info
{
public:
  enum
  {
    IDX_CHAR_ID,
    IDX_VIP_LVL,
    IDX_VIP_EXP,
    IDX_GUILD_JB_CNT,
    IDX_BUY_TI_LI_CNT,
    IDX_EXCHANGE_COIN_CNT,
    IDX_FREE_RELIVE_CNT,
    IDX_GET_IDXS,
    IDX_END
  };
  vip_info() { reset(); }
public:
  void reset()
  {
    this->char_id_       = 0;
    this->vip_lvl_       = 0;
    this->vip_exp_       = 0;
    this->guild_jb_cnt_  = 0;
    this->buy_ti_li_cnt_ = 0;
    this->exchange_coin_cnt_ = 0;
    this->free_relive_cnt_ = 0;
    ::memset(this->get_idxs_, 0, sizeof(this->get_idxs_));
  }
  void operator = (const vip_info &info)
  {
    if (&info == this) return ;

    this->char_id_ = info.char_id_;
    this->vip_lvl_ = info.vip_lvl_;
    this->vip_exp_ = info.vip_exp_;
    this->guild_jb_cnt_ = info.guild_jb_cnt_;
    this->buy_ti_li_cnt_= info.buy_ti_li_cnt_;
    this->exchange_coin_cnt_ = info.exchange_coin_cnt_;
    this->free_relive_cnt_ = info.free_relive_cnt_;
    ::strncpy(this->get_idxs_, info.get_idxs_, sizeof(this->get_idxs_) - 1);
  }
public:
  static const char *all_col()
  { return "char_id,vip_lvl,vip_exp,guild_jb_cnt,buy_ti_li_cnt,exchange_coin_cnt,free_relive_cnt,get_idxs"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      vip_info::all_col(),
                      db_tb_vip_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,"
                      "%d,%d,%d,%d,%d,"
                      "'%s'"
                      ")",
                      db_tb_vip_info,
                      vip_info::all_col(),
                      this->char_id_, this->vip_lvl_,
                      this->vip_exp_, this->guild_jb_cnt_, this->buy_ti_li_cnt_, this->exchange_coin_cnt_, this->free_relive_cnt_,
                      this->get_idxs_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "vip_lvl=%d,"
                      "vip_exp=%d,guild_jb_cnt=%d,buy_ti_li_cnt=%d,exchange_coin_cnt=%d,free_relive_cnt=%d,"
                      "get_idxs='%s'"
                      " where char_id=%d",
                      db_tb_vip_info,
                      this->vip_lvl_,
                      this->vip_exp_, this->guild_jb_cnt_, this->buy_ti_li_cnt_, this->exchange_coin_cnt_, this->free_relive_cnt_,
                      this->get_idxs_,
                      this->char_id_);
  }
public:
  char  vip_lvl_;                               // vip等级
  char  guild_jb_cnt_;                          // 公会聚宝次数
  char  buy_ti_li_cnt_;                         // 购买体力值次数
  char  exchange_coin_cnt_;                     // 兑换金币次数
  char  free_relive_cnt_;                       // 免费复活次数
  int   char_id_;                               // 玩家ID
  int   vip_exp_;                               // vip经验
  char  get_idxs_[MAX_VIP_LVL + 1];             // vip等级奖励领取
};
inline out_stream & operator << (out_stream &os, const vip_info *info)
{
  stream_ostr idxs((const char *)info->get_idxs_, ::strlen(info->get_idxs_));
  os << info->char_id_
    << info->vip_lvl_
    << info->vip_exp_
    << info->guild_jb_cnt_
    << info->buy_ti_li_cnt_
    << info->exchange_coin_cnt_
    << info->free_relive_cnt_
    << idxs;
  return os;
}
inline in_stream & operator >> (in_stream &is, vip_info *info)
{
  stream_istr idxs(info->get_idxs_, sizeof(info->get_idxs_));
  is >> info->char_id_
    >> info->vip_lvl_
    >> info->vip_exp_
    >> info->guild_jb_cnt_
    >> info->buy_ti_li_cnt_
    >> info->exchange_coin_cnt_
    >> info->free_relive_cnt_
    >> idxs;
  return is;
}
#endif  //VIP_INFO_H_
