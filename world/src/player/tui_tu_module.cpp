#include "tui_tu_module.h"
#include "tui_tu_config.h"
#include "behavior_log.h"
#include "global_param_cfg.h"
#include "vip_module.h"
#include "monster_cfg.h"
#include "package_module.h"
#include "all_char_info.h"
#include "dropped_item.h"
#include "dropped_item_mgr.h"
#include "time_util.h"
#include "tui_tu_log.h"
#include "db_proxy.h"
#include "player_obj.h"
#include "scp_obj.h"
#include "scp_mgr.h"
#include "task_module.h"
#include "scp_module.h"
#include "cheng_jiu_module.h"
#include "mail_config.h"
#include "sys_log.h"
#include "clsid.h"
#include "util.h"
#include "rank_module.h"
#include "kai_fu_act_obj.h"
#include "ltime_act_module.h"
#include "lvl_param_cfg.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("tui_tu");
static ilog_obj *e_log = err_log::instance()->get_ilog("tui_tu");

enum
{
  TUI_TU_COMMON    = 1,  // 普通
  TUI_TU_JING_YING = 2,  // 精英
};
class tui_tu_turn_item_info
{
public:
  tui_tu_turn_item_info() : rate_(0) { }
  int rate_;
  item_amount_bind_t item_;
};
class tui_tu_top_score_char
{
public:
  tui_tu_top_score_char(const int char_id, const int used_time) :
    char_id_(char_id),
    used_time_(used_time)
  { }
  int char_id_;
  int used_time_;
};
typedef std::map<int/*scene_cid*/, tui_tu_top_score_char> tui_tu_top_score_char_map_t;
typedef std::map<int/*scene_cid*/, tui_tu_top_score_char>::iterator tui_tu_top_score_char_map_itor;
static tui_tu_top_score_char_map_t s_tui_tu_top_score_char_map;

void tui_tu_module::on_char_login(player_obj *player)
{
  if (player->char_info_->in_time_ == 0) // for new char
  {
    int scene_cid = global_param_cfg::birth_mission_cid;
    const tui_tu_cfg_obj *ttco = tui_tu_config::instance()->get_tui_tu_cfg_obj(scene_cid);
    if (ttco == NULL) return ;
    scp_obj *so = scp_module::construct_scp(scene_cid, player->id(), 0, 0);
    if (so == NULL || so->open() != 0)
    {
      e_log->rinfo("char %d enter first mission failed!", player->id());
      delete so;
      return ;
    }
    player->scene_cid_  = scene_cid;
    player->scene_id_   = so->first_scene_id();
    player->coord_x_    = ttco->enter_x_;
    player->coord_y_    = ttco->enter_y_;
    so->enter_scp(player->id(), player->scene_id_);

    tui_tu_log *new_info = tui_tu_log_pool::instance()->alloc();
    new_info->state_ = tui_tu_log::ST_FINISHED;
    new_info->char_id_ = player->id();
    new_info->cid_  = scene_cid;
    new_info->begin_time_ = time_util::now;
    player->tui_tu_log_.push_back(new_info);
  }else
  {
    if (!clsid::is_tui_tu_scp_scene(player->scene_cid())) return ;
    player->do_resume_last_position();
  }
}
void tui_tu_module::on_char_logout(player_obj *player)
{
  if (clsid::is_tui_tu_scp_scene(player->scene_cid()))
    scp_mgr::instance()->exit_scp(player->scene_id(), player->id(), scp_obj::CHAR_INITIATIVE_EXIT);
}
void tui_tu_module::on_transfer_scene(player_obj *player,
                                      const int from_scene_id,
                                      const int from_scene_cid,
                                      const int /*to_scene_id*/,
                                      const int to_scene_cid)
{
  if (from_scene_cid == to_scene_cid) return ;
  if (!clsid::is_tui_tu_scp_scene(from_scene_cid)) return ;
  if (!clsid::is_tui_tu_scp_scene(to_scene_cid))
    scp_mgr::instance()->exit_scp(from_scene_id, player->id(), scp_obj::CHAR_INITIATIVE_EXIT);
}
int tui_tu_module::handle_db_get_tui_tu_log_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(tui_tu_log) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    tui_tu_log *info = (tui_tu_log *)bf;

    if (tui_tu_config::instance()->get_tui_tu_cfg_obj(info->cid_) == NULL)
      continue;
    tui_tu_log *new_info = tui_tu_log_pool::instance()->alloc();
    ::memcpy(new_info, info, sizeof(tui_tu_log));
    player->tui_tu_log_.push_back(new_info);
  }
  return 0;
}
void tui_tu_module::destroy(player_obj *player)
{
  while (!player->tui_tu_log_.empty())
    tui_tu_log_pool::instance()->release(player->tui_tu_log_.pop_front());
  while (!player->tui_tu_dropped_item_list_.empty())
    dropped_item_pool::instance()->release(player->tui_tu_dropped_item_list_.pop_front());
  while (!player->tui_tu_turn_item_list_.empty())
    delete player->tui_tu_turn_item_list_.pop_front();
}
void tui_tu_module::on_load_one_log(const tui_tu_log *l)
{
  if (l->state_ != tui_tu_log::ST_FINISHED) return ;
  tui_tu_top_score_char_map_itor itor = s_tui_tu_top_score_char_map.find(l->cid_);
  if (itor == s_tui_tu_top_score_char_map.end())
  {
    tui_tu_top_score_char v(l->char_id_, l->used_time_);
    s_tui_tu_top_score_char_map.insert(std::make_pair(l->cid_, v));
  }else
  {
    if (itor->second.used_time_ == 0
        || itor->second.used_time_ > l->used_time_)
    {
      itor->second.char_id_ = l->char_id_;
      itor->second.used_time_ = l->used_time_;
    }
  }
}
char_brief_info *tui_tu_module::get_score_top_char(const int scene_cid, int &top_score)
{
  tui_tu_top_score_char_map_itor itor = s_tui_tu_top_score_char_map.find(scene_cid);
  if (itor == s_tui_tu_top_score_char_map.end()) return NULL;
  top_score = itor->second.used_time_;
  return all_char_info::instance()->get_char_brief_info(itor->second.char_id_);
}
void tui_tu_module::db_insert_tui_tu_log(player_obj *player, tui_tu_log *ttl)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr((const char *)ttl, sizeof(tui_tu_log));
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_TUI_TU_LOG, &os);
}
void tui_tu_module::db_update_tui_tu_log(player_obj *player, tui_tu_log *ttl)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr((const char *)ttl, sizeof(tui_tu_log));
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_TUI_TU_LOG, &os);
}
void tui_tu_module::do_clean_resource(player_obj *player)
{
  while (!player->tui_tu_turn_item_list_.empty())
    delete player->tui_tu_turn_item_list_.pop_front();
  while (!player->tui_tu_dropped_item_list_.empty())
    dropped_item_pool::instance()->release(player->tui_tu_dropped_item_list_.pop_front());
}
int tui_tu_module::get_chapter_idx(const int type, const int cid)
{
  if (type == TUI_TU_COMMON)
    return tui_tu_chapter_config::instance()->get_chapter_idx(cid);
  return tui_tu_jy_chapter_config::instance()->get_chapter_idx(cid);
}
tui_tu_chapter_cfg_obj *tui_tu_module::get_chapter_cfg(const int type, const int idx)
{
  if (type == TUI_TU_COMMON)
    return tui_tu_chapter_config::instance()->get_tui_tu_chapter_cfg_obj(idx);
  return tui_tu_jy_chapter_config::instance()->get_tui_tu_chapter_cfg_obj(idx);
}
int tui_tu_module::get_valid_max_chapter(player_obj *player, const int type)
{
  // 先找当前已经做过的推图
  int cur_max_chapter_idx = -1;
  for (ilist_node<tui_tu_log *> *itor = player->tui_tu_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    int idx = tui_tu_module::get_chapter_idx(type, itor->value_->cid_);
    if (idx != -1 && idx > cur_max_chapter_idx)
      cur_max_chapter_idx = idx;
  }
  if (cur_max_chapter_idx == -1)
    return 1;
  
  // 再看当前章节是否已经做完
  tui_tu_chapter_cfg_obj *chapter_cfg = tui_tu_module::get_chapter_cfg(type, cur_max_chapter_idx);
  if (chapter_cfg == NULL) return -1;

  for (ilist_node<int> *itor = chapter_cfg->scene_cid_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    ilist_node<tui_tu_log *> *litor = player->tui_tu_log_.head();
    for (; litor != NULL; litor = litor->next_)
    {
      if (litor->value_->cid_ == itor->value_
          && litor->value_->state_ == tui_tu_log::ST_FINISHED)
        break;
    }
    if (litor == NULL)  // 如果当前章节还有未完成的
      return cur_max_chapter_idx;
  }

  if (tui_tu_module::get_chapter_cfg(type, cur_max_chapter_idx + 1) != NULL)
    return cur_max_chapter_idx + 1;
  return cur_max_chapter_idx;
}
tui_tu_log *tui_tu_module::get_tui_tu_log(player_obj *player,
                                          const int scene_cid)
{
  for (ilist_node<tui_tu_log *> *itor = player->tui_tu_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == scene_cid)
      return itor->value_;
  }
  return NULL;
}
int tui_tu_module::get_next_scene_cid(tui_tu_chapter_cfg_obj *chapter_cfg,
                                      ilist<int> &cur_chapter_scene_list)
{
  int to_open_cid = 0;
  if (cur_chapter_scene_list.size()
      != chapter_cfg->scene_cid_list_.size())
  {
    if (cur_chapter_scene_list.empty())
      to_open_cid = chapter_cfg->scene_cid_list_.head()->value_;
    else
    {
      for (ilist_node<int> *itor = cur_chapter_scene_list.head();
           itor != NULL;
           itor = itor->next_)
      {
        tui_tu_cfg_obj *ttco = tui_tu_config::instance()->get_tui_tu_cfg_obj(itor->value_);
        if (ttco == NULL
            || ttco->next_scene_cid_ == 0)
          continue;
        if (!cur_chapter_scene_list.find(ttco->next_scene_cid_))
        {
          to_open_cid = ttco->next_scene_cid_;
          break;
        }
      }
    }
  }
  return to_open_cid;
}
void tui_tu_module::do_build_tui_tu_show_info(tui_tu_log *l,
                                              const int scene_cid,
                                              out_stream &os)
{
  if (l != NULL)
    os << l->cid_ << l->used_time_ << l->first_award_ << l->state_;
  else
    os << scene_cid << (int)0 << (char)0 << (char)tui_tu_log::ST_NO_OPENED;

  int top_used_time = 0;
  char_brief_info *cbi = tui_tu_module::get_score_top_char(scene_cid, top_used_time);
  if (cbi != NULL)
  {
    os << cbi->char_id_
      << stream_ostr(cbi->name_, ::strlen(cbi->name_))
      << cbi->career_
      << top_used_time;
  }else
    os << 0 << stream_ostr("", 0) << (char)0 << 0;
}
// -------------------------------------------------------------------------------------
void player_obj::do_exit_tui_tu_scp()
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
  {
    this->del_status(OBJ_DEAD);
    int relive_energy_per = global_param_cfg::relive_energy_p;
    this->do_add_hp(this->obj_attr_.total_hp() * relive_energy_per / 100);
    this->broadcast_hp_mp();
  }
  this->do_transfer_to(this->char_info_->last_scene_cid_,
                       this->char_info_->last_scene_cid_,
                       this->char_info_->last_coord_x_,
                       this->char_info_->last_coord_y_);
}
void player_obj::on_exit_tui_tu_scp()
{
  tui_tu_module::do_clean_resource(this);
}
int player_obj::do_enter_tui_tu_scp(const int scene_cid, const tui_tu_cfg_obj *ttco)
{
  scp_obj *so = scp_module::construct_scp(scene_cid, this->id(), 0, 0);
  if (so == NULL || so->open() != 0)
  {
    delete so;
    return ERR_TUI_TU_SCP_CREATE_FAILED;
  }

  int ret = this->do_transfer_to(scene_cid,
                                 so->first_scene_id(),
                                 ttco->enter_x_,
                                 ttco->enter_y_);
  if (ret != 0) return ret;
  so->enter_scp(this->id_, this->scene_id_);
  
  tui_tu_module::do_clean_resource(this);

  tui_tu_log *new_info = NULL;
  for (ilist_node<tui_tu_log *> *itor = this->tui_tu_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == scene_cid)
      new_info = itor->value_;
  }
  bool new_one = false;
  if (new_info == NULL)
  {
    new_info = tui_tu_log_pool::instance()->alloc();
    new_info->state_ = tui_tu_log::ST_INPROGRESS;
    new_info->char_id_ = this->id();
    new_info->cid_  = scene_cid;
    new_one  = true;
    this->tui_tu_log_.push_back(new_info);
  }
  new_info->begin_time_ = time_util::now;
  new_info->free_turn_cnt_ = 0;
  new_info->is_perfect_ = false;

  if (new_one)
    tui_tu_module::db_insert_tui_tu_log(this, new_info);

  behavior_log::instance()->store(BL_ENTER_SCP,
                                  time_util::now,
                                  "%d|%d",
                                  this->id_, scene_cid);
  return 0;
}
int tui_tu_module::dispatch_msg(player_obj *player,
                                const int msg_id,
                                const char *msg,
                                const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = tui_tu_module::FUNC(player, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch(msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_TUI_TU_CHAPTER_INFO,    clt_obtain_tui_tu_chapter_info);
    SHORT_CODE(REQ_OBTAIN_TUI_TU_INFO,            clt_obtain_tui_tu_info);
    SHORT_CODE(REQ_ENTER_TUI_TU,                  clt_enter);
    SHORT_CODE(REQ_IN_TUI_TU_KILL_MONSTER,        clt_in_tui_tu_kill_mst);
    SHORT_CODE(REQ_IN_TUI_TU_RELIVE,              clt_in_tui_tu_relive);
    SHORT_CODE(REQ_TUI_TU_END,                    clt_tui_tu_end);
    SHORT_CODE(REQ_EXIT_TUI_TU,                   clt_exit);
    SHORT_CODE(REQ_TUI_TU_DO_TURN,                clt_tui_tu_do_turn);
    SHORT_CODE(REQ_TUI_TU_SAO_DANG,               clt_tui_tu_sao_dang);
    SHORT_DEFAULT;
  }
  return ret;
}
int tui_tu_module::clt_obtain_tui_tu_chapter_info(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  char type = 1;  // 1: 普通 2: 精英
  int chapter_idx = 0;
  is >> type >> chapter_idx;
  if (type != TUI_TU_COMMON
      && type != TUI_TU_JING_YING)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (chapter_idx == -1) // 当玩家第一次打开推图的界面时，不知道自己当前的章节，发-1过来
    chapter_idx = tui_tu_module::get_valid_max_chapter(player, type);

  tui_tu_chapter_cfg_obj *chapter_cfg = tui_tu_module::get_chapter_cfg(type, chapter_idx);
  if (chapter_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  out_stream os(client::send_buf, client::send_buf_len);
  os << type << chapter_idx;
  short *count = (short *)os.wr_ptr();
  os << (short)0;

  static ilist<int> cur_chapter_scene_list;
  cur_chapter_scene_list.clear();

  for (ilist_node<tui_tu_log *> *itor = player->tui_tu_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    tui_tu_log *l = itor->value_;
    if (!chapter_cfg->scene_cid_list_.find(l->cid_))
      continue;

    cur_chapter_scene_list.push_back(l->cid_);
    tui_tu_module::do_build_tui_tu_show_info(l, l->cid_, os);

    ++(*count);
  }

  // 查找下个可进的关卡
  int to_open_cid = tui_tu_module::get_next_scene_cid(chapter_cfg,
                                                      cur_chapter_scene_list);
  if (to_open_cid != 0)
  {
    tui_tu_module::do_build_tui_tu_show_info(NULL, to_open_cid, os);
    ++(*count);
  }

  return player->send_respond_ok(RES_OBTAIN_TUI_TU_CHAPTER_INFO, &os);
}
int tui_tu_module::clt_obtain_tui_tu_info(player_obj *player, const char *msg, const int len)
{
  int scene_cid = 0;
  in_stream is(msg, len);
  is >> scene_cid;

  out_stream os(client::send_buf, client::send_buf_len);
  ilist_node<tui_tu_log *> *itor = player->tui_tu_log_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_->cid_ == scene_cid)
      break;
  }
  tui_tu_module::do_build_tui_tu_show_info(itor == NULL ? NULL : itor->value_,
                                           scene_cid,
                                           os);
  return player->send_respond_ok(RES_OBTAIN_TUI_TU_INFO, &os);
}
int tui_tu_module::clt_enter(player_obj *player, const char *msg, const int len)
{
  if (player->scene_id() != player->scene_cid()) // in scp
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  in_stream is(msg, len);
  int scene_cid = 0;
  is >> scene_cid;

  const tui_tu_cfg_obj *tui_tu_cfg = tui_tu_config::instance()->get_tui_tu_cfg_obj(scene_cid);
  if (tui_tu_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (player->char_extra_info_->ti_li_ < tui_tu_cfg->tili_cost_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TI_LI_NOT_ENOUGH);

  int ret = player->can_transfer_to(scene_cid);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = player->do_enter_tui_tu_scp(scene_cid, tui_tu_cfg);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int tui_tu_module::clt_in_tui_tu_kill_mst(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_tui_tu_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int mst_cid = 0;
  short x = 0;
  short y = 0;
  in_stream is(msg, len);
  is >> mst_cid >> x >> y;

  bool got_award = false;
  player->on_clt_kill_mst(mst_cid, x, y, NTF_IN_TUI_TU_KILL_MST_AWARD, got_award);
  return 0;
}
int tui_tu_module::clt_in_tui_tu_relive(player_obj *player, const char *msg, const int len)
{
  if (!clsid::is_tui_tu_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  char relive_mode = 0;
  in_stream is(msg, len);
  is >> relive_mode;

  if (relive_mode != 2)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  int ret = player->do_relive(relive_mode);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int tui_tu_module::clt_exit(player_obj *player, const char *, const int )
{
  if (!clsid::is_tui_tu_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player->do_exit_tui_tu_scp();
  return 0;
}
int tui_tu_module::clt_tui_tu_end(player_obj *player, const char *msg, const int len)
{
  char lian_ji_break = 1;
  in_stream is(msg, len);
  is >> lian_ji_break;

  if (!clsid::is_tui_tu_scp_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  tui_tu_log *ttl = tui_tu_module::get_tui_tu_log(player, player->scene_cid());
  if (ttl == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_NOT_EXIST);

  const tui_tu_cfg_obj *tui_tu_cfg = tui_tu_config::instance()->get_tui_tu_cfg_obj(player->scene_cid());
  if (tui_tu_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (lian_ji_break == 0)
    ttl->is_perfect_ = true;

  // cost ti li
  player->do_reduce_ti_li(tui_tu_config::instance()->ti_li_cost(player->scene_cid()));

  int ret = tui_tu_module::do_finish_tui_tu(player, ttl, tui_tu_cfg, false);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int tui_tu_module::do_finish_tui_tu(player_obj *player,
                                    tui_tu_log *ttl,
                                    const tui_tu_cfg_obj *tui_tu_cfg,
                                    const bool sao_dang)
{
  bool to_update_db = false;
  if (ttl->state_ != tui_tu_log::ST_FINISHED)
  {
    ttl->state_ = tui_tu_log::ST_FINISHED;
    to_update_db = true;
  }
  int cur_used_time = 0;
  bool star_change = false;
  if (!sao_dang)
  {
    cur_used_time = (time_util::now - ttl->begin_time_) * 1000 + rand() % 1000;
    if (cur_used_time <= 0) cur_used_time = 5000;
    if (ttl->used_time_ == 0
        || ttl->used_time_ > cur_used_time)
    {
      to_update_db = true;
      ttl->used_time_ = cur_used_time;
      tui_tu_module::on_load_one_log(ttl);
      star_change = true;
    }
  }else
  {
    cur_used_time = ttl->used_time_;
    tui_tu_module::do_notify_boss_drop_items(player, tui_tu_cfg->boss_cid_);
  }

  int award_exp = 0;
  if (ttl->first_award_ == 0)
  {
    package_module::do_insert_or_mail_item(player,
                                           tui_tu_cfg->first_item_award_.cid_,
                                           tui_tu_cfg->first_item_award_.amount_,
                                           tui_tu_cfg->first_item_award_.bind_,
                                           mail_config::PACKAGE_IS_FULL,
                                           MONEY_GOT_TUI_TU_FIRST_AWARD,
                                           ITEM_GOT_TUI_TU_FIRST_AWARD,
                                           ttl->cid_,
                                           0);
    award_exp += tui_tu_cfg->first_exp_award_;
    ttl->first_award_ = 1;
    to_update_db = true;
    cheng_jiu_module::on_complete_tui_tu(player);
  }
  
  int star = tui_tu_module::do_calc_star(cur_used_time, tui_tu_cfg->limit_time_);

  task_module::on_finish_tui_tu(player, ttl->cid_, star);
  tui_tu_module::on_star_update(player);
  { // for kai fu act
    int cur_chapter = tui_tu_module::get_chapter_idx(TUI_TU_COMMON, ttl->cid_);
    tui_tu_chapter_cfg_obj *chapter_cfg = tui_tu_module::get_chapter_cfg(TUI_TU_COMMON, cur_chapter);
    if (chapter_cfg != NULL)
    {
      ilist_node<int> *itor = chapter_cfg->scene_cid_list_.head();
      for(; itor != NULL; itor = itor->next_)
        if (tui_tu_module::get_tui_tu_log(player, itor->value_) == NULL)
          break;
      if (itor == NULL)
        kai_fu_act_obj::on_tui_tu_end(player, cur_chapter);
    }
  }

  if (!ttl->is_perfect_
      && cur_used_time <= tui_tu_cfg->limit_time_ * global_param_cfg::tui_tu_perfect_p / 100)
    ttl->is_perfect_ = true;

  award_exp += lvl_param_cfg::instance()->mission_exp(player->lvl())\
               * (10000 + vip_module::to_get_tui_tu_add_exp(player)) / 10000;
  player->do_got_exp(award_exp);
  if (to_update_db
      && ttl->cid_ != global_param_cfg::birth_mission_cid)  // 出生的推图是不用处理的
    tui_tu_module::db_update_tui_tu_log(player, ttl);

  dropped_item::do_clear_dropped_item_list();
  tui_tu_turn_cfg_obj *turn_cfg = tui_tu_turn_config::instance()->get_tui_tu_turn_cfg_obj(tui_tu_cfg->turn_award_);
  if (turn_cfg != NULL)
  {
    for (ilist_node<pair_t<int> > *itor = turn_cfg->box_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      dropped_item::do_drop_item_from_box(0,
                                          0,
                                          itor->value_.first_,  // 借用一下 src_cid
                                          itor->value_.second_,
                                          dropped_item::dropped_item_list);
    }
  }
  char turn_item_amt = dropped_item::dropped_item_list.size();
  if (turn_item_amt > MAX_TUI_TU_TURN_ITEM_AMOUNT)
    turn_item_amt = MAX_TUI_TU_TURN_ITEM_AMOUNT;
  out_stream os(client::send_buf, client::send_buf_len);
  os << cur_used_time << ttl->free_turn_cnt_ << turn_item_amt;
  while (!dropped_item::dropped_item_list.empty())
  {
    dropped_item *di = dropped_item::dropped_item_list.pop_front();
    if (player->tui_tu_turn_item_list_.size() < MAX_TUI_TU_TURN_ITEM_AMOUNT)
    {
      int real_item_cid = di->cid();
      if (clsid::is_char_equip(real_item_cid))
        real_item_cid = clsid::get_equip_cid_by_career(player->career(), real_item_cid);
      tui_tu_turn_item_info *tp = new tui_tu_turn_item_info();
      tp->rate_ = di->src_cid_;
      tp->item_ = item_amount_bind_t(real_item_cid, di->amount_, di->bind_type_);
      player->tui_tu_turn_item_list_.push_back(tp);
      os << real_item_cid << di->amount_ << di->bind_type_;
    }
    dropped_item_pool::instance()->release(di);
  }
  player->send_request(NTF_NOTIFY_CLT_TUI_TU_END_INFO, &os);
  return 0;
}
void tui_tu_module::do_notify_boss_drop_items(player_obj *player, const int boss_cid)
{
  monster_cfg_obj *mco = monster_cfg::instance()->get_monster_cfg_obj(boss_cid);
  if (mco == NULL) return ;

  dropped_item::do_clear_dropped_item_list();
  int drop_times = ltime_act_module::on_drop_items();
  for (int i = 0; i < drop_times; ++i)
  {
    dropped_item::do_build_drop_item_list(player->id(),
                                          0,
                                          boss_cid,
                                          dropped_item::dropped_item_list);
  }

  for (ilist_node<dropped_item *> *itor = dropped_item::dropped_item_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    dropped_item *di = itor->value_;
    int real_item_cid = di->cid();
    if (clsid::is_char_equip(real_item_cid))
      real_item_cid = clsid::get_equip_cid_by_career(player->career(), real_item_cid);
    package_module::do_insert_or_mail_item(player,
                                           real_item_cid,
                                           di->amount_,
                                           di->bind_type_,
                                           mail_config::PACKAGE_IS_FULL,
                                           MONEY_GOT_TUI_TU_TURN,
                                           ITEM_GOT_TUI_TU_TURN,
                                           player->scene_cid(),
                                           0);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  char *count = (char *)os.wr_ptr();
  os << (char)0;
  while (!dropped_item::dropped_item_list.empty())
  {
    dropped_item *di = dropped_item::dropped_item_list.pop_front();
    int real_item_cid = di->cid();
    if (clsid::is_char_equip(real_item_cid))
      real_item_cid = clsid::get_equip_cid_by_career(player->career(), real_item_cid);
    os << real_item_cid << di->amount_ << di->bind_type_;
    ++(*count);
    dropped_item_pool::instance()->release(di);
  }
  if (*count > 0)
    player->send_request(NTF_TUI_TU_BOSS_DROP, &os);
}
void tui_tu_module::on_kill_mst(player_obj *player,
                                const int mst_cid,
                                ilist<dropped_item *> &dropped_item_list)
{
  if (!clsid::is_tui_tu_scp_scene(player->scene_cid())) return ;
  const tui_tu_cfg_obj *ttco = tui_tu_config::instance()->get_tui_tu_cfg_obj(player->scene_cid());
  if (ttco == NULL) return ;
  if (ttco->boss_cid_ == mst_cid)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    char *count = (char *)os.wr_ptr();
    os << (char)0;
    for (ilist_node<dropped_item *> *itor = dropped_item_list.head();
         itor != NULL;
         itor = itor->next_)
    {
      dropped_item *di = itor->value_;
      int real_item_cid = di->cid();
      if (clsid::is_char_equip(real_item_cid))
        real_item_cid = clsid::get_equip_cid_by_career(player->career(), real_item_cid);
      os << real_item_cid << di->amount_ << di->bind_type_;
      ++(*count);
    }
    if (*count > 0)
      player->send_request(NTF_TUI_TU_BOSS_DROP, &os);
  }
}
int tui_tu_module::clt_tui_tu_do_turn(player_obj *player, const char *msg, const int len)
{
  int scene_cid = 0;
  char type = 0;
  in_stream is(msg, len);
  is >> type >> scene_cid;
  if (!clsid::is_tui_tu_scp_scene(scene_cid))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (player->tui_tu_turn_item_list_.empty())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_TURN_OVER);

  const tui_tu_cfg_obj *tui_tu_cfg = tui_tu_config::instance()->get_tui_tu_cfg_obj(scene_cid);
  if (tui_tu_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  tui_tu_log *tl = tui_tu_module::get_tui_tu_log(player, scene_cid);
  if (tl == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_NOT_EXIST);

  item_amount_bind_t ok_one;
  if (type == 2) // all
  {
    int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND, tui_tu_cfg->turn_all_cost_);
    if (ret != 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ret); 

    while (!player->tui_tu_turn_item_list_.empty())
    {
      tui_tu_turn_item_info *tinfo = player->tui_tu_turn_item_list_.pop_front();
      package_module::do_insert_or_mail_item(player,
                                             tinfo->item_.cid_,
                                             tinfo->item_.amount_,
                                             tinfo->item_.bind_,
                                             mail_config::TUI_TU_TURN_AWARD,
                                             MONEY_GOT_TUI_TU_TURN,
                                             ITEM_GOT_TUI_TU_TURN,
                                             scene_cid,
                                             0);
      delete tinfo;
    } // end of `while ('
    player->do_lose_money(tui_tu_cfg->turn_all_cost_,
                          M_BIND_UNBIND_DIAMOND,
                          MONEY_LOSE_TUI_TU_TURN_ALL,
                          scene_cid,
                          0,
                          0);
  }else if (type == 1) // general
  {
    if (tl->free_turn_cnt_
        >= (tui_tu_cfg->free_turn_cnt_
            + vip_module::to_get_tui_tu_free_turn_cnt(player)
            + (tl->is_perfect_ ? 1 : 0)
            ))
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_TURN_FREE_CNT_NOT_ENOUGH);

    int total_rate = 0;
    for (ilist_node<tui_tu_turn_item_info *> *itor = player->tui_tu_turn_item_list_.head();
         itor != NULL;
         itor = itor->next_)
      total_rate += itor->value_->rate_;
    if (total_rate == 0) total_rate = 1;

    int box_rand_rate = rand() % total_rate + 1;
    int box_drop_rate_1 = 0;
    int box_drop_rate_2 = 0;
    bool first_box = true;
    int s = player->tui_tu_turn_item_list_.size();
    for (int i = 1; i <= s; ++i)
    {
      tui_tu_turn_item_info *ti = player->tui_tu_turn_item_list_.pop_front();
      util::cake_rate(ti->rate_,
                      first_box,
                      box_drop_rate_1,
                      box_drop_rate_2);
      if (box_rand_rate < box_drop_rate_1
          || box_rand_rate > box_drop_rate_2)
      {
        player->tui_tu_turn_item_list_.push_back(ti);
        continue;
      }
      ok_one = ti->item_;
      delete ti;
      break;
    }
    if (ok_one.cid_ == 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

    package_module::do_insert_or_mail_item(player,
                                           ok_one.cid_,
                                           ok_one.amount_,
                                           ok_one.bind_,
                                           mail_config::TUI_TU_TURN_AWARD,
                                           MONEY_GOT_TUI_TU_TURN,
                                           ITEM_GOT_TUI_TU_TURN,
                                           scene_cid,
                                           0);
    tl->free_turn_cnt_ += 1;
  }else // end of `if (type == 1)'
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  out_stream os(client::send_buf, client::send_buf_len);
  os << ok_one.cid_ << ok_one.amount_;
  return player->send_respond_ok(RES_TUI_TU_DO_TURN, &os); 
}
int tui_tu_module::clt_tui_tu_sao_dang(player_obj *player, const char *msg, const int len)
{
  int scene_cid = 0;
  char auto_buy = 0;
  in_stream is(msg, len);
  is >> scene_cid >> auto_buy;

  const tui_tu_cfg_obj *tui_tu_cfg = tui_tu_config::instance()->get_tui_tu_cfg_obj(scene_cid);
  if (tui_tu_cfg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  tui_tu_log *ttl = tui_tu_module::get_tui_tu_log(player, scene_cid);
  if (ttl == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TUI_TU_NOT_EXIST);;

  if (ttl->state_ != tui_tu_log::ST_FINISHED)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = player->do_check_item_enough_by_auto_buy(global_param_cfg::sao_dang_ka_cid, 1, auto_buy);
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  if (player->char_extra_info_->ti_li_ < tui_tu_cfg->tili_cost_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TI_LI_NOT_ENOUGH);

  int auto_buy_cai_liao_amt = ret;

  ttl->free_turn_cnt_ = 0;
  ttl->is_perfect_ = false;
  player->do_reduce_ti_li(tui_tu_cfg->tili_cost_);
  tui_tu_module::do_clean_resource(player);

  ret = tui_tu_module::do_finish_tui_tu(player, ttl, tui_tu_cfg, true);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->do_auto_buy_item(global_param_cfg::sao_dang_ka_cid, auto_buy_cai_liao_amt);
  if (auto_buy_cai_liao_amt < 1)
    package_module::do_remove_item(player,
                                   global_param_cfg::sao_dang_ka_cid,
                                   1 - auto_buy_cai_liao_amt,
                                   ITEM_LOSE_USE,
                                   scene_cid);

  return 0;
}
int tui_tu_module::do_calc_star(const int used_time, const int limit_time)
{
  int star = 0;
  if (used_time <= 0)
    star = 0;
  else if (used_time <= limit_time * global_param_cfg::tui_tu_star_p.first_ / 100)
    star = 3;
  else if (used_time > limit_time * global_param_cfg::tui_tu_star_p.first_ / 100
           && used_time <= limit_time * global_param_cfg::tui_tu_star_p.second_ / 100)
  {
    star = 2;
  }else
    star = 1;

  return star;
}
int tui_tu_module::get_total_star(player_obj *player)
{
  int sum = 0;
  for (ilist_node<tui_tu_log *> *itor = player->tui_tu_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    const tui_tu_cfg_obj *tui_tu_cfg = tui_tu_config::instance()->get_tui_tu_cfg_obj(itor->value_->cid_);
    if (tui_tu_cfg == NULL) continue;
    sum += tui_tu_module::do_calc_star(itor->value_->used_time_, tui_tu_cfg->limit_time_);
  }
  return sum;
}
void tui_tu_module::on_star_update(player_obj *player)
{
  int sum = tui_tu_module::get_total_star(player);
  if (player->char_extra_info_->total_mstar_ != sum)
  {
    rank_module::on_char_mstar_change(player->id(),
                                      player->char_extra_info_->total_mstar_,
                                      sum);
    player->char_extra_info_->total_mstar_ = sum;
    player->db_save_char_extra_info();
  }
}
