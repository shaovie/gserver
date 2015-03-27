// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-22 21:46
 */
//========================================================================

#ifndef CHAR_EXTRA_INFO_H_
#define CHAR_EXTRA_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>
#include "singleton.h"

#include "obj_pool.h"
#include "global_macros.h"

// Forward declarations

const char db_tb_char_extra_info[] = "char_extra_info";

/**
 * @class char_extra_info
 *
 * @brief
 */
class char_extra_info
{
public:
  enum
  {
    IDX_CHAR_ID = 0,
    IDX_YES_OR_NO,
    IDX_ZX_VALUE,
    IDX_HUO_YUE_SCORE,
    IDX_GET_LVL_AWARD_IDX,
    IDX_JING_JI_MAX_RANK,
    IDX_LAST_JING_JI_TIME,
    IDX_JING_JI_SCORE,
    IDX_GOT_TI_LI_AWARD_TIME,
    IDX_TI_LI,
    IDX_TI_LI_TIME,
    IDX_SEVEN_DAY_LOGIN,
    IDX_CHECK_SYS_MAIL_TIME,
    IDX_WATER_TREE_TIME,
    IDX_WORSHIP_TIME,
    IDX_JING_LI,
    IDX_JING_LI_TIME,
    IDX_FA_BAO_DJ, IDX_FA_BAO,
    IDX_WT_GOODS_CNT, IDX_LUCKY_TURN_GOODS_CNT, IDX_DAILY_GOODS_LUCKY_DRAW_CNT,
    IDX_XSZC_HONOR, IDX_TOTAL_MSTAR,
    IDX_GUILD_SCP_TIME,
    IDX_END
  };
  enum
  {
    ST_GOT_UPGRADE_NOTICE   = 1L << 0,    // 是否获取过升级公告
    ST_GOT_FIRST_RECHARGE   = 1L << 1,    // 是否获取首充奖励
  };

  char_extra_info() { this->reset(); }

  void reset()
  {
    this->char_id_               = 0;
    this->check_sys_mail_time_   = 0;
    this->yes_or_no_             = 0;
    this->zx_value_              = 0;
    this->huo_yue_score_         = 0;
    this->get_lvl_award_idx_     = 0;
    this->jing_ji_max_rank_      = 0;
    this->last_jing_ji_time_     = 0;
    this->jing_ji_score_         = 0;
    this->got_ti_li_award_time_  = 0;
    this->ti_li_                 = 0;
    this->ti_li_time_            = 0;
    this->water_tree_time_       = 0;
    this->worship_time_          = 0;
    this->jing_li_               = 0;
    this->jing_li_time_          = 0;
    this->fa_bao_dj_             = 0;
    this->wt_goods_cnt_          = 0;
    this->lucky_turn_goods_cnt_  = 0;
    this->daily_goods_lucky_draw_cnt_ = 0;
    this->xszc_honor_            = 0;
    this->total_mstar_           = 0;
    this->guild_scp_time_        = 0;
    ::memset(this->fa_bao_, 0, sizeof(this->fa_bao_));
    ::memset(this->seven_day_login_, 0, sizeof(this->seven_day_login_));
  }
  void operator = (const char_extra_info &cei)
  {
    if (&cei == this) return ;

    this->char_id_               = cei.char_id_;
    this->check_sys_mail_time_   = cei.check_sys_mail_time_;
    this->yes_or_no_             = cei.yes_or_no_;
    this->zx_value_              = cei.zx_value_;
    this->huo_yue_score_         = cei.huo_yue_score_;
    this->get_lvl_award_idx_     = cei.get_lvl_award_idx_;
    this->jing_ji_max_rank_      = cei.jing_ji_max_rank_;
    this->last_jing_ji_time_     = cei.last_jing_ji_time_;
    this->jing_ji_score_         = cei.jing_ji_score_;
    this->got_ti_li_award_time_  = cei.got_ti_li_award_time_;
    this->ti_li_                 = cei.ti_li_;
    this->ti_li_time_            = cei.ti_li_time_;
    this->water_tree_time_       = cei.water_tree_time_;
    this->worship_time_          = cei.worship_time_;
    this->jing_li_               = cei.jing_li_;
    this->jing_li_time_          = cei.jing_li_time_;
    this->fa_bao_dj_             = cei.fa_bao_dj_;
    this->wt_goods_cnt_          = cei.wt_goods_cnt_;
    this->lucky_turn_goods_cnt_  = cei.lucky_turn_goods_cnt_;
    this->daily_goods_lucky_draw_cnt_  = cei.daily_goods_lucky_draw_cnt_;
    this->xszc_honor_            = cei.xszc_honor_;
    this->total_mstar_           = cei.total_mstar_;
    this->guild_scp_time_        = cei.guild_scp_time_;
    ::strncpy(this->fa_bao_, cei.fa_bao_, sizeof(this->fa_bao_) - 1);
    ::strncpy(this->seven_day_login_, cei.seven_day_login_, sizeof(this->seven_day_login_) - 1);
  }
  static const char *all_col()
  {
    return "char_id,yes_or_no,zx_value,huo_yue_score,get_lvl_award_idx,"
      "jing_ji_max_rank,last_jing_ji_time,jing_ji_score,got_ti_li_award_time,ti_li,ti_li_time,"
      "seven_day_login,check_sys_mail_time,water_tree_time,worship_time,"
      "jing_li,jing_li_time,fa_bao_dj,fa_bao,wt_goods_cnt,lucky_turn_goods_cnt,"
      "daily_goods_lucky_draw_cnt,xszc_honor,total_mstar,guild_scp_time";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select %s from %s where char_id=%d",
                      char_extra_info::all_col(),
                      db_tb_char_extra_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)values("
                      "%d,"
                      "%d,%d,%d,"
                      "%d,"
                      "%d,%d,%d,"
                      "%d,%d,%d,"
                      "'%s',"
                      "%d,%d,%d,"
                      "%d,%d,"
                      "%d,'%s',"
                      "%d,%d,%d,"
                      "%d,%d,%d"
                      ")",
                      db_tb_char_extra_info,
                      char_extra_info::all_col(),
                      this->char_id_,
                      this->yes_or_no_, this->zx_value_, this->huo_yue_score_,
                      this->get_lvl_award_idx_,
                      this->jing_ji_max_rank_, this->last_jing_ji_time_, this->jing_ji_score_,
                      this->got_ti_li_award_time_, this->ti_li_, this->ti_li_time_,
                      this->seven_day_login_,
                      this->check_sys_mail_time_, this->water_tree_time_, this->worship_time_,
                      this->jing_li_, this->jing_li_time_,
                      this->fa_bao_dj_, this->fa_bao_,
                      this->wt_goods_cnt_, this->lucky_turn_goods_cnt_, this->daily_goods_lucky_draw_cnt_,
                      this->xszc_honor_, this->total_mstar_, this->guild_scp_time_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "yes_or_no=%d,zx_value=%d,huo_yue_score=%d,"
                      "get_lvl_award_idx=%d,"
                      "jing_ji_max_rank=%d,last_jing_ji_time=%d,jing_ji_score=%d,"
                      "got_ti_li_award_time=%d,ti_li=%d,ti_li_time=%d,"
                      "seven_day_login='%s',"
                      "check_sys_mail_time=%d,water_tree_time=%d,worship_time=%d,"
                      "jing_li=%d,jing_li_time=%d,"
                      "fa_bao_dj=%d,fa_bao='%s',"
                      "wt_goods_cnt=%d,lucky_turn_goods_cnt=%d,daily_goods_lucky_draw_cnt=%d,"
                      "xszc_honor=%d,total_mstar=%d,guild_scp_time=%d"
                      " where char_id=%d",
                      db_tb_char_extra_info,
                      this->yes_or_no_, this->zx_value_, this->huo_yue_score_,
                      this->get_lvl_award_idx_,
                      this->jing_ji_max_rank_, this->last_jing_ji_time_, this->jing_ji_score_,
                      this->got_ti_li_award_time_, this->ti_li_, this->ti_li_time_,
                      this->seven_day_login_,
                      this->check_sys_mail_time_, this->water_tree_time_, this->worship_time_,
                      this->jing_li_, this->jing_li_time_,
                      this->fa_bao_dj_, this->fa_bao_,
                      this->wt_goods_cnt_, this->lucky_turn_goods_cnt_, this->daily_goods_lucky_draw_cnt_,
                      this->xszc_honor_, this->total_mstar_, this->guild_scp_time_,
                      this->char_id_);
  }

  char  fa_bao_dj_;                      // 法宝等阶
  char  wt_goods_cnt_;                   // 灌溉中获得实物奖励个数
  char  lucky_turn_goods_cnt_;           // 微信分享幸运转盘中获得实物奖励个数
  char  daily_goods_lucky_draw_cnt_;     // 日常实物抽奖次数
  short huo_yue_score_;                  // 活跃度积分
  short get_lvl_award_idx_;              // 等级奖励
  short ti_li_;                          // 体力值
  short jing_li_;                        // 精力值
  int  char_id_;
  int  yes_or_no_;                       // 保存一些是否状态
  int  check_sys_mail_time_;             // 检查系统邮件的时间
  int  zx_value_;                        // 占星点数值
  int  jing_ji_max_rank_;                // 竞技场最好排名
  int  last_jing_ji_time_;               // 上次竞技场挑战时间
  int  jing_ji_score_;                   // 竞技场积分
  int  got_ti_li_award_time_;            // 领取活动奖励体力值时间
  int  ti_li_time_;                      // 体力更新时间
  int  worship_time_;                    // 崇拜王族时间
  int  water_tree_time_;                 // 浇树时间
  int  jing_li_time_;                    // 精力恢复时间
  int  guild_scp_time_;                  // 公会副本时间
  int  xszc_honor_;                      // 雄狮战场荣誉
  int  total_mstar_;                     // 关卡星级总和
  char seven_day_login_[8];              // 七天登陆
  char fa_bao_[FA_BAO_PART_CNT * 2];     // 法宝穿戴信息
};

/**
 * @class char_extra_info_pool
 *
 * @brief
 */
class char_extra_info_pool : public singleton<char_extra_info_pool>
{
  friend class singleton<char_extra_info_pool>;
public:
  char_extra_info *alloc() { return this->pool_.alloc(); }

  void release(char_extra_info *p)
  {
    p->reset();
    this->pool_.release(p);
  }
private:
  char_extra_info_pool() { }
  obj_pool<char_extra_info, obj_pool_std_allocator<char_extra_info> > pool_;
};
#endif // CHAR_EXTRA_INFO_H_

