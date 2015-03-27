#include "jing_ji_module.h"
#include "jing_ji_cfg.h"
#include "jing_ji_rank.h"
#include "jing_ji_log.h"
#include "global_param_cfg.h"
#include "all_char_info.h"
#include "istream.h"
#include "guild_module.h"
#include "skill_info.h"
#include "mblock_pool.h"
#include "monster_mgr.h"
#include "monster_obj.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "player_copy_mst.h"
#include "db_proxy.h"
#include "sys_log.h"
#include "scp_module.h"
#include "scp_mgr.h"
#include "scp_config.h"
#include "mail_info.h"
#include "mail_module.h"
#include "mail_config.h"
#include "time_util.h"
#include "char_msg_queue_module.h"
#include "char_db_msg_queue.h"
#include "huo_yue_du_module.h"
#include "cheng_jiu_module.h"
#include "vip_module.h"
#include "clsid.h"
#include "aev.h"
#include "sys.h"
#include "behavior_id.h"
#include "notice_module.h"
#include "task_module.h"

// Lib header
#include <map>
#include <set>

#define ONCE_SEND_MAIL_AMT            50
#define REFRESH_COMPETITOR_COUNT      4

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("jing_ji");
static ilog_obj *e_log = err_log::instance()->get_ilog("jing_ji");

bool jing_ji_module::today_send_award = false;

class jing_ji_match_info
{
public:
  int competitor_id_ ;      // 对手ID
  int player_copy_mst_id_; // 角色镜像ID
};
typedef std::map<int/*char_id*/, jing_ji_rank *> jing_ji_char_rank_map_t;
typedef std::map<int/*char_id*/, jing_ji_rank *>::iterator jing_ji_char_rank_map_itor;
typedef std::map<int/*rank*/, int/*char_id*/> jing_ji_rank_char_map_t;
typedef std::map<int/*rank*/, int/*char_id*/>::iterator jing_ji_rank_char_map_itor;

typedef std::map<int/*char_id*/, jing_ji_match_info *> jing_ji_match_map_t;
typedef std::map<int/*char_id*/, jing_ji_match_info *>::iterator jing_ji_match_map_itor;

typedef std::set<int/*char_id*/> on_jing_ji_char_set_t;
typedef std::set<int/*char_id*/>::iterator on_jing_ji_char_set_itor;

static jing_ji_char_rank_map_t s_char_rank_map;
static jing_ji_rank_char_map_t s_rank_char_map;
static jing_ji_match_map_t s_jing_ji_match_map;
static on_jing_ji_char_set_t s_on_jing_ji_char_set;
static int s_max_rank = 0;

class jing_ji_award_timer : public ev_handler
{
public:
  jing_ji_award_timer()
  {
    for (jing_ji_rank_char_map_itor itor = s_rank_char_map.begin();
         itor != s_rank_char_map.end();
         ++itor)
      this->award_char_list_.push_back(pair_t<int>(itor->second, itor->first));
  }
  virtual int handle_timeout(const time_value &)
  {
    for (int i = 0; i < ONCE_SEND_MAIL_AMT; ++i)
    {
      if (this->award_char_list_.empty()) return -1;
      pair_t<int> v = this->award_char_list_.pop_front();
      jing_ji_module::do_send_jing_ji_award(v.first_, v.second_);
    }

    return 0;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    delete this;
    return 0;
  }
private:
  ilist<pair_t<int> > award_char_list_;
};

static int build_one_jing_ji_player_info(const int rank, out_stream &os)
{
  int char_id = s_rank_char_map[rank];
  if (char_id == 0) return -1;

  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return -1;
  os << char_id
    << rank
    << stream_ostr(cbi->name_, ::strlen(cbi->name_))
    << cbi->career_
    << cbi->lvl_
    << cbi->zhan_li_;

  return 0;
}
static void build_jing_ji_log_info(jing_ji_log *jjl, out_stream &os)
{
  os << jjl->char_id_;
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(jjl->char_id_);
  if (cbi == NULL)
    os << stream_ostr(" ", 1);
  else
    os << stream_ostr(cbi->name_, ::strlen(cbi->name_));

  os << jjl->competitor_id_;
  cbi = all_char_info::instance()->get_char_brief_info(jjl->competitor_id_);
  if (cbi == NULL)
    os << stream_ostr(" ", 1);
  else
    os << stream_ostr(cbi->name_, ::strlen(cbi->name_));

  os << jjl->rank_change_;
}
int jing_ji_module::handle_db_get_other_skill_list_result(player_obj *, in_stream &is)
{
  int mst_id = 0;
  int cnt = 0;
  is >> mst_id >> cnt;
  monster_obj *mo = monster_mgr::instance()->find(mst_id);
  if (mo == NULL) return 0;

  for (int i = 0; i < cnt; ++i)
  {
    char skill_bf[sizeof(skill_info) + 4] = {0};
    stream_istr si(skill_bf, sizeof(skill_bf));
    is >> si;
    skill_info *sinfo = (skill_info *)skill_bf;

    mblock *mb = mblock_pool::instance()->alloc(sizeof(int) * 2);
    *mb << sinfo->cid_ << sinfo->lvl_;
    mo->post_aev(AEV_PUSH_SKILL, mb);
  }
  return 0;
}
int jing_ji_module::handle_db_get_jing_ji_log_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  out_stream os(client::send_buf, client::send_buf_len);
  os << cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(jing_ji_log) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    jing_ji_log *jjl = (jing_ji_log *)bf;

    build_jing_ji_log_info(jjl, os);
  }
  player->send_respond_ok(RES_GET_JING_JI_LOG_LIST, &os);
  return 0;
}
void jing_ji_module::destroy(player_obj *player)
{
  jing_ji_match_map_itor itor = s_jing_ji_match_map.find(player->id());
  if (itor == s_jing_ji_match_map.end()) return ;

  s_on_jing_ji_char_set.erase(itor->first);
  s_on_jing_ji_char_set.erase(itor->second->competitor_id_);

  delete itor->second;
  s_jing_ji_match_map.erase(itor);
}
void jing_ji_module::on_load_on_rank(jing_ji_rank *jjr)
{
  s_char_rank_map.insert(std::make_pair(jjr->char_id_, jjr));
  s_rank_char_map[jjr->rank_] = jjr->char_id_;
  if (jjr->rank_ > s_max_rank)
    s_max_rank = jjr->rank_;
}
void jing_ji_module::on_enter_scene(player_obj *player)
{
  if (player->scene_cid() != global_param_cfg::jing_ji_map)
    return ;

  jing_ji_match_map_itor itor = s_jing_ji_match_map.find(player->id());
  if (itor != s_jing_ji_match_map.end())
  {
    monster_obj *mo = monster_mgr::instance()->find(itor->second->player_copy_mst_id_);
    if (mo == NULL) return ;
    mo->post_aev(AEV_TO_PATROL, NULL);
  }
}
void jing_ji_module::on_after_login(player_obj *player)
{
  if (player->lvl() >= global_param_cfg::jing_ji_lvl)
    player->do_notify_jing_ji_score_to_clt();
}
void jing_ji_module::on_char_lvl_up(player_obj *player)
{
  if (player->lvl() == global_param_cfg::jing_ji_lvl
      && s_char_rank_map.find(player->id()) == s_char_rank_map.end())
  {
    jing_ji_module::db_update_rank(player->db_sid(), player->id(), ++s_max_rank);
  }
}
void jing_ji_module::on_jing_ji_end(const int char_id, const bool win)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return;

  jing_ji_match_map_itor itor = s_jing_ji_match_map.find(char_id);
  if (itor == s_jing_ji_match_map.end()) return;
  jing_ji_match_info *info = itor->second;

  int rank = 0;
  int old_max_rank = player->char_extra_info_->jing_ji_max_rank_;
  jing_ji_char_rank_map_itor itor_char = s_char_rank_map.find(char_id);
  jing_ji_char_rank_map_itor itor_competitor = s_char_rank_map.find(info->competitor_id_);
  if (win
      && itor_char != s_char_rank_map.end()
      && itor_competitor != s_char_rank_map.end())
  {
    int char_rank = itor_char->second->rank_;
    int competitor_rank = itor_competitor->second->rank_;

    int competitor_sid = info->competitor_id_;
    player_obj *competitor = player_mgr::instance()->find(info->competitor_id_);
    if (competitor != NULL)
      competitor_sid = competitor->db_sid();

    jing_ji_module::db_update_rank(player->db_sid(), char_id, competitor_rank);
    jing_ji_module::db_update_rank(competitor_sid, info->competitor_id_, char_rank);

    jing_ji_module::get_max_rank_award(player);

    rank = char_rank - competitor_rank;

    if (competitor_rank <= 3)
    {
      char_brief_info *cbi =
        all_char_info::instance()->get_char_brief_info(info->competitor_id_);
      if (cbi != NULL)
        notice_module::jing_ji_win(player->id(), player->name(),
                                   info->competitor_id_, cbi->name_,
                                   competitor_rank);
    }
  }

  jing_ji_module::insert_jing_ji_log_2_db(player, info->competitor_id_, rank);

  out_stream os(client::send_buf, client::send_buf_len);
  os << (win ? (char)1 : (char)0);
  if (itor_char != s_char_rank_map.end())
    os << itor_char->second->rank_;
  else
    os << 0;
  os << old_max_rank;
  player->send_request(NTF_JING_JI_CHALLENGE_END, &os);

  jing_ji_module::destroy(player);
}
void jing_ji_module::get_max_rank_award(player_obj *player)
{
  int old_rank = player->char_extra_info_->jing_ji_max_rank_;
  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(player->id());
  if (itor == s_char_rank_map.end()
      || (itor->second->rank_ >= old_rank && old_rank != 0))
    return;

  int rank = itor->second->rank_;

  player->char_extra_info_->jing_ji_max_rank_ = rank;
  player->db_save_char_extra_info();

  if (rank == 1)
    notice_module::jing_ji_rank_top(player->id(), player->name());

  int bind_diamond = jing_ji_first_award_cfg::instance()->get_diamond(old_rank, rank);
  int coin = jing_ji_first_award_cfg::instance()->get_coin(old_rank, rank);
  int ji_fen = jing_ji_first_award_cfg::instance()->get_ji_fen(old_rank, rank);
  if (bind_diamond != 0 || coin != 0 || ji_fen != 0)
  {
    const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::MAIL_JJ_FRIST_RANK_AWARD);
    if (mo == NULL) return;

    // replace
    int size = 4;
    ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_NUMBER, rank);
    ::strncpy(mail_module::replace_str[1], MAIL_P_NUMBER, sizeof(mail_module::replace_str[1]) - 1);
    ::snprintf(mail_module::replace_value[1],
               sizeof(mail_module::replace_value[1]),
               "%s(%d)",
               STRING_NUMBER, bind_diamond);
    ::strncpy(mail_module::replace_str[2], MAIL_P_NUMBER, sizeof(mail_module::replace_str[2]) - 1);
    ::snprintf(mail_module::replace_value[2],
               sizeof(mail_module::replace_value[2]),
               "%s(%d)",
               STRING_NUMBER, coin);
    ::strncpy(mail_module::replace_str[3], MAIL_P_NUMBER, sizeof(mail_module::replace_str[3]) - 1);
    ::snprintf(mail_module::replace_value[3],
               sizeof(mail_module::replace_value[3]),
               "%s(%d)",
               STRING_NUMBER, ji_fen);
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   size,
                                   content);
    mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                              mo->sender_name_, mo->title_, content,
                              mail_info::MAIL_TYPE_JING_JI,
                              coin, 0, bind_diamond,
                              0, NULL,
                              player->db_sid(), time_util::now);

    mail_module::do_notify_haved_new_mail(player, 1);
  }

  player->char_extra_info_->jing_ji_score_ += ji_fen;
  player->db_save_char_extra_info();
  player->do_notify_jing_ji_score_to_clt();
}
void jing_ji_module::insert_jing_ji_log_2_db(player_obj *player,
                                             const int competitor_id,
                                             const int rank_change)
{
  jing_ji_log jjl;
  jjl.char_id_       = player->id();
  jjl.competitor_id_ = competitor_id;
  jjl.rank_change_   = rank_change;
  jjl.time_          = time_util::now;

  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << stream_ostr((char *)&jjl, sizeof(jing_ji_log));
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_JING_JI_LOG, &db_os);
}
void jing_ji_module::db_update_rank(const int db_sid,
                                    const int char_id,
                                    const int rank)
{
  jing_ji_rank *jjr = NULL;
  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(char_id);
  if (itor == s_char_rank_map.end())
  {
    jjr = new jing_ji_rank();
    s_char_rank_map.insert(std::make_pair(char_id, jjr));
  }else
    jjr = itor->second;

  jjr->char_id_ = char_id;
  jjr->rank_    = rank;

  s_rank_char_map[rank] = char_id;

  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << db_sid << stream_ostr((char *)jjr, sizeof(jing_ji_rank));
  db_proxy::instance()->send_request(char_id, REQ_UPDATE_JING_JI_RANK, &db_os);
}
void jing_ji_module::do_timeout(const int now)
{
  static int s_last_check_award_time = 0;
  if (now - s_last_check_award_time < 60) return ;
  s_last_check_award_time = now;

  date_time dt(now);
  if (dt.hour() != 21 && jing_ji_module::today_send_award)
    jing_ji_module::today_send_award = false;

  if (dt.hour() != 21 || jing_ji_module::today_send_award) return;

  jing_ji_module::today_send_award = true;

  sys::r->schedule_timer(new jing_ji_award_timer(), time_value(0, 0), time_value(1, 0));
}
void jing_ji_module::do_send_jing_ji_award(const int char_id, const int rank)
{
  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(char_id);
  if (itor == s_char_rank_map.end()) return;

  jing_ji_rank *jjr = itor->second;
  if (time_util::diff_days(jjr->award_time_) == 0) return;

  //
  {
    jjr->award_time_ = time_util::now;
    out_stream db_os(client::send_buf, client::send_buf_len);
    db_os << 0 << stream_ostr((char *)jjr, sizeof(jing_ji_rank));
    db_proxy::instance()->send_request(char_id, REQ_UPDATE_JING_JI_RANK, &db_os);
  }

  jj_award_obj *jao = jing_ji_award_cfg::instance()->get_jj_award_obj(rank);
  if (jao == NULL) return ;

  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::MAIL_JJ_RANK_AWARD);
  if (mo == NULL) return ;

  // replace
  int size = 4;
  ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
  ::snprintf(mail_module::replace_value[0],
             sizeof(mail_module::replace_value[0]),
             "%s(%d)",
             STRING_NUMBER, rank);
  ::strncpy(mail_module::replace_str[1], MAIL_P_NUMBER, sizeof(mail_module::replace_str[1]) - 1);
  ::snprintf(mail_module::replace_value[1],
             sizeof(mail_module::replace_value[1]),
             "%s(%d)",
             STRING_NUMBER, jao->bind_diamond_);
  ::strncpy(mail_module::replace_str[2], MAIL_P_NUMBER, sizeof(mail_module::replace_str[2]) - 1);
  ::snprintf(mail_module::replace_value[2],
             sizeof(mail_module::replace_value[2]),
             "%s(%d)",
             STRING_NUMBER, jao->coin_);
  ::strncpy(mail_module::replace_str[3], MAIL_P_NUMBER, sizeof(mail_module::replace_str[3]) - 1);
  ::snprintf(mail_module::replace_value[3],
             sizeof(mail_module::replace_value[3]),
             "%s(%d)",
             STRING_NUMBER, jao->ji_fen_);
  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);

  mail_module::do_send_mail(char_id, mail_info::MAIL_SEND_SYSTEM_ID,
                            mo->sender_name_, mo->title_, content,
                            mail_info::MAIL_TYPE_JING_JI,
                            jao->coin_, 0, jao->bind_diamond_,
                            0, NULL,
                            char_id, time_util::now);

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player != NULL)
  {
    mail_module::do_notify_haved_new_mail(player, 1);
    player->char_extra_info_->jing_ji_score_ += jao->ji_fen_;
    player->db_save_char_extra_info();
    player->do_notify_jing_ji_score_to_clt();
  }else
  {
    char_db_msg_queue cdmq;
    cdmq.char_id_ = char_id;
    cdmq.msg_id_  = CMQ_JING_JI_AWARD;
    ::snprintf(cdmq.param_, sizeof(cdmq.param_),
               "%d",
               jao->ji_fen_);
    out_stream os(client::send_buf, client::send_buf_len);
    os << 0 << stream_ostr((char *)&cdmq, sizeof(char_db_msg_queue));
    db_proxy::instance()->send_request(char_id, REQ_INSERT_CHAR_DB_MSG, &os);
  }
}
void player_obj::do_notify_jing_ji_score_to_clt()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->char_extra_info_->jing_ji_score_;
  this->send_request(NTF_UPDATE_JING_JI_SCORE, &os);
}
void jing_ji_module::do_build_n_rank_info(out_stream &os)
{
  short *count = (short*)os.wr_ptr();
  os << short(0);
  for (int i = 1; i <= global_param_cfg::rank_amount; ++i)
  {
    jing_ji_rank_char_map_itor itor = s_rank_char_map.find(i);
    if (itor == s_rank_char_map.end()) continue;
    int char_id = itor->second;

    char_brief_info *cbi =
      all_char_info::instance()->get_char_brief_info(char_id);
    if (cbi == NULL)
      break;
    else
    {
      os << char_id << cbi->vip_ << stream_ostr(cbi->name_, ::strlen(cbi->name_)) << cbi->career_;
      int guild_id = guild_module::get_guild_id(char_id);
      char *guild_name = guild_module::get_guild_name(guild_id);
      if (guild_name == NULL)
        os << stream_ostr("", 0);
      else
        os << stream_ostr(guild_name, ::strlen(guild_name));
      os << cbi->zhan_li_;
    }
    ++(*count);
  }
}
void jing_ji_module::get_char_list_before_rank(ilist<int> &char_list, const int n)
{
  for (int i = 1; i <= global_param_cfg::rank_amount && i <= n; ++i)
  {
    jing_ji_rank_char_map_itor itor = s_rank_char_map.find(i);
    if (itor != s_rank_char_map.end())
      char_list.push_back(itor->second);
  }
}
void jing_ji_module::do_notify_can_jing_ji(player_obj *player)
{
  if (player->notify_jing_ji_
      || (jing_ji_module::can_jing_ji(player) != 0))
    return;

  player->send_request(NTF_NOTIFY_CAN_JING_JI, NULL);
  player->notify_jing_ji_ = true;
}
int jing_ji_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = jing_ji_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_JING_JI_INFO,         clt_obtain_jing_ji_info);
    SHORT_CODE(REQ_JING_JI_REFRESH_COMPETITOR,  clt_refresh_competitor);
    SHORT_CODE(REQ_JING_JI_CHALLENGE,           clt_jing_ji_challenge);
    SHORT_CODE(REQ_JING_JI_CLEAN_TIME,          clt_jing_ji_clean_time);
    SHORT_CODE(REQ_GET_JING_JI_LOG_LIST,        clt_get_jing_ji_log_list);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int jing_ji_module::clt_obtain_jing_ji_info(player_obj *player, const char *, const int )
{
  if (!global_param_cfg::jing_ji_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_JI_CLOSED);

  if (player->lvl() < global_param_cfg::jing_ji_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(player->id());
  if (itor == s_char_rank_map.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  int remaind_time = (global_param_cfg::jing_ji_cd) -
    (time_util::now - player->char_extra_info_->last_jing_ji_time_);
  if (remaind_time < 0) remaind_time = 0;

  if (!player->notify_jing_ji_
      && (jing_ji_module::can_jing_ji(player) == 0))
    player->notify_jing_ji_ = true;

  out_stream os(client::send_buf, client::send_buf_len);
  os << itor->second->rank_
    << remaind_time
    << player->char_extra_info_->jing_ji_score_
    << player->jing_ji_cnt();
  return player->send_request(RES_OBTAIN_JING_JI_INFO, &os);
}
int jing_ji_module::clt_refresh_competitor(player_obj *player, const char *, const int )
{
  if (!global_param_cfg::jing_ji_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_JI_CLOSED);

  if (player->lvl() < global_param_cfg::jing_ji_lvl)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  int my_rank = 0;
  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(player->id());
  if (itor != s_char_rank_map.end())
    my_rank = itor->second->rank_;

  if (my_rank == 0)
  {
    e_log->rinfo("char %d rank is 0 when %s", player->id(), __func__);
    jing_ji_module::db_update_rank(player->db_sid(), player->id(), ++s_max_rank);
    my_rank = s_max_rank;
  }else if (my_rank == 1)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_JI_NO_COMPETITOR);

  int result[REFRESH_COMPETITOR_COUNT] = {0, 0, 0, 0};
  jing_ji_module::get_refresh_competitor_result(my_rank, result);

  out_stream os(client::send_buf, client::send_buf_len);
  char *n = os.wr_ptr();
  os << (char)0;

  for (size_t i = 0; i < sizeof(result)/sizeof(result[0]); ++i)
  {
    if (build_one_jing_ji_player_info(result[i], os) == 0)
      (*n)++;
  }

  return player->send_respond_ok(RES_JING_JI_REFRESH_COMPETITOR, &os);
}
void jing_ji_module::get_refresh_competitor_result(const int my_rank,
                                                   int refresh_rank_result[])
{
  if (my_rank == 2)
  {
    refresh_rank_result[0] = 1;
  }else if (my_rank == 3)
  {
    refresh_rank_result[0] = 1;
    refresh_rank_result[1] = 2;
  }else if (my_rank == 4)
  {
    refresh_rank_result[0] = 1;
    refresh_rank_result[1] = 2;
    refresh_rank_result[2] = 3;
  }else if (my_rank == 5)
  {
    refresh_rank_result[0] = 1;
    refresh_rank_result[1] = 2;
    refresh_rank_result[2] = 3;
    refresh_rank_result[3] = 4;
  }else if (my_rank > 5 && my_rank <= 10)
  {
    // 随机取前面人中的4个人作为挑战目标
    for (size_t i = 0; i < REFRESH_COMPETITOR_COUNT;)
    {
      int rank = rand() % (my_rank - 1) + 1;
      size_t j = 0;
      for (; j < REFRESH_COMPETITOR_COUNT; ++j)
      {
        if (refresh_rank_result[j] == rank) // repead
          break;
      }
      if (j == REFRESH_COMPETITOR_COUNT)
      {
        refresh_rank_result[i] = rank;
        ++i;
      }
    }
  }else
  {
    // 系数以后看是否配置在全局表中
    int rank_range_idx = my_rank / 10 * 10;
    int r_range_1 = 0;
    int r_range_2 = rank_range_idx * global_param_cfg::jing_ji_prm[0] / 10;
    int r_range_3 = rank_range_idx * global_param_cfg::jing_ji_prm[1] / 10;
    int r_range_4 = rank_range_idx * global_param_cfg::jing_ji_prm[2] / 10;

    refresh_rank_result[0] = rand() % (r_range_2 - r_range_1) + 1 + r_range_1;
    refresh_rank_result[1] = rand() % (r_range_3 - r_range_2) + 1 + r_range_2;
    refresh_rank_result[2] = rand() % (r_range_4 - r_range_3) + 1 + r_range_3;
    refresh_rank_result[3] = rand() % (my_rank - 1 - r_range_4) + 1 + r_range_4;
  }
}
int jing_ji_module::clt_jing_ji_challenge(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::jing_ji_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_JI_CLOSED);

  int rank = 0;
  int competitor_id = 0;
  in_stream is(msg, len);
  is >> rank >> competitor_id;

  int ret = jing_ji_module::can_jing_ji(player, rank, competitor_id);
  if (ret < 0)
    return player->send_respond_err(RES_JING_JI_CHALLENGE, ret);

  ret = scp_module::do_enter_single_scp(player, global_param_cfg::jing_ji_map);
  if (ret < 0)
    return player->send_respond_err(RES_JING_JI_CHALLENGE, ret);

  ret = jing_ji_module::create_competitor(player, competitor_id, global_param_cfg::jing_ji_mst);
  if (ret < 0)
    return player->send_respond_err(RES_JING_JI_CHALLENGE, ret);

  if (player->do_add_hp(player->total_hp()) != 0
      && player->do_add_mp(player->total_mp()) != 0)
    player->broadcast_hp_mp();

  jing_ji_match_map_itor itor = s_jing_ji_match_map.find(player->id());
  if (itor != s_jing_ji_match_map.end())
  {
    itor->second->competitor_id_ = competitor_id;
    itor->second->player_copy_mst_id_ = ret;
  }else
  {
    jing_ji_match_info *info = new jing_ji_match_info();
    info->competitor_id_ = competitor_id;
    info->player_copy_mst_id_ = ret;
    s_jing_ji_match_map.insert(std::make_pair(player->id(), info));
  }

  player->jing_ji_cnt(player->jing_ji_cnt() + 1);
  player->db_save_daily_clean_info();

  if (player->jing_ji_cnt() >= global_param_cfg::jing_ji_no_cd_times)
  {
    player->char_extra_info_->last_jing_ji_time_ = time_util::now;
    player->db_save_char_extra_info();
  }
  player->notify_jing_ji_ = false;

  s_on_jing_ji_char_set.insert(player->id());
  s_on_jing_ji_char_set.insert(competitor_id);

  huo_yue_du_module::on_jing_ji(player);
  cheng_jiu_module::on_jing_ji(player);
  task_module::on_jing_ji(player);

  return player->send_respond_ok(RES_JING_JI_CHALLENGE);
}
int jing_ji_module::can_jing_ji(player_obj *player, const int rank, const int competitor_id)
{
  jing_ji_char_rank_map_itor itor = s_char_rank_map.find(competitor_id);
  if (itor == s_char_rank_map.end()
      || itor->second->rank_ != rank)
    return ERR_COMPETITOR_RANK_CHANGE;

  int ret = jing_ji_module::can_jing_ji(player);
  if (ret != 0) return ret;

  if (s_on_jing_ji_char_set.find(player->id()) != s_on_jing_ji_char_set.end()
      || s_on_jing_ji_char_set.find(competitor_id) != s_on_jing_ji_char_set.end())
    return ERR_COMPETITOR_ON_JING_JI;

  if (!clsid::is_world_scene(player->scene_cid()))
    return ERR_CAN_NOT_JING_JI_IN_THIS_SCENE;

  ret = scp_module::can_enter_scp(player, global_param_cfg::jing_ji_map);
  if (ret != 0) return ret;

  return 0;
}
int jing_ji_module::can_jing_ji(player_obj *player)
{
  if (player->lvl() < global_param_cfg::jing_ji_lvl)
    return ERR_LVL_NOT_ENOUGH;
  int vip_cnt = vip_module::to_get_day_jing_ji_cnt(player);
  if (player->jing_ji_cnt() >= global_param_cfg::jing_ji_times + vip_cnt)
    return ERR_OVER_JING_JI_COUNT; 

  if (time_util::now - player->char_extra_info_->last_jing_ji_time_
      < global_param_cfg::jing_ji_cd)
    return ERR_JING_JI_IN_CD;

  return 0;
}
int jing_ji_module::create_competitor(player_obj *player,
                                      const int competitor_id,
                                      coord_t &enter_coord)
{
  // spawn monster
  player_copy_mst *p = new player_copy_mst();
  // 坐标暂时没定配在哪里
  if (p->init(competitor_id,
              player->scene_id(),
              player->scene_cid(),
              DIR_XX,
              enter_coord.x_,
              enter_coord.y_) != 0
      || p->do_activate(1000) != 0)
  {
    delete p;
    return ERR_UNKNOWN;
  }
  p->master_id(competitor_id);

  player_obj *competitor = player_mgr::instance()->find(competitor_id);
  if (competitor != NULL)
  {
    for (skill_map_itor itor = competitor->skill_map_.begin();
         itor != competitor->skill_map_.end();
         ++itor)
    {
      mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
      *mb << itor->second->cid_ << itor->second->lvl_;
      p->post_aev(AEV_PUSH_SKILL, mb);
    }
  }else
  {
    out_stream db_os(client::send_buf, client::send_buf_len);
    db_os << player->db_sid() << competitor_id << p->id();
    db_proxy::instance()->send_request(competitor_id, REQ_GET_OTHER_SKILL_LIST, &db_os);
  }

  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_JING_JI_MST);
  mb << p->id();
  scp_mgr::instance()->do_something(player->scene_id(), &mb, NULL, NULL);

  return p->id();
}
int jing_ji_module::clt_jing_ji_clean_time(player_obj *player, const char *, const int )
{
  int vip_cnt = vip_module::to_get_day_jing_ji_cnt(player);
  if (time_util::now - player->char_extra_info_->last_jing_ji_time_
      >= global_param_cfg::jing_ji_cd
      || player->jing_ji_cnt() >= global_param_cfg::jing_ji_times + vip_cnt)
    return player->send_respond_err(RES_JING_JI_CLEAN_TIME, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND, global_param_cfg::jing_ji_reset_cost);
  if (ret < 0)
    return player->send_respond_err(RES_JING_JI_CLEAN_TIME, ret);

  player->do_lose_money(global_param_cfg::jing_ji_reset_cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_JING_JI_CLEAN_TIME,
                        0, 0, 0);

  player->char_extra_info_->last_jing_ji_time_ = 0;
  player->db_save_char_extra_info();
  player->notify_jing_ji_ = false;

  return player->send_respond_ok(RES_JING_JI_CLEAN_TIME);
}
int jing_ji_module::clt_get_jing_ji_log_list(player_obj *player, const char *, const int )
{
  if (!global_param_cfg::jing_ji_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_JI_CLOSED);

  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_GET_JING_JI_LOG, &db_os);

  return 0;
}
