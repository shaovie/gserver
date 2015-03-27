// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-18 18:20
 */
//========================================================================

#ifndef CHENG_JIU_INFO_H_
#define CHENG_JIU_INFO_H_

#include "global_macros.h"

// Lib header
#include <stdio.h>
#include "obj_pool.h"
#include "singleton.h"

// Forward declarations

const char db_tb_cheng_jiu_info[] = "cheng_jiu";

/**
 * @class cheng_jiu_info
 *
 * @brief
 */
class cheng_jiu_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CJ_TYPE,
    IDX_ID, IDX_VALUE,
    IDX_END
  };

  cheng_jiu_info() { this->reset(); }

  static const char *all_col()
  { return "char_id,cj_type,id,value"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      cheng_jiu_info::all_col(),
                      db_tb_cheng_jiu_info,
                      this->char_id_);
  }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s) values("
                      "%d,%d,%d,%d"
                      ")",
                      db_tb_cheng_jiu_info,
                      cheng_jiu_info::all_col(),
                      this->char_id_, this->type_,
                      this->id_, this->value_);
  }

  void reset()
  {
    this->char_id_ = 0;
    this->type_    = 0;
    this->id_      = 0;
    this->value_   = 0;
  }

  int char_id_;           // 角色id
  int type_;              // 成就类型
  int id_;                // 成就id
  int value_;             // 成就值
};

/**
 * @class cheng_jiu_info_pool
 *
 * @brief
 */
class cheng_jiu_info_pool : public singleton<cheng_jiu_info_pool>
{
  friend class singleton<cheng_jiu_info_pool>;
public:
  cheng_jiu_info *alloc() { return this->pool_.alloc(); }

  void release(cheng_jiu_info *p)
  {
    p->reset();
    this->pool_.release(p);
  }
private:
  cheng_jiu_info_pool() { }
  obj_pool<cheng_jiu_info, obj_pool_std_allocator<cheng_jiu_info> > pool_;
};

#endif  // CHENG_JIU_INFO_H_
