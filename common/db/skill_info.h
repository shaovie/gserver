// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-29 11:34
 */
//========================================================================

#ifndef SKILL_INFO_H_
#define SKILL_INFO_H_

#include "obj_pool.h"
#include "singleton.h"

// Lib header
#include <stdio.h>

// Forward declarations

const char db_tb_skill_info[] = "skill";

/**
 * @class skill_info
 * 
 * @brief
 */
class skill_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID, IDX_LVL, IDX_USE_TIME,
    IDX_END
  };
  skill_info() { this->reset(); }
  
  static const char *all_col() { return "char_id,cid,lvl,use_time"; }
  void reset()
  {
    this->char_id_        = 0;
    this->cid_            = 0;
    this->rate_           = 0;
    this->lvl_            = 1;
    this->use_time_       = 0;
    this->use_usec_       = 0;
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values(%d,%d,%d,%d)",
                      db_tb_skill_info,
                      skill_info::all_col(),
                      this->char_id_, this->cid_, this->lvl_, this->use_time_);
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      skill_info::all_col(),
                      db_tb_skill_info,
                      this->char_id_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set lvl=%d,use_time=%d where char_id=%d and cid=%d",
                      db_tb_skill_info,
                      this->lvl_, this->use_time_,
                      this->char_id_, this->cid_);
  }

  short   lvl_;
  short   rate_;
  int     char_id_;
  int     cid_;
  int     use_time_;
  int     use_usec_;      // do not store in db
};
/**
 * @class skill_info_pool
 * 
 * @brief
 */
class skill_info_pool : public singleton<skill_info_pool>
{
  friend class singleton<skill_info_pool>;
public:
  skill_info *alloc() { return this->pool_.alloc(); }

  void release(skill_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  skill_info_pool() { }
  obj_pool<skill_info, obj_pool_std_allocator<skill_info> > pool_;
};
#endif // SKILL_INFO_H_

