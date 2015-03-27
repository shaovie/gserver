// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-23 16:19
 */
//========================================================================

#ifndef GUILD_INFO_H_
#define GUILD_INFO_H_

#include "istream.h"
#include "global_macros.h"

// Lib header
#include <stdio.h>
#include <string.h>

const char db_tb_guild_info[] = "guild";

/**
 * @class guild_info
 *
 * @brief
 */
class guild_info
{
public:
  enum
  {
    IDX_GUILD_ID,
    IDX_NAME,
    IDX_CHAIRMAN_ID,IDX_LVL,
    IDX_PURPOSE,
    IDX_C_TIME,
    IDX_APPLY_DJ_LIMIT,
    IDX_GUILD_RESOURCE,
    IDX_ZQ_LVL,
    IDX_TLD_LVL,
    IDX_JBD_LVL,
    IDX_IS_GHZ_WINNER,
    IDX_LAST_SUMMON_BOSS_TIME,
    IDX_APPLY_ZL_LIMIT,
    IDX_SCP_LVL,
    IDX_SCP_LAST_OPEN_TIME,
    IDX_END
  };
  guild_info() { reset(); }
public:
  void reset()
  {
    this->lvl_                   = 0;
    this->guild_id_              = 0;
    this->chairman_id_           = 0;
    this->c_time_                = 0;
    this->apply_dj_limit_        = 0;
    this->guild_resource_        = 0;
    this->zq_lvl_                = 0;
    this->tld_lvl_               = 0;
    this->jbd_lvl_               = 0;
    this->is_ghz_winner_         = 0;
    this->last_summon_boss_time_ = 0;
    this->apply_zl_limit_        = 0;
    this->scp_lvl_               = 0;
    this->scp_last_open_time_    = 0;
    ::memset(this->name_, 0, sizeof(this->name_));
    ::memset(this->purpose_, 0, sizeof(this->purpose_));
  }
public:
  static const char *all_col()
  {
    return "guild_id,name,chairman_id,lvl,"
      "purpose,c_time,"
      "apply_dj_limit,guild_resource,"
      "zq_lvl,tld_lvl,jbd_lvl,is_ghz_winner,last_summon_boss_time,apply_zl_limit,"
      "scp_lvl,scp_last_open_time";
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,'%s',"
                      "%d,%d,"
                      "'%s',%d,"
                      "%d,%d,"
                      "%d,%d,%d,%d,"
                      "%d,%d,"
                      "%d,%d"
                      ")",
                      db_tb_guild_info,
                      guild_info::all_col(),
                      this->guild_id_, this->name_,
                      this->chairman_id_, this->lvl_,
                      this->purpose_, this->c_time_,
                      this->apply_dj_limit_, this->guild_resource_,
                      this->zq_lvl_, this->tld_lvl_, this->jbd_lvl_, this->is_ghz_winner_,
                      this->last_summon_boss_time_, this->apply_zl_limit_,
                      this->scp_lvl_, this->scp_last_open_time_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where guild_id=%d",
                      db_tb_guild_info,
                      this->guild_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "name='%s',chairman_id=%d,"
                      "lvl=%d,purpose='%s',"
                      "c_time=%d,"
                      "apply_dj_limit=%d,guild_resource=%d,"
                      "zq_lvl=%d,tld_lvl=%d,jbd_lvl=%d,is_ghz_winner=%d,"
                      "last_summon_boss_time=%d,apply_zl_limit=%d,"
                      "scp_lvl=%d,scp_last_open_time=%d"
                      " where guild_id=%d",
                      db_tb_guild_info,
                      this->name_, this->chairman_id_,
                      this->lvl_, this->purpose_,
                      this->c_time_,
                      this->apply_dj_limit_, this->guild_resource_,
                      this->zq_lvl_, this->tld_lvl_, this->jbd_lvl_, this->is_ghz_winner_,
                      this->last_summon_boss_time_, this->apply_zl_limit_,
                      this->scp_lvl_, this->scp_last_open_time_,
                      this->guild_id_);
  }
public:
  char  lvl_;                                  // 大厅等级
  char  zq_lvl_;                               // 战旗等级
  char  tld_lvl_;                              // 屠龙殿等级
  char  jbd_lvl_;                              // 聚宝袋等级
  char  is_ghz_winner_;                        // 是否是公会战胜利者
  char  scp_lvl_;                              // 公会副本等级
  short apply_dj_limit_;                       // 等级限制
  int   guild_id_;                             // 公会id
  int   chairman_id_;                          // 会长
  int   c_time_;                               // 创建时间
  int   apply_zl_limit_;                       // 战力限制
  int   guild_resource_;                       // 资源
  int   last_summon_boss_time_;                // 上次召唤boss时间
  int   scp_last_open_time_;                   // 本周开启公会副本
  char  name_[MAX_NAME_LEN + 1];               // 公会名
  char  purpose_[MAX_GUILD_PURPOSE_LEN + 1];   // 宗旨
};
inline out_stream & operator << (out_stream &os, const guild_info *info)
{
  stream_ostr name((const char *)info->name_, ::strlen(info->name_));
  stream_ostr purpose((const char *)info->purpose_, ::strlen(info->purpose_));
  os << info->lvl_
    << info->guild_id_
    << info->chairman_id_
    << info->c_time_
    << info->apply_dj_limit_
    << info->guild_resource_
    << info->zq_lvl_
    << info->tld_lvl_
    << info->jbd_lvl_
    << info->is_ghz_winner_
    << info->apply_zl_limit_
    << info->last_summon_boss_time_
    << info->scp_lvl_
    << info->scp_last_open_time_
    << name
    << purpose;
  return os;
}
inline in_stream & operator >> (in_stream &is, guild_info *info)
{
  stream_istr name(info->name_, sizeof(info->name_));
  stream_istr purpose(info->purpose_, sizeof(info->purpose_));
  is >> info->lvl_
    >> info->guild_id_
    >> info->chairman_id_
    >> info->c_time_
    >> info->apply_dj_limit_
    >> info->guild_resource_
    >> info->zq_lvl_
    >> info->tld_lvl_
    >> info->jbd_lvl_
    >> info->is_ghz_winner_
    >> info->apply_zl_limit_
    >> info->last_summon_boss_time_
    >> info->scp_lvl_
    >> info->scp_last_open_time_
    >> name
    >> purpose;
  return is;
}
#endif  //GUILD_INFO_H_
