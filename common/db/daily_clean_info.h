// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-22 21:46
 */
//========================================================================

#ifndef DAILY_CLEAN_INFO_H_
#define DAILY_CLEAN_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "obj_pool.h"
#include "global_macros.h"

// Forward declarations

const char db_tb_daily_clean_info[] = "daily_clean_info";

/**
 * @class daily_clean_info
 *
 * @brief
 */
class daily_clean_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_CLEAN_TIME,
    IDX_GUILD_FREE_JB_CNT,
    IDX_GUILD_COST_JB_CNT,
    IDX_GUILD_PROMOTE_SKILL_CNT,
    IDX_OL_AWARD_GET_IDX,
    IDX_LOGIN_AWARD_IF_GET,
    IDX_DAILY_TASK_DONE_CNT, IDX_GUILD_TASK_DONE_CNT,
    IDX_OL_AWARD_START_TIME,
    IDX_CON_LOGIN_DAYS, IDX_JING_JI_CNT, IDX_FREE_REFRESH_JJ_CNT,
    IDX_IF_MC_REBATE,
    IDX_WORSHIP_CNT,
    IDX_GOODS_LUCKY_TURN_CNT, IDX_DAILY_WX_SHARED, IDX_IF_GOT_GOODS_LUCKY_DRAW,
    IDX_FREE_RELIVE_CNT,
    IDX_DXC_ENTER_CNT,
    IDX_END
  };

  daily_clean_info() { this->reset(); }

  void reset()
  {
    this->char_id_                 = 0;
    this->clean_time_              = 0;
    this->guild_free_jb_cnt_       = 0;
    this->guild_cost_jb_cnt_       = 0;
    this->guild_promote_skill_cnt_ = 0;
    this->ol_award_get_idx_        = 0;
    this->login_award_if_get_      = 0;
    this->daily_task_done_cnt_     = 0;
    this->guild_task_done_cnt_     = 0;
    this->ol_award_start_time_     = 0;
    this->con_login_days_          = 1;
    this->jing_ji_cnt_             = 0;
    this->free_refresh_jj_cnt_     = 0;
    this->if_mc_rebate_            = 0;
    this->worship_cnt_             = 0;
    this->goods_lucky_turn_cnt_    = 0;
    this->daily_wx_shared_         = 0;
    this->if_got_goods_lucky_draw_ = 0;
    this->free_relive_cnt_         = 0;
    this->dxc_enter_cnt_           = 0;
  }
  void operator = (const daily_clean_info &cei)
  {
    if (&cei == this) return ;

    this->char_id_                     = cei.char_id_;
    this->clean_time_                  = cei.clean_time_;
    this->guild_free_jb_cnt_           = cei.guild_free_jb_cnt_;
    this->guild_cost_jb_cnt_           = cei.guild_cost_jb_cnt_;
    this->guild_promote_skill_cnt_     = cei.guild_promote_skill_cnt_;
    this->ol_award_get_idx_            = cei.ol_award_get_idx_;
    this->login_award_if_get_          = cei.login_award_if_get_;
    this->daily_task_done_cnt_         = cei.daily_task_done_cnt_;
    this->guild_task_done_cnt_         = cei.guild_task_done_cnt_;
    this->ol_award_start_time_         = cei.ol_award_start_time_;
    this->con_login_days_              = cei.con_login_days_;
    this->jing_ji_cnt_                 = cei.jing_ji_cnt_;
    this->free_refresh_jj_cnt_         = cei.free_refresh_jj_cnt_;
    this->if_mc_rebate_                = cei.if_mc_rebate_;
    this->worship_cnt_                 = cei.worship_cnt_;
    this->goods_lucky_turn_cnt_        = cei.goods_lucky_turn_cnt_;
    this->daily_wx_shared_             = cei.daily_wx_shared_;
    this->if_got_goods_lucky_draw_     = cei.if_got_goods_lucky_draw_;
    this->free_relive_cnt_             = cei.free_relive_cnt_;
    this->dxc_enter_cnt_               = cei.dxc_enter_cnt_;
  }
  static const char *all_col()
  {
    return "char_id,clean_time,"
      "guild_free_jb_cnt,guild_cost_jb_cnt,guild_promote_skill_cnt,"
      "ol_award_get_idx,login_award_if_get,daily_task_done_cnt,guild_task_done_cnt,ol_award_start_time,"
      "con_login_days,jing_ji_cnt,free_refresh_jj_cnt,if_mc_rebate,worship_cnt,goods_lucky_turn_cnt,"
      "daily_wx_shared,if_got_goods_lucky_draw,free_relive_cnt,dxc_enter_cnt";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      daily_clean_info::all_col(),
                      db_tb_daily_clean_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,%d,"
                      "%d,"
                      "%d,%d,"
                      "%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,%d,"
                      "%d,%d,"
                      "%d,%d,%d,"
                      "%d,"
                      "%d"
                      ")",
                      db_tb_daily_clean_info,
                      daily_clean_info::all_col(),
                      this->char_id_, this->clean_time_,
                      this->guild_free_jb_cnt_,
                      this->guild_cost_jb_cnt_, this->guild_promote_skill_cnt_,
                      this->ol_award_get_idx_, this->login_award_if_get_,
                      this->daily_task_done_cnt_, this->guild_task_done_cnt_, this->ol_award_start_time_,
                      this->con_login_days_, this->jing_ji_cnt_, this->free_refresh_jj_cnt_,
                      this->if_mc_rebate_, this->worship_cnt_,
                      this->goods_lucky_turn_cnt_, this->daily_wx_shared_, this->if_got_goods_lucky_draw_,
                      this->free_relive_cnt_,
                      this->dxc_enter_cnt_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "clean_time=%d,"
                      "guild_free_jb_cnt=%d,"
                      "guild_cost_jb_cnt=%d,guild_promote_skill_cnt=%d,"
                      "ol_award_get_idx=%d,login_award_if_get=%d,"
                      "daily_task_done_cnt=%d,guild_task_done_cnt=%d,ol_award_start_time=%d,"
                      "con_login_days=%d,jing_ji_cnt=%d,free_refresh_jj_cnt=%d,"
                      "if_mc_rebate=%d,worship_cnt=%d,"
                      "goods_lucky_turn_cnt=%d,daily_wx_shared=%d,if_got_goods_lucky_draw=%d,"
                      "free_relive_cnt=%d,"
                      "dxc_enter_cnt=%d"
                      " where char_id=%d",
                      db_tb_daily_clean_info,
                      this->clean_time_,
                      this->guild_free_jb_cnt_,
                      this->guild_cost_jb_cnt_, this->guild_promote_skill_cnt_,
                      this->ol_award_get_idx_, this->login_award_if_get_,
                      this->daily_task_done_cnt_, this->guild_task_done_cnt_, this->ol_award_start_time_,
                      this->con_login_days_, this->jing_ji_cnt_, this->free_refresh_jj_cnt_,
                      this->if_mc_rebate_, this->worship_cnt_,
                      this->goods_lucky_turn_cnt_, this->daily_wx_shared_, this->if_got_goods_lucky_draw_,
                      this->free_relive_cnt_,
                      this->dxc_enter_cnt_,
                      this->char_id_);
  }

  char  guild_free_jb_cnt_;        // 公会免费聚宝次数
  char  guild_cost_jb_cnt_;        // 公会花钱聚宝次数
  char  ol_award_get_idx_;         // 当日领取在线奖励索引
  char  login_award_if_get_;       // 当日是否领取登陆奖励
  char  jing_ji_cnt_;              // 竞技次数
  char  free_refresh_jj_cnt_;      // 免费刷新竞技次数
  char  if_mc_rebate_;             // 是否领取月卡的返利
  char  dxc_enter_cnt_;            // 地下城挑战次数
  char  goods_lucky_turn_cnt_;     // 今日实物奖励转盘次数
  char  daily_wx_shared_;          // 每日微信分享状态 0: 未分享 1: 已分享
  char  if_got_goods_lucky_draw_;  // 是否抽取"日常实物抽奖"
  char  free_relive_cnt_;          // 免费复活次数
  short guild_promote_skill_cnt_;  // 公会技能升级次数
  short daily_task_done_cnt_;      // 日常任务已完成次数
  short guild_task_done_cnt_;      // 公会任务已完成次数
  short worship_cnt_;              // 今日崇拜王族次数
  int   con_login_days_;           // 连续登陆天数
  int   char_id_;
  int   clean_time_;               // 清零时间
  int   ol_award_start_time_;      // 当日在线奖励计时开始
};

/**
 * @class daily_clean_info_pool
 *
 * @brief
 */
class daily_clean_info_pool : public singleton<daily_clean_info_pool>
{
  friend class singleton<daily_clean_info_pool>;
public:
  daily_clean_info *alloc() { return this->pool_.alloc(); }

  void release(daily_clean_info *p)
  {
    p->reset();
    this->pool_.release(p);
  }
private:
  daily_clean_info_pool() { }

  obj_pool<daily_clean_info, obj_pool_std_allocator<daily_clean_info> > pool_;
};
#endif // DAILY_CLEAN_INFO_H_

