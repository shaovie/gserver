// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-13 13:38
 */
//========================================================================

#ifndef CHAR_INFO_H_
#define CHAR_INFO_H_

// Lib header
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "obj_pool.h"
#include "singleton.h"

#include "def.h"
#include "global_macros.h"

// Forward declarations

const char db_tb_char_info[] = "char_info";

/**
 * @class char_info
 *
 * @brief
 */
class char_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_ACCOUNT, IDX_NAME,
    IDX_CAREER,
    IDX_EXP, IDX_LVL, IDX_COIN, IDX_B_DIAMOND,
    IDX_HP, IDX_MP, IDX_ZHAN_LI,
    IDX_DIR, IDX_COORD_X, IDX_COORD_Y, IDX_SCENE_ID, IDX_SCENE_CID,
    IDX_LAST_COORD_X, IDX_LAST_COORD_Y, IDX_LAST_SCENE_CID,
    IDX_PK_MODE, IDX_SIN_VAL, IDX_SIN_OL_TIME, IDX_PKG_CAP, IDX_STORAGE_CAP,
    IDX_CUR_TITLE,
    IDX_IP, IDX_OL_TIME,
    IDX_C_TIME, IDX_IN_TIME, IDX_OUT_TIME,
    IDX_END
  };
  char_info() { this->reset(); }

  void reset()
  {
    this->char_id_          = 0;
    this->career_           = 0;
    this->dir_              = DIR_RIGHT_DOWN;
    this->exp_              = 0;
    this->lvl_              = 1;
    this->coin_             = 0;
    this->b_diamond_        = 0;
    this->hp_               = 1;
    this->mp_               = 0;
    this->zhan_li_          = 0;
    this->coord_x_          = 0;
    this->coord_y_          = 0;
    this->last_coord_x_     = 0;
    this->last_coord_y_     = 0;
    this->scene_id_         = 0;
    this->scene_cid_        = 0;
    this->last_scene_cid_   = 0;
    this->pk_mode_          = PK_MODE_FIGHT;
    this->sin_val_          = 0;
    this->sin_ol_time_      = 0;
    this->pkg_cap_          = 0;
    this->storage_cap_      = 0;
    this->cur_title_        = 0;
    this->ol_time_          = 0;
    this->c_time_           = 0;
    this->in_time_          = 0;
    this->out_time_         = 0;
    ::memset(this->ip_, 0, sizeof(this->ip_));
    ::memset(this->account_, 0, sizeof(this->account_));
    ::memset(this->name_, 0, sizeof(this->name_));
  }
  static const char *all_col()
  {
    return "char_id,account,name,career,exp,lvl,coin,b_diamond,hp,mp,zhan_li,"
      "dir,coord_x,coord_y,scene_id,scene_cid,"
      "last_coord_x,last_coord_y,last_scene_cid,"
      "pk_mode,sin_val,sin_ol_time,pkg_cap,storage_cap,cur_title,"
      "ip,ol_time,c_time,in_time,out_time";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select %s from %s where char_id=%d",
                      char_info::all_col(),
                      db_tb_char_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "insert into %s(%s)values("
                      "%d,"
                      "'%s','%s',"
                      "%d,"
                      "%ld,%d,%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,%d,%d,%d,"
                      "%d,"
                      "'%s',%d,"
                      "%d,%d,%d"
                      ")",
                      db_tb_char_info,
                      char_info::all_col(),
                      this->char_id_,
                      this->account_, this->name_,
                      this->career_,
                      this->exp_, this->lvl_, this->coin_, this->b_diamond_,
                      this->hp_, this->mp_, this->zhan_li_,
                      this->dir_, this->coord_x_, this->coord_y_,
                      this->scene_id_, this->scene_cid_,
                      this->last_coord_x_, this->last_coord_y_, this->last_scene_cid_,
                      this->pk_mode_, this->sin_val_, this->sin_ol_time_, this->pkg_cap_, this->storage_cap_,
                      this->cur_title_,
                      this->ip_, this->ol_time_,
                      this->c_time_, this->in_time_, this->out_time_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "update %s set "
                      "exp=%ld,lvl=%d,coin=%d,b_diamond=%d,"
                      "hp=%d,mp=%d,zhan_li=%d,"
                      "dir=%d,coord_x=%d,coord_y=%d,"
                      "scene_id=%d,scene_cid=%d,"
                      "last_coord_x=%d,last_coord_y=%d,last_scene_cid=%d,"
                      "pk_mode=%d,sin_val=%d,sin_ol_time=%d,pkg_cap=%d,storage_cap=%d,"
                      "cur_title=%d,"
                      "ip='%s',ol_time=%d,"
                      "in_time=%d,out_time=%d"
                      " where char_id=%d",
                      db_tb_char_info,
                      this->exp_, this->lvl_, this->coin_, this->b_diamond_,
                      this->hp_, this->mp_, this->zhan_li_,
                      this->dir_, this->coord_x_, this->coord_y_,
                      this->scene_id_, this->scene_cid_,
                      this->last_coord_x_, this->last_coord_y_, this->last_scene_cid_,
                      this->pk_mode_, this->sin_val_, this->sin_ol_time_, this->pkg_cap_, this->storage_cap_,
                      this->cur_title_,
                      this->ip_, this->ol_time_,
                      this->in_time_, this->out_time_,
                      this->char_id_);
  }
  int select_search_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len, 
                      "select char_id from %s where name like '%%%s%%' limit 16",
                      db_tb_char_info,
                      this->name_);
  }
public:
  char    career_;                         // 职业 5种
  char    dir_;                            // 朝向
  char    pk_mode_;                        // PK 模式

  short   lvl_;                            // 级别
  short   coord_x_;                        // 坐标x
  short   coord_y_;                        // 坐标y
  short   last_coord_x_;                   // 坐标x
  short   last_coord_y_;                   // 坐标y

  short   pkg_cap_;                        // 背包容量
  short   storage_cap_;                    // 仓库容量
  short   sin_val_;                        // 罪恶值
  short   cur_title_;                      // 当前显示称号

  int     char_id_;                        // 玩家ID
  int     hp_;
  int     mp_;
  int     zhan_li_;                        // 战力
  int     coin_;                           // 金币
  int     b_diamond_;
  int     scene_id_;
  int     scene_cid_;
  int     last_scene_cid_;
  int     sin_ol_time_;                    // 罪恶值累积在线时间
  int     ol_time_;                        // 累积在线时间
  int     c_time_;                         // 角色创建时间
  int     in_time_;                        // 角色登录时间
  int     out_time_;                       // 角色退出时间

  int64_t exp_;                            // 经验值
  char    account_[MAX_ACCOUNT_LEN + 1];   // 帐户名
  char    name_[MAX_NAME_LEN + 1];         // 角色名
  char ip_[MAX_IP_ADDR_LEN + 1];
};

/**
 * @class char_info_pool
 *
 * @brief
 */
class char_info_pool : public singleton<char_info_pool>
{
  friend class singleton<char_info_pool>;
public:
  char_info *alloc() { return this->pool_.alloc(); }

  void release(char_info *p)
  {
    p->reset();
    this->pool_.release(p);
  }
private:
  char_info_pool() { }
  obj_pool<char_info, obj_pool_std_allocator<char_info> > pool_;
};

#endif // CHAR_INFO_H_

