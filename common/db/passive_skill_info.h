// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-29 11:34
 */
//========================================================================

#ifndef PASSIVE_SKILL_INFO_H_
#define PASSIVE_SKILL_INFO_H_

#include "obj_pool.h"
#include "singleton.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_passive_skill_info[] = "passive_skill";

/**
 * @class passive_skill_info
 * 
 * @brief
 */
class passive_skill_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_LVL, IDX_UP_CNT, IDX_BLESS,
    IDX_END
  };
  passive_skill_info() { this->reset(); }
  
  static const char *all_col() { return "char_id,cid,lvl,up_cnt,bless"; }
  void reset()
  {
    this->char_id_        = 0;
    this->cid_            = 0;
    this->lvl_            = 1;
    this->up_cnt_         = 0;
    this->bless_          = 0;
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "replace into %s(%s)values(%d,%d,%d,%d,%d)",
                      db_tb_passive_skill_info,
                      passive_skill_info::all_col(),
                      this->char_id_, this->cid_, this->lvl_, this->up_cnt_, this->bless_);
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      passive_skill_info::all_col(),
                      db_tb_passive_skill_info,
                      this->char_id_);
  }

  short   lvl_;
  int     char_id_;
  int     cid_;
  int     up_cnt_;
  int     bless_;      // do not store in db
};
/**
 * @class passive_skill_info_pool
 * 
 * @brief
 */
class passive_skill_info_pool : public singleton<passive_skill_info_pool>
{
  friend class singleton<passive_skill_info_pool>;
public:
  passive_skill_info *alloc() { return this->pool_.alloc(); }

  void release(passive_skill_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  passive_skill_info_pool() { }
  obj_pool<passive_skill_info, obj_pool_std_allocator<passive_skill_info> > pool_;
};
#endif // PASSIVE_SKILL_INFO_H_

