// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-07 14:34
 */
//========================================================================

#ifndef TIANFU_SKILL_INFO_H_
#define TIANFU_SKILL_INFO_H_

#include "obj_pool.h"
#include "singleton.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_tianfu_skill_info[] = "tianfu_skill";

/**
 * @class tianfu_skill_info
 * 
 * @brief
 */
class tianfu_skill_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_LVL, IDX_EXP,
    IDX_END
  };
  tianfu_skill_info() { this->reset(); }
  
  static const char *all_col() { return "char_id,cid,lvl,exp"; }
  void reset()
  {
    this->char_id_        = 0;
    this->cid_            = 0;
    this->lvl_            = 1;
    this->exp_            = 0;
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values(%d,%d,%d,%d)",
                      db_tb_tianfu_skill_info,
                      tianfu_skill_info::all_col(),
                      this->char_id_, this->cid_, this->lvl_, this->exp_);
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      tianfu_skill_info::all_col(),
                      db_tb_tianfu_skill_info,
                      this->char_id_);
  }

  short   lvl_;
  int     char_id_;
  int     cid_;
  int     exp_;
};
/**
 * @class tianfu_skill_info_pool
 * 
 * @brief
 */
class tianfu_skill_info_pool : public singleton<tianfu_skill_info_pool>
{
  friend class singleton<tianfu_skill_info_pool>;
public:
  tianfu_skill_info *alloc() { return this->pool_.alloc(); }

  void release(tianfu_skill_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  tianfu_skill_info_pool() { }
  obj_pool<tianfu_skill_info, obj_pool_std_allocator<tianfu_skill_info> > pool_;
};
#endif // TIANFU_SKILL_INFO_H_

