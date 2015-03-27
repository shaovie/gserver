// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-31 10:02
 */
//========================================================================

#ifndef MALL_BUY_LOG_H_
#define MALL_BUY_LOG_H_ 

// Lib header
#include <stdio.h>
#include <string.h>

#include "global_macros.h"

const char db_tb_mall_buy_log[] = "mall_buy_log";

/**
 * @class mall_buy_log
 * 
 * @brief
 */
class mall_buy_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_NPC_CID, IDX_ITEM_CID, IDX_AMOUNT,
    IDX_BUY_TIME,
    IDX_END
  };
  mall_buy_log () { this->reset(); }

  void reset()
  {
    this->char_id_    = 0;
    this->npc_cid_    = 0;
    this->item_cid_   = 0;
    this->amount_     = 0;
    this->buy_time_   = 0;
  }

  static const char *all_col()
  { return "char_id,npc_cid,item_cid,amount,buy_time"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where char_id=%d and buy_time>%d",
                      mall_buy_log::all_col(),
                      db_tb_mall_buy_log,
                      this->char_id_, this->buy_time_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "%d,%d,%d,%d,%d"
                      ")",
                      db_tb_mall_buy_log,
                      mall_buy_log::all_col(),
                      this->char_id_,
                      this->npc_cid_, this->item_cid_, this->amount_, this->buy_time_);
  }

  int char_id_;
  int npc_cid_;
  int item_cid_;
  int amount_;
  int buy_time_;
};
#endif // MALL_BUY_LOG_H_ 
