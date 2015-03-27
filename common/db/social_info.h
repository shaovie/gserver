// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2013-12-15 11:30
 */
//========================================================================

#ifndef SOCIAL_INFO_H_
#define SOCIAL_INFO_H_

#include "global_macros.h"

// Lib header
#include <string.h>
#include "obj_pool.h"
#include "singleton.h"

// Forward declarations

const char db_tb_social_info[] = "social";

/**
 * @class social_info
 * 
 * @brief
 */
class social_info
{
  friend class obj_pool_std_allocator<social_info>;
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_RELATION,
    IDX_SOCIALER_ID,
    IDX_REL_VALUE,
    IDX_END
  };
  social_info() { reset(); }
  ~social_info() { }

  static const char *all_col()
  { return "char_id,relation,socialer_id"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      social_info::all_col(),
                      db_tb_social_info,
                      this->char_id_);
  }
  int select_other_socialer_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select char_id from %s where socialer_id=%d",
                      db_tb_social_info,
                      this->socialer_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) values("
                      "%d,%d,%d"
                      ")",
                      db_tb_social_info,
                      social_info::all_col(),
                      this->char_id_,
                      this->relation_, this->socialer_id_);
  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s "
                      "where char_id=%d and relation=%d and socialer_id=%d",
                      db_tb_social_info,
                      this->char_id_, this->relation_, this->socialer_id_);
  }

  void reset()
  {
    this->char_id_          = 0;
    this->relation_         = 0;
    this->socialer_id_      = 0;
  }

  char relation_;                   // 关系（好友、黑名单、仇人）
  int char_id_;                     // 角色id
  int socialer_id_;                 // 关系人id
};
/**
 * @class social_info_pool
 * 
 * @brief
 */
class social_info_pool : public singleton<social_info_pool>
{
  friend class singleton<social_info_pool>;
public:
  social_info *alloc() { return this->pool_.alloc(); }

  void release(social_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  social_info_pool() : pool_() { }

  obj_pool<social_info, obj_pool_std_allocator<social_info> > pool_;
};

#endif  // SOCIAL_INFO_H_
