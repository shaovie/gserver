#include "preload_db.h"
#include "svc_config.h"
#include "mysql_db.h"
#include "sys_log.h"
#include "util.h"
#include "all_char_info.h"
#include "account_info.h"
#include "char_info.h"
#include "char_extra_info.h"
#include "recharge_log.h"
#include "consume_log.h"
#include "social_info.h"
#include "guild_info.h"
#include "guild_member_info.h"
#include "guild_apply_info.h"
#include "guild_module.h"
#include "market_info.h"
#include "market_module.h"
#include "rank_module.h"
#include "tui_tu_log.h"
#include "tui_tu_module.h"
#include "forbid_opt.h"
#include "forbid_opt_module.h"
#include "char_attr.h"
#include "sys.h"
#include "vip_info.h"
#include "notice_info.h"
#include "service_info.h"
#include "gm_notice_module.h"
#include "jing_ji_rank.h"
#include "jing_ji_module.h"
#include "name_storage_cfg.h"
#include "ltime_act.h"
#include "ltime_act_module.h"
#include "lucky_turn.h"
#include "lucky_turn_score.h"
#include "lucky_turn_module.h"
#include "worship_info.h"
#include "water_tree_info.h"
#include "mobai_module.h"
#include "water_tree_module.h"
#include "water_tree_info.h"
#include "tianfu_skill_info.h"
#include "lueduo_item.h"
#include "lue_duo_module.h"
#include "goods_award_module.h"

// Lib header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

int preload_db::do_preload_db()
{
  mysql_db db;
  if (db.open(svc_config::instance()->db_port(),
              svc_config::instance()->db_host(),
              svc_config::instance()->db_user(),
              svc_config::instance()->db_passwd(),
              svc_config::instance()->db_name()) != 0)
  {
    e_log->error("mysql db open failed when preload!");
    return -1;
  }

  if (preload_db::preload_service_info(&db) != 0) return -1;
  if (preload_db::preload_account_info(&db) != 0) return -1;
  if (preload_db::preload_char_info(&db) != 0) return -1;
  if (preload_db::preload_char_extra_info(&db) != 0) return -1;
  if (preload_db::preload_recharge_log(&db) != 0) return -1;
  if (preload_db::preload_consume_log(&db) != 0) return -1;
  if (preload_db::preload_char_attr(&db) != 0) return -1;
  if (preload_db::preload_social_info(&db) != 0) return -1;
  if (preload_db::preload_guild_info(&db) != 0) return -1;
  if (preload_db::preload_guild_member_info(&db) != 0) return -1;
  if (preload_db::preload_guild_apply_info(&db) != 0) return -1;
  if (preload_db::preload_max_market_id(&db) != 0) return -1;
  if (preload_db::preload_market_info(&db) != 0) return -1;
  if (preload_db::preload_tui_tu_log(&db) != 0) return -1;
  if (preload_db::preload_forbid_opt(&db) != 0) return -1;
  if (preload_db::preload_vip_info(&db) != 0) return -1; // must after preload_char_info
  if (preload_db::preload_notice_info(&db) != 0) return -1;
  if (preload_db::preload_jing_ji_rank(&db) != 0) return -1;
  if (preload_db::preload_ltime_act(&db) != 0) return -1;
  if (preload_db::preload_lucky_turn(&db) != 0) return -1;
  if (preload_db::preload_lucky_turn_score(&db) != 0) return -1;
  if (preload_db::preload_worship_info(&db) != 0) return -1;
  if (preload_db::preload_water_tree(&db) != 0) return -1;
  if (preload_db::preload_tianfu_skill(&db) != 0) return -1;
  if (preload_db::preload_lueduo_item(&db) != 0) return -1;

  s_log->rinfo("preload all db ok!");
  return 0;
}
int preload_db::preload_service_info(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select "
                       "unix_timestamp(open_time)," // 0
                       "opened,"                    // 1
                       "ghz_closed_time"           // 2
                       " from %s",
                       db_tb_service_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (row != NULL)
  {
    sys::svc_info->open_time         = ::atoi(row[0]);
    sys::svc_info->opened            = ::atoi(row[1]);
    sys::svc_info->ghz_closed_time   = ::atoi(row[2]);
  }else
  {
    e_log->error("load service info error!");
    return -1;
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_account_info(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select %s from %s",
                       account_info::all_col(),
                       db_tb_account_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    account_brief_info *abi = new account_brief_info();
    ::strncpy(abi->ac_info_->account_, row[account_info::IDX_ACCOUNT], sizeof(abi->ac_info_->account_) - 1);
    abi->ac_info_->track_st_   = ::atoi(row[account_info::IDX_TRACK_ST]);
    abi->ac_info_->diamond_    = ::atoi(row[account_info::IDX_DIAMOND]);
    abi->ac_info_->char_id_    = ::atoi(row[account_info::IDX_CHAR_ID]);
    ::strncpy(abi->ac_info_->channel_, row[account_info::IDX_CHANNEL], sizeof(abi->ac_info_->channel_) - 1);
    abi->ac_info_->c_time_     = ::atoi(row[account_info::IDX_C_TIME]);
    all_char_info::instance()->insert_account_info(abi->ac_info_->account_, abi);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_recharge_log(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select char_id,rc_time,value from %s",
                       db_tb_recharge_log);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
    char_brief_info::on_recharge_ok(::atoi(row[0]), ::atoi(row[1]), ::atoi(row[2]));

  mysql_free_result(res);
  return 0;
}
int preload_db::preload_consume_log(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select char_id,cs_type,cs_time,value from %s",
                       db_tb_consume_log);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
    char_brief_info::on_consume_diamond(::atoi(row[0]), ::atoi(row[1]), ::atoi(row[2]), ::atoi(row[3]));

  mysql_free_result(res);
  return 0;
}
int preload_db::preload_char_info(mysql_db *db)
{
  char sql[512] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select char_id," // 0
                       "account,"        // 1
                       "name,"           // 2
                       "career,"         // 3
                       "lvl,"            // 4
                       "out_time,"       // 5
                       "zhan_li,"        // 6
                       "sin_val,"        // 7
                       "exp,"            // 8
                       "cur_title,"      // 9
                       "scene_cid"       // 10
                       " from %s",
                       db_tb_char_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  int max_char_id = 0;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    char_brief_info *cbi = new char_brief_info();
    cbi->char_id_    = ::atoi(row[0]);
    char account[MAX_ACCOUNT_LEN + 1];
    ::strncpy(account, row[1], sizeof(account) - 1);
    ::strncpy(cbi->name_, row[2], sizeof(cbi->name_) - 1);
    cbi->career_     = ::atoi(row[3]);
    cbi->lvl_        = ::atoi(row[4]);
    cbi->out_time_   = ::atoi(row[5]);
    cbi->zhan_li_    = ::atoi(row[6]);
    cbi->sin_val_    = ::atoi(row[7]);
    int64_t exp      = ::atol(row[8]);
    cbi->title_cid_  = ::atoi(row[9]);
    cbi->scene_cid_  = ::atoi(row[10]);
    all_char_info::instance()->insert_char_info(cbi);
    account_brief_info *abi = all_char_info::instance()->get_account_brief_info(account);
    if (abi != NULL)
      abi->char_list_.push_back(cbi);

    if (cbi->char_id_ > max_char_id
        && cbi->char_id_ / CHAR_ID_RANGE == svc_config::instance()->service_sn())
      max_char_id = cbi->char_id_;
    rank_module::on_char_get_exp(cbi->char_id_, 0, 0, cbi->lvl_, exp, true);
    rank_module::on_char_zhanli_change(cbi->char_id_, 0, cbi->zhan_li_, true);
    sys::select_career(cbi->career_);
  }
  mysql_free_result(res);

  sys::last_char_id = max_char_id;
  if (sys::last_char_id == 0)
    sys::last_char_id = CHAR_ID_RANGE * svc_config::instance()->service_sn();
  s_log->rinfo("last char id is %d", sys::last_char_id);
  name_storage_cfg::instance()->adjust_data();
  return 0;
}
int preload_db::preload_char_extra_info(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select "
                       "char_id,"              // 0
                       "fa_bao_dj,"            // 1
                       "fa_bao,"               // 2
                       "wt_goods_cnt,"         // 3
                       "lucky_turn_goods_cnt," // 4
                       "total_mstar"           // 5
                       " from %s",
                       db_tb_char_extra_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  int max_wt_goods_cnt = 0;
  int max_lucky_turn_goods_cnt = 0;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    int char_id = ::atoi(row[0]);
    char_brief_info *cbi =
      all_char_info::instance()->get_char_brief_info(char_id);
    if (cbi != NULL)
    {
      cbi->fa_bao_dj_ = ::atoi(row[1]);
      ::strncpy(cbi->fa_bao_, row[2], sizeof(cbi->fa_bao_) - 1);
      max_wt_goods_cnt = util::max(::atoi(row[3]), max_wt_goods_cnt);
      max_lucky_turn_goods_cnt = util::max(::atoi(row[4]), max_lucky_turn_goods_cnt);
      int total_mstar = ::atoi(row[5]);
      rank_module::on_char_mstar_change(char_id, 0, total_mstar, true);
      cbi->total_mstar_ = total_mstar;
    }
  }
  goods_award_module::on_preload_ok(max_wt_goods_cnt, max_lucky_turn_goods_cnt);
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_char_attr(mysql_db *db)
{
  char sql[512] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select %s from %s",
                       char_attr::all_col(),
                       db_tb_char_attr);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    char_brief_info *cbi =
      all_char_info::instance()->get_char_brief_info(::atoi(row[char_attr::IDX_CHAR_ID]));
    if (cbi != NULL)
    {
      cbi->attr_[ATTR_T_HP]         = ::atoi(row[char_attr::IDX_HP]);
      cbi->attr_[ATTR_T_MP]         = ::atoi(row[char_attr::IDX_MP]);
      cbi->attr_[ATTR_T_GONG_JI]    = ::atoi(row[char_attr::IDX_GONG_JI]);
      cbi->attr_[ATTR_T_FANG_YU]    = ::atoi(row[char_attr::IDX_FANG_YU]);
      cbi->attr_[ATTR_T_MING_ZHONG] = ::atoi(row[char_attr::IDX_MING_ZHONG]);
      cbi->attr_[ATTR_T_SHAN_BI]    = ::atoi(row[char_attr::IDX_SHAN_BI]);
      cbi->attr_[ATTR_T_BAO_JI]     = ::atoi(row[char_attr::IDX_BAO_JI]);
      cbi->attr_[ATTR_T_KANG_BAO]   = ::atoi(row[char_attr::IDX_KANG_BAO]);
      cbi->attr_[ATTR_T_SHANG_MIAN] = ::atoi(row[char_attr::IDX_SHANG_MIAN]);
      cbi->zhu_wu_cid_              = ::atoi(row[char_attr::IDX_ZHU_WU]);
      cbi->fu_wu_cid_               = ::atoi(row[char_attr::IDX_FU_WU]);
    }
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_social_info(mysql_db *db)
{
  char sql[512] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select char_id,socialer_id from %s",
                       db_tb_social_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  int cnt = 0;
  while ((row = mysql_fetch_row(res)))
  {
    int char_id     = ::atoi(row[0]);
    int socialer_id = ::atoi(row[1]);
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(socialer_id);
    if (cbi != NULL)
    {
      ++cnt;
      cbi->socialer_list_.push_back(char_id);
    }
  }
  s_log->rinfo("socialer cnt total size %d", cnt);
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_guild_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_guild_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (guild_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_guild_info);
    return -1;
  }
  int max_guild_id = 0;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    guild_info *info = new guild_info();
    info->guild_id_       = ::atoi(row[guild_info::IDX_GUILD_ID]);
    info->chairman_id_    = ::atoi(row[guild_info::IDX_CHAIRMAN_ID]);
    info->lvl_            = ::atoi(row[guild_info::IDX_LVL]);
    info->c_time_         = ::atoi(row[guild_info::IDX_C_TIME]);
    info->apply_dj_limit_ = ::atoi(row[guild_info::IDX_APPLY_DJ_LIMIT]);
    info->guild_resource_ = ::atoi(row[guild_info::IDX_GUILD_RESOURCE]);
    info->zq_lvl_         = ::atoi(row[guild_info::IDX_ZQ_LVL]);
    info->tld_lvl_        = ::atoi(row[guild_info::IDX_TLD_LVL]);
    info->jbd_lvl_        = ::atoi(row[guild_info::IDX_JBD_LVL]);
    info->is_ghz_winner_  = ::atoi(row[guild_info::IDX_IS_GHZ_WINNER]);
    info->last_summon_boss_time_ = ::atoi(row[guild_info::IDX_LAST_SUMMON_BOSS_TIME]);
    info->apply_zl_limit_ = ::atoi(row[guild_info::IDX_APPLY_ZL_LIMIT]);
    info->scp_lvl_        = ::atoi(row[guild_info::IDX_SCP_LVL]);
    info->scp_last_open_time_ = ::atoi(row[guild_info::IDX_SCP_LAST_OPEN_TIME]);
    ::strncpy(info->name_, row[guild_info::IDX_NAME], sizeof(info->name_) - 1);
    ::strncpy(info->purpose_, row[guild_info::IDX_PURPOSE], sizeof(info->purpose_) - 1);

    if (info->guild_id_ > max_guild_id
        && info->guild_id_ / GUILD_ID_RANGE == svc_config::instance()->service_sn())
      max_guild_id = info->guild_id_;
    guild_module::do_insert_guild(info);
  }
  mysql_free_result(res);

  sys::last_guild_id = max_guild_id;
  if (sys::last_guild_id == 0)
    sys::last_guild_id = GUILD_ID_RANGE * svc_config::instance()->service_sn();
  s_log->rinfo("last guild id is %d", sys::last_guild_id);
  return 0;
}
int preload_db::preload_guild_member_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_guild_member_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (guild_member_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_guild_member_info);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    guild_member_info *info = new guild_member_info();
    info->char_id_       = ::atoi(row[guild_member_info::IDX_CHAR_ID]);
    info->guild_id_      = ::atoi(row[guild_member_info::IDX_GUILD_ID]);
    info->contrib_       = ::atoi(row[guild_member_info::IDX_CONTRIB]);
    info->total_contrib_ = ::atoi(row[guild_member_info::IDX_TOTAL_CONTRIB]);
    info->position_      = ::atoi(row[guild_member_info::IDX_POSITION]);
    guild_module::do_insert_guild_member(info);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_guild_apply_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_guild_apply_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (guild_apply_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_guild_apply_info);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    guild_apply_info *info = new guild_apply_info();
    info->char_id_    = ::atoi(row[guild_apply_info::IDX_CHAR_ID]);
    info->guild_id_   = ::atoi(row[guild_apply_info::IDX_GUILD_ID]);
    info->apply_time_ = ::atoi(row[guild_apply_info::IDX_APPLY_TIME]);

    guild_module::do_insert_guild_apply(info);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_max_market_id(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select market_id from %s order by market_id",
                       db_tb_market_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  int sn = svc_config::instance()->service_sn();
  int max_market_id = sn * MARKET_ID_RANGE;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    int market_id = ::atoi(row[0]);
    if (market_id / MARKET_ID_RANGE != sn) continue;
    for (++max_market_id; max_market_id < market_id; ++max_market_id)
      market_module::market_id_pool.push_back(max_market_id);
  }
  mysql_free_result(res);
  market_module::max_market_id = max_market_id;
  while (market_module::market_id_pool.size() < 1000)
    market_module::market_id_pool.push_back(++market_module::max_market_id);

  return 0;
}
int preload_db::preload_market_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_market_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (market_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_market_info);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    market_info *info   = market_info_pool::instance()->alloc();
    info->market_id_    = ::atoi(row[market_info::IDX_MARKET_ID]);
    info->char_id_      = ::atoi(row[market_info::IDX_CHAR_ID]);
    info->price_        = ::atoi(row[market_info::IDX_PRICE]);
    info->price_type_   = ::atoi(row[market_info::IDX_PRICE_TYPE]);
    info->begin_time_   = ::atoi(row[market_info::IDX_BEGIN_TIME]);

    info->item_          = item_obj_pool::instance()->alloc();
    info->item_->cid_    = ::atoi(row[market_info::IDX_CID]);
    info->item_->amount_ = ::atoi(row[market_info::IDX_AMOUNT]);
    info->item_->attr_float_coe_ = ::atoi(row[market_info::IDX_ATTR_FLOAT_COE]);

    int len = ::strlen(row[market_info::IDX_EXTRA_INFO]);
    if (len > 0)
    {
      info->item_->extra_info_ = mblock_pool::instance()->alloc(len + 1);
      info->item_->extra_info_->copy(row[market_info::IDX_EXTRA_INFO], len);
      info->item_->extra_info_->set_eof();
    }

    market_module::do_insert_market_info(info);
  }
  market_module::market_init();

  mysql_free_result(res);
  return 0;
}
int preload_db::preload_tui_tu_log(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_tui_tu_log);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (tui_tu_log::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_tui_tu_log);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    tui_tu_log ttl;
    ttl.char_id_    = ::atoi(row[tui_tu_log::IDX_CHAR_ID]);
    ttl.cid_        = ::atoi(row[tui_tu_log::IDX_CID]);
    ttl.used_time_  = ::atoi(row[tui_tu_log::IDX_USED_TIME]);
    ttl.state_      = ::atoi(row[tui_tu_log::IDX_STATE]);
    tui_tu_module::on_load_one_log(&ttl);
  }

  mysql_free_result(res);
  return 0;
}
int preload_db::preload_forbid_opt(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_forbid_opt);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (forbid_opt::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_forbid_opt);
    return -1;
  }
  int max_forbid_id = 0;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    forbid_opt v;
    v.id_         = ::atoi(row[forbid_opt::IDX_ID]);
    v.opt_        = ::atoi(row[forbid_opt::IDX_OPT]);
    v.begin_time_ = ::atoi(row[forbid_opt::IDX_BEGIN_TIME]);
    v.end_time_   = ::atoi(row[forbid_opt::IDX_END_TIME]);
    ::strncpy(v.target_, row[forbid_opt::IDX_TARGET], sizeof(v.target_) - 1);
    if (v.id_ > max_forbid_id
        && v.id_ / FORBID_ID_RANGE == svc_config::instance()->service_sn())
      max_forbid_id = v.id_;
    forbid_opt_module::do_insert_forbid_opt(v, true);
  }
  mysql_free_result(res);

  sys::last_forbid_id = max_forbid_id;
  if (sys::last_forbid_id == 0)
    sys::last_forbid_id = FORBID_ID_RANGE * svc_config::instance()->service_sn();
  s_log->rinfo("last forbid id is %d", sys::last_forbid_id);
  return 0;
}
int preload_db::preload_vip_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_vip_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (vip_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_vip_info);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    const int char_id = ::atoi(row[vip_info::IDX_CHAR_ID]);
    char_brief_info *cbi =  all_char_info::instance()->get_char_brief_info(char_id);
    if (cbi == NULL) continue;
    cbi->vip_ = ::atoi(row[vip_info::IDX_VIP_LVL]);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_notice_info(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select id,"
                       "unix_timestamp(begin_time)," // 1
                       "unix_timestamp(end_time),"   // 2
                       "interval_time,"              // 3
                       "content"                     // 4
                       " from %s",
                       db_tb_notice_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (notice_info::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_notice_info);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    notice_info *ni = new notice_info();
    ni->id_             = ::atoi(row[notice_info::IDX_ID]);
    ni->begin_time_     = ::atoi(row[notice_info::IDX_BEGIN_TIME]);
    ni->end_time_       = ::atoi(row[notice_info::IDX_END_TIME]);
    ni->interval_time_  = ::atoi(row[notice_info::IDX_INTERVAL_TIME]);
    ::strncpy(ni->content_, row[notice_info::IDX_CONTENT], sizeof(ni->content_) - 1);

    gm_notice_module::load_notice_from_db(ni);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_jing_ji_rank(mysql_db *db)
{
  char sql[128] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_jing_ji_rank);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (jing_ji_rank::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_jing_ji_rank);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    jing_ji_rank *jjr = new jing_ji_rank();
    jjr->char_id_    = ::atoi(row[jing_ji_rank::IDX_CHAR_ID]);
    jjr->rank_       = ::atoi(row[jing_ji_rank::IDX_RANK]);
    jjr->award_time_ = ::atoi(row[jing_ji_rank::IDX_AWARD_TIME]);
    jing_ji_module::on_load_on_rank(jjr);
  }
  mysql_free_result(res);
  return 0;
}
int preload_db::preload_ltime_act(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select "
                       "act_id,"                     // 0
                       "unix_timestamp(begin_time)," // 1
                       "unix_timestamp(end_time)"    // 2
                       " from %s",
                       db_tb_ltime_act);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (ltime_act::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_ltime_act);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    ltime_act lta;
    lta.act_id_      = ::atoi(row[ltime_act::IDX_ACT_ID]);
    lta.begin_time_  = ::atoi(row[ltime_act::IDX_BEGIN_TIME]);
    lta.end_time_    = ::atoi(row[ltime_act::IDX_END_TIME]);
    ltime_act_module::do_update_act(&lta, true);
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_lucky_turn(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_lucky_turn);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (lucky_turn::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_lucky_turn);
    return -1;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (row != NULL)
  {
    int v = ::atoi(row[lucky_turn::IDX_ACC_AWARD]);
    lucky_turn_module::on_load_lucky_turn(v);
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_lucky_turn_score(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select * from %s",
                       db_tb_lucky_turn_score);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  if (lucky_turn_score::IDX_END != mysql_num_fields(res))
  {
    e_log->error("%s table error!", db_tb_lucky_turn_score);
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    lucky_turn_module::on_load_lucky_turn_score(::atoi(row[lucky_turn_score::IDX_CHAR_ID]),
                                                ::atoi(row[lucky_turn_score::IDX_SCORE]),
                                                ::atoi(row[lucky_turn_score::IDX_TURN_TIME]),
                                                true);
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_worship_info(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select "
                       "left_coin,"                 // 1
                       "acc_coin,"                  // 2
                       "acc_worship_cnt"            // 3
                       " from %s",
                       db_tb_worship_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (row != NULL)
  {
    mobai_module::wrc_info->left_coin_        = ::atoi(row[0]);
    mobai_module::wrc_info->acc_coin_         = ::atoi(row[1]);
    mobai_module::wrc_info->acc_worship_cnt_  = ::atoi(row[2]);
  }else
  {
    e_log->error("load service info error!");
    return -1;
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_water_tree(mysql_db *db)
{
  char sql[1024] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select "
                       "lvl,cheng_zhang"
                       " from %s",
                       db_tb_water_tree);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (row != NULL)
  {
    water_tree_module::wti.lvl_         = ::atoi(row[0]);
    water_tree_module::wti.cheng_zhang_ = ::atoi(row[1]);
  }else
  {
    len = ::snprintf(sql, sizeof(sql),
                     "insert into %s(lvl,cheng_zhang) values(1,0)",
                     db_tb_water_tree);
    if (db->store(sql, len) != 0)
    {
      e_log->error("do [%s] failed! err: %s",
                   sql,
                   db->mysql_strerror());
      return -1;
    }
    water_tree_module::wti.lvl_         = 1;
    water_tree_module::wti.cheng_zhang_ = 0;
  }
  mysql_free_result(res);

  return 0;
}
int preload_db::preload_tianfu_skill(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select char_id,cid,lvl from %s",
                       db_tb_tianfu_skill_info);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(::atoi(row[0]));
    if (cbi != NULL)
      cbi->tianfu_skill_list_.push_back(pair_t<int> (::atoi(row[1]), ::atoi(row[2])));
  }

  mysql_free_result(res);
  return 0;
}
int preload_db::preload_lueduo_item(mysql_db *db)
{
  char sql[256] = {0};
  int len = ::snprintf(sql, sizeof(sql),
                       "select %s from %s",
                       lueduo_item::all_col(), db_tb_lueduo_item);
  MYSQL_RES *res = db->query(sql, len);
  if (res == NULL)
  {
    e_log->error("do [%s] failed! err: %s",
                 sql,
                 db->mysql_strerror());
    return -1;
  }
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)))
  {
    lue_duo_module::deal_preload(::atoi(row[lueduo_item::IDX_CHAR_ID]),
                                 ::atoi(row[lueduo_item::IDX_CID]),
                                 ::atoi(row[lueduo_item::IDX_BIND_AMOUNT]),
                                 ::atoi(row[lueduo_item::IDX_UNBIND_AMOUNT]));
  }

  mysql_free_result(res);
  return 0;
}
