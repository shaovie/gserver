#include "skill_module.h"
#include "jing_ji_module.h"
#include "mblock_pool.h"
#include "global_param_cfg.h"
#include "behavior_id.h"
#include "player_obj.h"
#include "db_proxy.h"
#include "fighter_mgr.h"
#include "skill_config.h"
#include "skill_info.h"
#include "sys_log.h"
#include "message.h"
#include "clsid.h"
#include "error.h"
#include "util.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("skill");
static ilog_obj *e_log = err_log::instance()->get_ilog("skill");

int skill_module::handle_db_get_skill_list_result(player_obj *player, in_stream &is)
{
  player->handle_db_get_skill_list_result(is);
  return 0;
}
int skill_module::handle_db_get_other_skill_list_result(player_obj *player, in_stream &is)
{
  jing_ji_module::handle_db_get_other_skill_list_result(player, is);
  return 0;
}
void player_obj::handle_db_get_skill_list_result(in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char skill_bf[sizeof(skill_info) + 4] = {0};
    stream_istr si(skill_bf, sizeof(skill_bf));
    is >> si;
    skill_info *sinfo = (skill_info *)skill_bf;
    skill_map_itor itor = this->skill_map_.find(sinfo->cid_);
    if (itor != this->skill_map_.end()) continue;

    const skill_detail *sd = skill_config::instance()->get_detail(sinfo->cid_, sinfo->lvl_);
    if (sd == NULL) continue;

    skill_info *new_sinfo = skill_info_pool::instance()->alloc();
    ::memcpy(new_sinfo, sinfo, sizeof(skill_info));
    this->skill_map_.insert(std::make_pair(sinfo->cid_, new_sinfo));
  }
}
int skill_module::db_insert_skill_info(player_obj *player, skill_info *si)
{
  stream_ostr char_so((const char *)si, sizeof(skill_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << player->db_sid() << char_so;
  if (db_proxy::instance()->send_request(player->id(),
                                         REQ_INSERT_SKILL,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int skill_module::db_update_skill_info(player_obj *player, skill_info *si)
{
  stream_ostr char_so((const char *)si, sizeof(skill_info));
  out_stream char_os(client::send_buf, client::send_buf_len);
  char_os << player->db_sid() << char_so;
  if (db_proxy::instance()->send_request(player->id(),
                                         REQ_UPDATE_SKILL,
                                         &char_os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int player_obj::clt_use_skill(const char *msg, const int len)
{
  int   skill_cid   = 0;
  int   target_id   = 0;
  short target_x    = 0;
  short target_y    = 0;

  in_stream is(msg, len);
  is >> skill_cid >> target_id >> target_x >> target_y;

  skill_map_itor itor = this->skill_map_.find(skill_cid);
  if (itor == this->skill_map_.end())
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_SKILL_NOT_EXIST);

  skill_info *si = itor->second;
  time_value now = time_value::gettimeofday();
  const skill_cfg_obj *sco = skill_config::instance()->get_skill(skill_cid);
  if (sco == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  const skill_detail *sd = sco->get_detail(si->lvl_);
  if (sd == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int ret = this->can_use_skill_i(sd, si, target_id, now, coord_t(target_x, target_y));
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = this->do_use_skill(target_id,
                           now,
                           target_x,
                           target_y,
                           skill_cid,
                           sco->hurt_delay_,
                           sd);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  this->on_use_skill_ok(si, sd, now);

  out_stream os(client::send_buf, client::send_buf_len);
  os << skill_cid;
  return this->send_respond_ok(RES_USE_SKILL, &os);
}
int player_obj::can_use_skill_i(const skill_detail *sd,
                                const skill_info *si,
                                const int target_id,
                                const time_value &now,
                                const coord_t &skill_pos)
{
  if (this->mp() < sd->mp_) return ERR_MP_NOT_ENOUGH;
  if ((now - this->last_use_skill_time_).msec()
      < global_param_cfg::char_common_cd[(int)this->char_info_->career_])
    return ERR_SKILL_CD_LIMIT;

  int block_radius = 0;
  if (sd->distance_ > 0 && target_id != 0)
  {
    char_obj *target = fighter_mgr::instance()->find(target_id);
    if (target != NULL)
    {
      block_radius = target->block_radius();
    }else
      return ERR_SKILL_TARGET_NOT_EXIST;
  }

  return this->can_use_skill(sd, si, block_radius, now, skill_pos);
}
void player_obj::on_use_skill_ok(skill_info *si,
                                 const skill_detail *sd,
                                 const time_value &now)
{
  si->use_time_ = (int)now.sec();
  si->use_usec_ = (int)now.usec();
  if (this->do_reduce_mp(sd->mp_) > 0)
    this->broadcast_hp_mp();

  if (si->cid_ == 31110013 // 力量型 剑刃风暴
      && !this->zhuzai_fen_shen_.empty())
  {
    int ls = this->zhuzai_fen_shen_.size();
    for (int i = 0; i < ls; ++i)
    {
      int fen_shen_id = this->zhuzai_fen_shen_.pop_front();
      char_obj *fen_shen = fighter_mgr::instance()->find(fen_shen_id);
      if (fen_shen != NULL)
      {
        mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
        *mb << si->cid_ << si->lvl_;
        fen_shen->post_aev(AEV_SYNC_MASTER_USE_SKILL, mb);
        this->zhuzai_fen_shen_.push_back(fen_shen_id);
      }
    }
  }
}
// -------------------------- skill module ------------------------
int *skill_module::get_common_skills(const int career)
{
  // init common skill
  static int init_skill_list[CHAR_CAREER_CNT + 1][5] = {
    {0, 0, 0, 0},
    {LI_LIANG_COMMON_SKILL_1, LI_LIANG_COMMON_SKILL_2, LI_LIANG_COMMON_SKILL_3, 0},
    {MIN_JIE_COMMON_SKILL_1, MIN_JIE_COMMON_SKILL_2, MIN_JIE_COMMON_SKILL_3, MIN_JIE_BIAN_SHEN_COMMON_SKILL, 0},
    {ZHI_LI_COMMON_SKILL_1, ZHI_LI_COMMON_SKILL_2, ZHI_LI_COMMON_SKILL_3, 0},
  };
  return init_skill_list[career];
}
void skill_module::on_enter_game(player_obj *player)
{
  int *init_skill_list = skill_module::get_common_skills(player->career());
  for (int i = 0; i < 64/*magic number*/; ++i)
  {
    if (init_skill_list[i] == 0) break;

    int skill_cid = init_skill_list[i];
    skill_map_itor itor = player->skill_map_.find(skill_cid);
    if (itor != player->skill_map_.end()) continue;

    skill_info *new_sinfo = skill_info_pool::instance()->alloc();
    new_sinfo->char_id_ = player->id();
    new_sinfo->lvl_     = 1;
    new_sinfo->cid_     = skill_cid;
    player->skill_map_.insert(std::make_pair(new_sinfo->cid_, new_sinfo));
  }

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  char *cnt = mb.wr_ptr();
  *cnt = 0;
  mb << *cnt;
  skill_map_itor itor = player->skill_map_.begin();
  for (; itor != player->skill_map_.end(); ++itor)
  {
    mb << itor->second->cid_
      << itor->second->lvl_;
    ++(*cnt);
  }

  player->do_delivery(NTF_PUSH_CHAR_SKILL_LIST, &mb);

  skill_module::do_notify_skills_can_upgrade(player);
}
void skill_module::on_char_lvl_up(player_obj *player)
{
  skill_module::do_notify_skills_can_upgrade(player);
}
void skill_module::do_learn_skills(player_obj *player) // 暂时不用了
{
  ilist<int> *sl = skill_config::instance()->get_skill_list(player->career());
  if (sl == NULL) return ;

  for (ilist_node<int> *itor = sl->head(); itor != NULL; itor = itor->next_)
  {
    int skill_cid = itor->value_;
    if (player->skill_map_.find(skill_cid) != player->skill_map_.end())
      continue;
    const skill_detail *sd = skill_config::instance()->get_detail(skill_cid, 1);
    if (sd == NULL) continue;
    if (player->lvl() >= sd->lvl_)
      skill_module::do_learn_skill(player, skill_cid);
  }
}
void skill_module::do_notify_skills_can_upgrade(player_obj *player)
{
  ilist<int> *sl = skill_config::instance()->get_skill_list(player->career());
  if (sl == NULL) return ;

  for (ilist_node<int> *itor = sl->head(); itor != NULL; itor = itor->next_)
  {
    int skill_cid = itor->value_;
    skill_map_itor sk_itor = player->skill_map_.find(skill_cid);
    if (sk_itor == player->skill_map_.end()) continue;
    const skill_detail *sd = skill_config::instance()->get_detail(skill_cid,
                                                                  sk_itor->second->lvl_ + 1);
    if (sd == NULL) continue;
    if (player->lvl() >= sd->lvl_)
    {
      player->send_request(NTF_CHAR_SKILL_CAN_UPGRADE, NULL);
      break;
    }
  }
}
int skill_module::dispatch_msg(player_obj *player,
                               const int msg_id,
                               const char *msg,
                               const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = skill_module::FUNC(player, msg, len);     \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_UPGRADE_SKILL, clt_upgrade_skill);
    SHORT_CODE(REQ_CHAR_LEARN_SKILL, clt_learn_skill);
    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle skill msg %d return %d", msg_id, ret);
  return ret;
}
int skill_module::clt_learn_skill(player_obj *player, const char *msg, const int len)
{
  int skill_cid = 0;
  in_stream is(msg, len);
  is >> skill_cid;

  if (!clsid::is_char_zhu_dong_skill(skill_cid)) return 0;

  const skill_cfg_obj *sco = skill_config::instance()->get_skill(skill_cid);
  if (sco == NULL || sco->career_ != player->career()) return 0;

  const skill_detail* sd = skill_config::instance()->get_detail(skill_cid, 1);
  if (sd == NULL) return 0;

  skill_map_itor itor = player->skill_map_.find(skill_cid);
  if (itor != player->skill_map_.end()) return 0;

  skill_module::do_learn_skill(player, skill_cid);
  player->on_attr_update(ZHAN_LI_SKILL, skill_cid); // for zhan li update
  return 0;
}
int skill_module::clt_upgrade_skill(player_obj *player, const char *msg, const int len)
{
  int skill_cid = 0;
  in_stream is(msg, len);
  is >> skill_cid;

  skill_map_itor sk_itor = player->skill_map_.find(skill_cid);
  const skill_cfg_obj *sco = skill_config::instance()->get_skill(skill_cid);
  if (sco == NULL
      || sco->career_ != player->career()
      || sk_itor == player->skill_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const skill_detail *sd = skill_config::instance()->get_detail(skill_cid,
                                                                sk_itor->second->lvl_ + 1);

  if (sd == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  if (player->lvl() < sd->lvl_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);
  int ret = player->is_money_enough(M_COIN, sd->coin_);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  skill_info *si = sk_itor->second;
  si->char_id_ = player->id();
  si->cid_ = skill_cid;
  si->lvl_ = sd->cur_lvl_;

  skill_module::db_update_skill_info(player, si);
  player->do_lose_money(sd->coin_,
                        M_COIN,
                        MONEY_LOSE_UPGRADE_SKILL,
                        skill_cid,
                        si->lvl_,
                        0);

  player->on_attr_update(ZHAN_LI_SKILL, skill_cid); // for zhan li update
  out_stream os(client::send_buf, client::send_buf_len);
  skill_module::do_build_skill_info(si, os);
  return player->send_request(NTF_CHAR_SKILL_UPDATE, &os);
}
void skill_module::do_build_skill_info(skill_info *si, out_stream &os)
{
  os << si->cid_ << si->lvl_;
}
void skill_module::do_learn_skill(player_obj *player, const int skill_cid)
{
  skill_info *si = skill_info_pool::instance()->alloc();
  si->char_id_ = player->id();
  si->cid_ = skill_cid;
  si->lvl_ = 1;
  player->skill_map_.insert(std::make_pair(skill_cid, si));

  skill_module::db_insert_skill_info(player, si);

  out_stream os(client::send_buf, client::send_buf_len);
  skill_module::do_build_skill_info(si, os);
  player->send_request(NTF_CHAR_SKILL_UPDATE, &os);
}
