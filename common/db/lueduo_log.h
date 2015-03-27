// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-05 17:16
 */
//========================================================================

#ifndef LUEDUO_LOG_H_
#define LUEDUO_LOG_H_

// Lib header
#include "obj_pool.h"
#include "singleton.h"

const char db_tb_lueduo_log[] = "lueduo_log";

/**
 * @class lueduo_log
 * 
 * @brief
 */
class lueduo_log
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_ROBBER, IDX_TIME, IDX_MATERIAL, IDX_AMOUNT, IDX_RESULT,
    IDX_IF_REVENGE,

    IDX_END
  };
  lueduo_log() { this->reset(); }
  ~lueduo_log() { }
  
  void reset()
  {
    this->char_id_        = 0;
    this->robber_         = 0;
    this->time_           = 0;
    this->material_       = 0;
    this->amount_         = 0;
    this->result_         = 0;
    this->if_revenge_     = 0;
  }
  void operator = (const lueduo_log &io)
  {
    if (&io == this) return ;

    this->char_id_        = io.char_id_;
    this->robber_         = io.robber_;
    this->time_           = io.time_;
    this->material_       = io.material_;
    this->amount_         = io.amount_;
    this->result_         = io.result_;
    this->if_revenge_     = io.if_revenge_;
  }
  static const char *all_col()
  { return "char_id,robber,time,material,amount,result,if_revenge"; }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d order by time desc limit 10",
                      lueduo_log::all_col(), db_tb_lueduo_log,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s) "
                      "values(%d,%d,%d,%d,%d,%d,%d)",
                      db_tb_lueduo_log, lueduo_log::all_col(),
                      this->char_id_, this->robber_, this->time_,
                      this->material_, this->amount_,
                      this->result_, this->if_revenge_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set if_revenge=%d"
                      " where char_id=%d and robber=%d and time=%d",
                      db_tb_lueduo_log, this->if_revenge_,
                      this->char_id_, this->robber_, this->time_);

  }
  int delete_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "delete from %s"
                      " where char_id=%d and robber=%d and time=%d",
                      db_tb_lueduo_log,
                      this->char_id_, this->robber_, this->time_);

  }

public:
  char    result_;              // 是否成功
  char    if_revenge_;          // 是否报仇
  int     char_id_;             // 角色ID
  int     robber_;              // 强盗ID
  int     time_;                // 抢夺时间
  int     material_;            // 材料ID
  int     amount_;              // 被抢个数
};
/**
 * @class lueduo_log_pool
 * 
 * @brief
 */
class lueduo_log_pool : public singleton<lueduo_log_pool>
{
  friend class singleton<lueduo_log_pool>;
public:
  lueduo_log *alloc() { return this->pool_.alloc(); }

  void release(lueduo_log *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  lueduo_log_pool() { }

  obj_pool<lueduo_log, obj_pool_std_allocator<lueduo_log> > pool_;
};

#endif // LUEDUO_LOG_H_
