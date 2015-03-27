// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-03-28 19:09
 */
//========================================================================

#ifndef MARKET_INFO_H_
#define MARKET_INFO_H_

#include <string.h>

#include "obj_pool.h"
#include "item_obj.h"
#include "singleton.h"

// Forward declarations

const char db_tb_market_info[] = "market";
/**
 * @class market_info
 * 
 * @brief
 */
class market_info
{
  friend class obj_pool_std_allocator<market_info>;
public:
  enum
  {
    IDX_MARKET_ID = 0,
    IDX_CHAR_ID,
    IDX_PRICE, IDX_PRICE_TYPE, IDX_BEGIN_TIME,
    IDX_CID, IDX_AMOUNT, IDX_ATTR_FLOAT_COE, IDX_EXTRA_INFO,
    IDX_END 
  };
  market_info() { this->item_ = NULL; this->reset(); }

  ~market_info() 
  {
    if (this->item_ != NULL)
      item_obj_pool::instance()->release(this->item_);
  }

  static const char *all_col()
  { return "market_id,char_id,price,price_type,begin_time,\
    cid,amount,attr_float_coe,extra_info"; }

  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) values("
                      "%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,"
                      "%d,"
                      "'%s'"
                      ")",
                      db_tb_market_info,
                      market_info::all_col(),
                      this->market_id_, this->char_id_,
                      this->price_, this->price_type_, this->begin_time_,
                      this->item_->cid_, this->item_->amount_,
                      this->item_->attr_float_coe_,
                      this->item_->extra_info_ == NULL ? "" : this->item_->extra_info_->rd_ptr());
  }

  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s "
                      "where market_id=%d",
                      db_tb_market_info,
                      this->market_id_);
  }

  void reset()
  {
    this->price_type_  = 0; 
    this->market_id_   = 0;
    this->char_id_     = 0;
    this->price_       = 0;
    this->begin_time_  = 0;
    if (this->item_ != NULL)
      item_obj_pool::instance()->release(this->item_);
    this->item_ = NULL; 
  }

  char price_type_;                  // 价格类型 1:元宝 2:铜钱  
  int market_id_;                    // 市场订单id
  int char_id_;                      // 卖家id
  int price_;                        // 价格
  int begin_time_;                   // 上架时间
  item_obj *item_;                   // 物品
};
/**
 * @class market_info_pool
 * 
 * @brief
 */
class market_info_pool : public singleton<market_info_pool>
{
  friend class singleton<market_info_pool>;
public:
  market_info *alloc() { return this->pool_.alloc(); }

  void release(market_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  market_info_pool() : pool_() { }
  obj_pool<market_info, obj_pool_std_allocator<market_info> > pool_;
};

inline out_stream & operator << (out_stream &os, market_info *mi)
{
  stream_ostr mi_so((const char *)mi, sizeof(market_info));
  os << mi_so << mi->item_;
  return os;
}
inline in_stream & operator >> (in_stream &is, market_info *mi)
{
  char mt_bf[sizeof(market_info) + 4] = {0};
  stream_istr mt_si(mt_bf, sizeof(mt_bf));

  is >> mt_si;
  market_info *p = (market_info *)mt_bf;
  p->item_ = NULL;
  ::memcpy((char *)mi, mt_si.str(), sizeof(market_info));
  mi->item_ = item_obj_pool::instance()->alloc();
  is >> mi->item_;
  return is;
}
#endif // MARKET_INFO_H_

