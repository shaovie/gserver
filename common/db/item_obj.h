// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-10-10 10:16
 */
//========================================================================

#ifndef ITEM_OBJ_H_
#define ITEM_OBJ_H_

#include "istream.h"
#include "mblock_pool.h"
#include "global_macros.h"

// Lib header
#include <cassert>
#include "obj_pool.h"
#include "singleton.h"

// Forward declarations

const char db_tb_item[] = "item";

/**
 * @class item_obj
 * 
 * @brief
 */
class item_obj
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_ID, IDX_PKG, IDX_CID, IDX_BIND,
    IDX_AMOUNT, IDX_ATTR_FLOAT_COE,
    IDX_EXTRA_INFO,

    IDX_END
  };
  item_obj() { this->extra_info_ = NULL; this->reset(); }
  ~item_obj()
  {
    if (this->extra_info_ != NULL)
      mblock_pool::instance()->release(this->extra_info_);
  }
  
  void reset()
  {
    this->char_id_           = 0;
    this->id_                = 0;
    this->pkg_               = 0;
    this->cid_               = 0;
    this->bind_              = 0;
    this->amount_            = 0;
    this->attr_float_coe_    = 0;
    if (this->extra_info_ != NULL)
      mblock_pool::instance()->release(this->extra_info_);
    this->extra_info_        = NULL;
  }
  void operator = (const item_obj &io)
  {
    if (&io == this) return ;

    this->char_id_           = io.char_id_;
    this->id_                = io.id_;
    this->pkg_               = io.pkg_;
    this->cid_               = io.cid_;
    this->bind_              = io.bind_;
    this->amount_            = io.amount_;
    this->attr_float_coe_    = io.attr_float_coe_;
    if (io.extra_info_ != NULL)
    {
      if (this->extra_info_ == NULL
          || this->extra_info_->length() < io.extra_info_->length())
      {
        if (this->extra_info_ != NULL)
          mblock_pool::instance()->release(this->extra_info_);
        this->extra_info_ = mblock_pool::instance()->alloc(io.extra_info_->size());
      }
      this->extra_info_->reset();
      this->extra_info_->copy(io.extra_info_->rd_ptr(), io.extra_info_->length());
      this->extra_info_->set_eof();
    }else if (this->extra_info_ != NULL)
    {
      mblock_pool::instance()->release(this->extra_info_);
      this->extra_info_ = NULL;
    }
  }
  static const char *all_col()
  { return "char_id,id,pkg,cid,bind,amount,attr_float_coe,extra_info"; }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) "
                      "values(%d,%d,%d,%d,%d,%d,%d,'%s')",
                      db_tb_item,
                      item_obj::all_col(),
                      this->char_id_, this->id_,
                      this->pkg_, this->cid_,
                      this->bind_, this->amount_,
                      this->attr_float_coe_,
                      this->extra_info_ == NULL ? "" : this->extra_info_->rd_ptr());
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      item_obj::all_col(),
                      db_tb_item,
                      this->char_id_);
  }
  int select_single_pkg_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d and pkg=%d",
                      item_obj::all_col(),
                      db_tb_item,
                      this->char_id_,
                      this->pkg_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "pkg=%d,cid=%d,"
                      "bind=%d,amount=%d,attr_float_coe=%d,"
                      "extra_info='%s'"
                      " where char_id=%d and id=%d",
                      db_tb_item,
                      this->pkg_, this->cid_,
                      this->bind_, this->amount_,
                      this->attr_float_coe_,
                      this->extra_info_ == NULL ? "" : this->extra_info_->rd_ptr(),
                      this->char_id_, this->id_);

  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s where char_id=%d and id=%d",
                      db_tb_item,
                      this->char_id_,
                      this->id_);
  }

public:
  char    bind_;                      // 绑定属性
  char    pkg_;                       // 容器类型
  short   attr_float_coe_;            // 装备属性浮动系数
  int     amount_;                    // 道具叠加数量
  int     char_id_;                   // 所属角色ID
  int     id_;                        // 道具ID
  int     cid_;                       // 道具cid
  mblock *extra_info_;
};
inline out_stream & operator << (out_stream &os, item_obj *io)
{
  stream_ostr item_so((const char *)io, sizeof(item_obj));
  os << item_so;
  if (io->extra_info_ != NULL)
    os << stream_ostr(io->extra_info_->rd_ptr(), io->extra_info_->length());
  else
    os << stream_ostr("", 0);
  return os;
}
inline in_stream & operator >> (in_stream &is, item_obj *io)
{
  char extra_info_bf[MAX_ITEM_EXTRA_INFO_LEN + 1] = {0};
  stream_istr extra_info_si(extra_info_bf, sizeof(extra_info_bf));

  char item_bf[sizeof(item_obj) + 4] = {0};
  stream_istr item_si(item_bf, sizeof(item_bf));

  is >> item_si >> extra_info_si;

  item_obj *p = (item_obj *)item_bf;
  p->extra_info_ = NULL;
  *io = *p;

  if (extra_info_si.str_len() > 0)
  {
    io->extra_info_ = mblock_pool::instance()->alloc(extra_info_si.str_len() + 1);
    io->extra_info_->copy(extra_info_si.str(), extra_info_si.str_len());
    io->extra_info_->set_eof();
  }
  return is;
}
/**
 * @class item_obj_pool
 * 
 * @brief
 */
class item_obj_pool : public singleton<item_obj_pool>
{
  friend class singleton<item_obj_pool>;
public:
  item_obj *alloc() { return this->pool_.alloc(); }

  void release(item_obj *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  item_obj_pool() { }

  obj_pool<item_obj, obj_pool_std_allocator<item_obj> > pool_;
};

#endif // ITEM_OBJ_H_

