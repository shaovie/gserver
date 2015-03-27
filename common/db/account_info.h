// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-13 13:38
 */
//========================================================================

#ifndef ACCOUNT_INFO_H_
#define ACCOUNT_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "global_macros.h"

// Forward declarations

const char db_tb_account_info[] = "account_info";

/**
 * @class account_info
 * 
 * @brief
 */
class account_info
{
public:
  // track
  enum
  {
    TK_CREATE_AC_OK         = 1,          // 创建帐户成功
    TK_CREATE_CHAR_OK       = 3,          // 创建角色成功
    TK_ENTER_GAME_OK        = 5,          // 加载游戏画面成功
    TK_END
  };
  enum
  {
    IDX_ACCOUNT = 0, 
    IDX_TRACK_ST,
    IDX_DIAMOND, IDX_CHAR_ID,
    IDX_CHANNEL,
    IDX_C_TIME,
    IDX_END
  };
  account_info() { this->reset(); }
  
  void reset()
  {
    this->track_st_        = TK_CREATE_AC_OK;
    this->diamond_         = 0;
    this->char_id_         = 0;
    ::memset(this->account_, 0, sizeof(this->account_));
    ::memset(this->channel_, 0, sizeof(this->channel_));
  }
  void operator = (const account_info &v)
  {
    if (&v == this) return ;

    ::strncpy(this->account_, v.account_, sizeof(this->account_) - 1);
    ::strncpy(this->channel_, v.channel_, sizeof(this->channel_) - 1);
    this->diamond_   = v.diamond_;
    this->char_id_   = v.char_id_;
    this->c_time_    = v.c_time_;
  }
  static const char *all_col()
  { return "account,track_st,diamond,char_id,channel,c_time"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where account='%s'",
                      account_info::all_col(),
                      db_tb_account_info,
                      this->account_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "'%s',%d,%d,%d,'%s',%d"
                      ")",
                      db_tb_account_info,
                      account_info::all_col(),
                      this->account_, this->track_st_,
                      this->diamond_, this->char_id_,
                      this->channel_, this->c_time_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "update %s set track_st=%d,diamond=%d,char_id=%d where account='%s'",
                      db_tb_account_info,
                      this->track_st_,
                      this->diamond_, this->char_id_, this->account_);
  }
  int set_diamond_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set diamond=%d where account='%s'",
                      db_tb_account_info,
                      this->diamond_, this->account_);
  }

  char track_st_;                       // 帐号追踪状态
  int  diamond_;
  int  c_time_;
  int  char_id_;
  char channel_[MAX_CHANNEL_LEN + 1];
  char account_[MAX_ACCOUNT_LEN + 1];
};

#endif // ACCOUNT_INFO_H_

