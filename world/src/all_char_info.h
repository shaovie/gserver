// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-05 11:46
 */
//========================================================================

#ifndef ALL_CHAR_INFO_H_
#define ALL_CHAR_INFO_H_

// Lib header
#include <string.h>
#include "singleton.h"

#include "def.h"
#include "ilist.h"
#include "global_macros.h"

// Forward declarations
class player_obj;
class account_info;
class recharge_log_info;
class consume_log_info;
class char_brief_info;
class all_char_info_impl;

/**
 * @class
 * 
 * @brief brief of char
 */
class account_brief_info 
{
public:
  account_brief_info();

  bool is_char_ok(const int char_id);
public:
  int acc_recharge_;
  account_info *ac_info_;
  ilist<char_brief_info *> char_list_;
};
/**
 * @class char_brief_info
 * 
 * @brief brief of char
 */
class char_brief_info
{
public:
  char_brief_info() :
    career_(0),
    vip_(0),
    fa_bao_dj_(0),
    lvl_(1),
    sin_val_(0),
    title_cid_(0),
    all_qh_lvl_(0),
    sum_bao_shi_lvl_(0),
    char_id_(0),
    out_time_(0),
    zhu_wu_cid_(0),
    fu_wu_cid_(0),
    zhan_li_(0),
    scene_cid_(0),
    total_mstar_(0)
  {
    ::memset(name_, '\0', sizeof(name_));
    ::memset(fa_bao_, '\0', sizeof(fa_bao_));
    ::memset(attr_, 0, sizeof(attr_));
  }

  static void on_char_lvl_up(const int char_id, const short lvl);
  static void on_char_logout(player_obj *);
  static void on_char_zhan_li_update(const int char_id, const int val);
  static void on_char_sin_val_update(const int char_id, const int val);
  static void on_char_equip_update(const int char_id, const int part, const int equip_cid);
  static void on_char_title_update(const int char_id, const short title);
  static void on_char_vip_up(const int char_id, const char val);
  static void on_char_fa_bao_update(const int char_id, const char dj, const char *fa_bao);

  static void on_recharge_ok(const int char_id, const int t, const int value);
  static void on_consume_diamond(const int char_id, const int, const int t, const int value);

  static int acc_recharge(const int char_id, const int begin_time, const int end_time);
  static int daily_recharge(const int char_id, const int begin_time, const int end_time);
  static int acc_consume(const int char_id, const int begin_time, const int end_time);
  static int daily_consume(const int char_id, const int begin_time, const int end_time);
public:
  char  career_;
  char  vip_;
  char  fa_bao_dj_;                      // 法宝等阶
  short lvl_;
  short sin_val_;
  short title_cid_;
  short all_qh_lvl_;
  short sum_bao_shi_lvl_;
  int   char_id_;
  int   out_time_;
  int   zhu_wu_cid_;
  int   fu_wu_cid_;
  int   zhan_li_;
  int   scene_cid_;
  int   total_mstar_;
  char  name_[MAX_NAME_LEN + 1];
  char  fa_bao_[FA_BAO_PART_CNT * 2];     // 法宝穿戴信息
  int   attr_[ATTR_T_ITEM_CNT]; // 属性值
  ilist<int> socialer_list_;    // friends + black (我被谁建立关系)
  ilist<recharge_log_info *> recharge_log_;
  ilist<consume_log_info *> consume_log_;
  ilist<char_brief_info *> char_list_;
  ilist<pair_t<int> > tianfu_skill_list_; // first:cid second:lvl 只下线时更新
};
/**
 * @class all_char_info
 * 
 * @brief
 */
class all_char_info : public singleton<all_char_info>
{
  friend class singleton<all_char_info>;
public:
  void insert_account_info(const char *, account_brief_info *);
  account_brief_info *get_account_brief_info(const char *account);

  //=
  void insert_char_info(char_brief_info *);

  char_brief_info *get_char_brief_info(const char *name);
  char_brief_info *get_char_brief_info(const int char_id);

  ilist<int> *get_socialer_list(const int char_id);
  void on_i_add_socialer(const int my_id, const int socialer_id);
  void on_i_del_socialer(const int my_id, const int socialer_id);
private:
  all_char_info();
  all_char_info(const all_char_info &);
  all_char_info &operator = (const all_char_info &);

  all_char_info_impl *impl_;
};

#endif // ALL_CHAR_INFO_H_

