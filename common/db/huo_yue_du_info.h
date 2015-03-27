// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-05-04 16:58
 */
//========================================================================

#ifndef HUO_YUE_DU_INFO_H_
#define HUO_YUE_DU_INFO_H_

#include "mblock_pool.h"
#include "global_macros.h"

// Lib header
#include "obj_pool.h"
#include "singleton.h"

// Forward declarations

const char db_tb_huo_yue_du_info[] = "huo_yue_du";

/**
 * @class huo_yue_du_info
 *
 * @brief
 */
class huo_yue_du_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CID,
    IDX_SCORE,
    IDX_GET_AWARD,
    IDX_END
  };

  huo_yue_du_info() { this->reset(); }
  ~huo_yue_du_info() { }

  static const char *all_col()
  { return "char_id,cid,score,get_award"; }

  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      huo_yue_du_info::all_col(),
                      db_tb_huo_yue_du_info,
                      this->char_id_);
  }

  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "replace into %s(%s) values("
                      "%d,%d,%d,%d"
                      ")",
                      db_tb_huo_yue_du_info,
                      huo_yue_du_info::all_col(),
                      this->char_id_, this->cid_, this->score_, this->get_award_);
  }

  int clear_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set score=0,get_award=0 where char_id=%d",
                      db_tb_huo_yue_du_info,
                      this->char_id_);
  }

  void reset()
  {
    this->char_id_    = 0;
    this->cid_        = 0;
    this->score_      = 0;
    this->get_award_  = 0;
  }

  char  get_award_;             // 是否领取了奖励
  short cid_;                   // 活跃度cid
  short score_;                 // 当前活跃度积分
  int   char_id_;               // 角色id
};

/**
 * @class huo_yue_du_info_pool
 *
 * @brief
 */
class huo_yue_du_info_pool : public singleton<huo_yue_du_info_pool>
{
  friend class singleton<huo_yue_du_info_pool>;
public:
  huo_yue_du_info *alloc() { return this->pool_.alloc(); }

  void release(huo_yue_du_info *p)
  {
    p->reset();
    this->pool_.release(p);
  }
private:
  huo_yue_du_info_pool() { }
  obj_pool<huo_yue_du_info, obj_pool_std_allocator<huo_yue_du_info> > pool_;
};

#endif  // HUO_YUE_DU_INFO_H_
