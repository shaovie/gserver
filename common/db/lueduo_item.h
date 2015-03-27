// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-04 18:16
 */
//========================================================================

#ifndef LUEDUO_ITEM_H_
#define LUEDUO_ITEM_H_

// Lib header
#include "obj_pool.h"
#include "singleton.h"

// Forward declarations

const char db_tb_lueduo_item[] = "lueduo_item";

/**
 * @class lueduo_item
 * 
 * @brief
 */
class lueduo_item
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_BIND_AMOUNT, IDX_UNBIND_AMOUNT,

    IDX_END
  };
  lueduo_item() { this->reset(); }
  lueduo_item(const int char_id,
              const int cid,
              const int b_cnt,
              const int unb_cnt)
    : char_id_(char_id),
    cid_(cid),
    bind_amount_(b_cnt),
    unbind_amount_(unb_cnt)
  { }
  ~lueduo_item() { }
  
  void reset()
  {
    this->char_id_           = 0;
    this->cid_               = 0;
    this->bind_amount_       = 0;
    this->unbind_amount_     = 0;
  }
  void operator = (const lueduo_item &io)
  {
    if (&io == this) return ;

    this->char_id_           = io.char_id_;
    this->cid_               = io.cid_;
    this->bind_amount_       = io.bind_amount_;
    this->unbind_amount_     = io.unbind_amount_;
  }
  static const char *all_col()
  { return "char_id,cid,bind_amount,unbind_amount"; }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) "
                      "values(%d,%d,%d,%d)",
                      db_tb_lueduo_item, lueduo_item::all_col(),
                      this->char_id_, this->cid_, this->bind_amount_, this->unbind_amount_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set bind_amount=%d,unbind_amount=%d"
                      " where char_id=%d and cid=%d",
                      db_tb_lueduo_item, this->bind_amount_, this->unbind_amount_,
                      this->char_id_, this->cid_);

  }

public:
  int     char_id_;                   // 所属角色ID
  int     cid_;                       // 道具cid
  int     bind_amount_;               // 绑定道具数量
  int     unbind_amount_;             // 非绑道具数量
};
/**
 * @class lueduo_item_pool
 * 
 * @brief
 */
class lueduo_item_pool : public singleton<lueduo_item_pool>
{
  friend class singleton<lueduo_item_pool>;
public:
  lueduo_item *alloc() { return this->pool_.alloc(); }

  void release(lueduo_item *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  lueduo_item_pool() { }

  obj_pool<lueduo_item, obj_pool_std_allocator<lueduo_item> > pool_;
};

#endif // LUEDUO_ITEM_H_

