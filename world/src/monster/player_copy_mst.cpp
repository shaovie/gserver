#include "player_copy_mst.h"
#include "tianfu_skill_info.h"
#include "global_macros.h"
#include "all_char_info.h"
#include "skill_module.h"
#include "fighter_mgr.h"
#include "mblock_pool.h"
#include "guild_module.h"
#include "scene_config.h"
#include "monster_mgr.h"
#include "scp_module.h"
#include "scp_mgr.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "sys_log.h"
#include "time_util.h"
#include "fa_bao_module.h"

// Lib header
#include "mblock.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("mst");
static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");

static int s_player_copy_mst_id_pool = PLAYER_COPY_ID_BASE;
DIR_STEP(s_dir_step);

player_copy_mst::player_copy_mst() :
  char_id_(0),
  guild_id_(0),
  last_use_skill_cid_(0),
  last_select_skill_idx_(0),
  jing_ji_end_time_(0),
  to_patrol_time_(0)
{ }
player_copy_mst::~player_copy_mst()
{ }
//= aev
void player_copy_mst::dispatch_aev(const int aev_id, mblock *mb)
{
  if (aev_id == AEV_PUSH_SKILL)
    this->aev_push_skill(mb);
  else if (aev_id == AEV_ZHUZAI_FEN_SHEN_INFO)
    this->aev_zhuzai_fen_shen(mb);
  else if (aev_id == AEV_ZHUZAI_FEN_SHEN_DESTROY)
    this->aev_zhuzai_fen_shen_destroy(mb);
  else if (aev_id == AEV_JING_JI_END)
    this->aev_jing_ji_end(mb);
  else if (aev_id == AEV_TO_PATROL)
    this->aev_to_patrol(mb);
  else
    super::dispatch_aev(aev_id, mb);
}
void player_copy_mst::aev_push_skill(mblock *mb)
{
  int skill_cid = 0;
  short skill_lvl = 0;
  *mb >> skill_cid >> skill_lvl;
  for (ilist_node<skill_info *> *itor = this->skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (skill_cid == itor->value_->cid_)
      return ;
  }

  skill_info *si   = skill_info_pool::instance()->alloc();
  si->cid_         = skill_cid;
  si->rate_        = 100;
  si->lvl_         = skill_lvl;
  this->skill_list_.push_back(si);
}
void player_copy_mst::aev_zhuzai_fen_shen(mblock *mb)
{
  this->zhuzai_fen_shen_.push_back(*((int *)mb->rd_ptr()));
}
void player_copy_mst::aev_zhuzai_fen_shen_destroy(mblock *mb)
{
  this->zhuzai_fen_shen_.remove(*((int *)mb->rd_ptr()));
}
void player_copy_mst::aev_jing_ji_end(mblock *)
{
  this->jing_ji_end_time_ = time_util::now;
  this->live_time_ = time_util::now - this->birth_time_ + 3 + rand() % 5;
  this->to_clear_zhu_zai_fen_shen();
}
void player_copy_mst::aev_to_patrol(mblock *)
{
  if (this->to_patrol_time_ != 0) return ;
  this->to_patrol_time_ = time_util::now + 1 + rand() % 4;
}
int player_copy_mst::get_player_copy_id(const int char_id)
{
  monster_obj *pm = monster_mgr::instance()->find(char_id);
  if (pm != NULL)
    return pm->master_id();
  return char_id;
}
int player_copy_mst::do_assign_id()
{
  if (s_player_copy_mst_id_pool >= (PLAYER_COPY_ID_BASE + PLAYER_COPY_ID_RANGE - 1))
    s_player_copy_mst_id_pool = PLAYER_COPY_ID_BASE;
  return ++s_player_copy_mst_id_pool;
}
void player_copy_mst::do_release_id()
{ return ; }
void player_copy_mst::master_id(const int char_id)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return ;

  this->char_id_ = char_id;
  this->guild_id_ = guild_module::get_guild_id(this->char_id_);

  this->lvl_ = cbi->lvl_;
  this->career_ = cbi->career_;
  this->common_cd_ = global_param_cfg::char_common_cd[(int)this->career_];

  // skill
  int *birth_skill_list = skill_module::get_common_skills(this->career_);
  this->base_skill_cid_ = birth_skill_list[0];
  for (int i = 0; i < 64/*magic number*/; ++i)
  {
    if (birth_skill_list[i] == 0) break;
    skill_info *si   = skill_info_pool::instance()->alloc();
    si->cid_         = birth_skill_list[i];
    si->rate_        = 100;
    si->lvl_         = 1;
    this->skill_list_.push_back(si);
  }

  // attr
  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player != NULL)
  {
    obj_attr &o_attr = player->get_obj_attr();
    obj_attr::attr_v_add[ATTR_T_HP]              = o_attr.total_hp();
    obj_attr::attr_v_add[ATTR_T_MP]              = o_attr.total_mp();
    obj_attr::attr_v_add[ATTR_T_GONG_JI]         = o_attr.gong_ji();
    obj_attr::attr_v_add[ATTR_T_FANG_YU]         = o_attr.fang_yu();
    obj_attr::attr_v_add[ATTR_T_MING_ZHONG]      = o_attr.ming_zhong();
    obj_attr::attr_v_add[ATTR_T_SHAN_BI]         = o_attr.shan_bi();
    obj_attr::attr_v_add[ATTR_T_BAO_JI]          = o_attr.bao_ji();
    obj_attr::attr_v_add[ATTR_T_KANG_BAO]        = o_attr.kang_bao();
    obj_attr::attr_v_add[ATTR_T_SHANG_MIAN]      = o_attr.shang_mian();

    // tian fu skill
    for (ilist_node<tianfu_skill_info *> *itor = player->get_tianfu_list().head();
         itor != NULL;
         itor = itor->next_)
    {
      tianfu_skill_info *tf_info = tianfu_skill_info_pool::instance()->alloc();
      tf_info->char_id_ = this->id_;
      tf_info->cid_ = itor->value_->cid_;
      tf_info->lvl_ = itor->value_->lvl_;
      this->tianfu_skill_list_.push_back(tf_info);
    }

    // fa bao attr
    fa_bao_module::do_build_fa_bao_attr(this,
                                        player->fa_bao_dj(),
                                        player->fa_bao());
  }else
  {
    obj_attr::attr_v_add[ATTR_T_HP]              = cbi->attr_[ATTR_T_HP];
    obj_attr::attr_v_add[ATTR_T_MP]              = cbi->attr_[ATTR_T_MP];
    obj_attr::attr_v_add[ATTR_T_GONG_JI]         = cbi->attr_[ATTR_T_GONG_JI];
    obj_attr::attr_v_add[ATTR_T_FANG_YU]         = cbi->attr_[ATTR_T_FANG_YU];
    obj_attr::attr_v_add[ATTR_T_MING_ZHONG]      = cbi->attr_[ATTR_T_MING_ZHONG];
    obj_attr::attr_v_add[ATTR_T_SHAN_BI]         = cbi->attr_[ATTR_T_SHAN_BI];
    obj_attr::attr_v_add[ATTR_T_BAO_JI]          = cbi->attr_[ATTR_T_BAO_JI];
    obj_attr::attr_v_add[ATTR_T_KANG_BAO]        = cbi->attr_[ATTR_T_KANG_BAO];
    obj_attr::attr_v_add[ATTR_T_SHANG_MIAN]      = cbi->attr_[ATTR_T_SHANG_MIAN];

    // tian fu skill
    for (ilist_node<pair_t<int> > *itor = cbi->tianfu_skill_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      tianfu_skill_info *tf_info = tianfu_skill_info_pool::instance()->alloc();
      tf_info->char_id_ = this->id_;
      tf_info->cid_ = itor->value_.first_;
      tf_info->lvl_ = itor->value_.second_;
      this->tianfu_skill_list_.push_back(tf_info);
    }

    // fa bao attr
    fa_bao_module::do_build_fa_bao_attr(this,
                                        cbi->fa_bao_dj_,
                                        cbi->fa_bao_);
  }
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);

  this->obj_attr_.hp_ = this->total_hp();
  this->obj_attr_.mp_ = this->total_mp();
}
int player_copy_mst::do_rebirth_init(const time_value &now)
{
  this->cid_ = this->id_;
  this->dir_ = DIR_DOWN;
  this->coord_x_ = this->birth_coord_x_;
  this->coord_y_ = this->birth_coord_y_;

  this->status_           = ST_NULL;
  this->do_status_        = DO_PATROLING;
  this->unit_status_      = 0;
  this->birth_time_       = now.sec();
  this->att_type_         = ACTIVE_ATT;
  this->eye_radius_       = 100;
  this->chase_radius_     = 100;
  this->live_time_        = 5*60;

  this->att_obj_id_       = 0;

  this->move_speed_       = PLAYER_DEFAULT_SPEED; // same to player
  this->chase_speed_      = this->move_speed_;
  this->patrol_speed_     = this->move_speed_;
  this->prev_move_speed_  = this->move_speed_;
  this->fight_back_delay_ = 600;

  return 0;
}
int player_copy_mst::do_think(const time_value &now)
{
  fa_bao_module::do_timeout(this, time_util::now);
  return super::do_think(now);
}
void player_copy_mst::do_back(const time_value &)
{
  this->on_back();
}
void player_copy_mst::on_attack_somebody(const int target_id,
                                         const int /*obj_type*/,
                                         const time_value &/*now*/,
                                         const int /*skill_cid*/,
                                         const int /*real_hurt*/)
{
  if (target_id != 0
      && this->career_ == CAREER_LI_LIANG
      && !this->zhuzai_fen_shen_.empty())
  {
    int ls = this->zhuzai_fen_shen_.size();
    for (int i = 0; i < ls; ++i)
    {
      int fen_shen_id = this->zhuzai_fen_shen_.pop_front();
      char_obj *fen_shen = fighter_mgr::instance()->find(fen_shen_id);
      if (fen_shen != NULL)
      {
        mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
        *mb << target_id;
        fen_shen->post_aev(AEV_NOTIFY_ATTACK_TARGET_ID, mb);
        this->zhuzai_fen_shen_.push_back(fen_shen_id);
      }
    }
  }
}
void player_copy_mst::do_patrol(const time_value &now)
{
  if (this->to_patrol_time_ == 0
      || time_util::now < this->to_patrol_time_)
    return ;

  if (this->career_ == CAREER_ZHI_LI
      && !this->move_path_.empty()
      && now > this->reach_pos_time_
      && this->can_move(0, 0) == 0)
  {
    coord_t next_pos = this->move_path_.pop_front();
    this->do_move_i(now, next_pos.x_, next_pos.y_);
    return ;
  }
  this->do_search_in_view_area(now);
}
void player_copy_mst::do_dead(const int /*killer_id*/)
{
  if (this->can_switch_to_status(ST_DIE_IN_SCENE) != 0) return ;
  this->status_ = ST_DIE_IN_SCENE;

  this->broadcast_unit_life_status(LIFE_V_DEAD);
  time_value now = time_value::gettimeofday();
  this->dead_time_ = now + time_value(3, 0);
  buff_module::do_remove_all_buff(this);

  this->to_clear_zhu_zai_fen_shen();

  // do something
  if (this->scene_cid_ == global_param_cfg::jing_ji_map
      || this->scene_cid_ == global_param_cfg::lue_duo_map)
  {
    mblock mb(client::send_buf, client::send_buf_len);
    mb.data_type(SCP_EV_CHAR_DEAD);
    mb << this->id();
    scp_mgr::instance()->do_something(this->scene_id(), &mb, NULL, NULL);
  }
}
bool player_copy_mst::do_build_snap_info(const int ,
                                         out_stream &os,
                                         ilist<pair_t<int> > *old_snap_unit_list,
                                         ilist<pair_t<int> > *new_snap_unit_list)
{
  new_snap_unit_list->push_back(pair_t<int>(this->id_, this->cid_));

  if (this->is_new_one(old_snap_unit_list))
  {
    os << T_CID_ID << this->cid_ << this->id_;
    this->do_build_snap_base_info(os);
    this->do_build_snap_pos_info(os);
    this->do_build_snap_equip_info(os);
    this->do_build_snap_hp_mp_info(os);
    this->do_build_snap_status_info(os);

    os << T_END;
    return true;
  }

  return false;
}
void player_copy_mst::do_build_snap_base_info(out_stream &os)
{
  os << T_BASE_INFO;
  char vip = 0;
  short sin_val = 0;
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(this->char_id_);
  short all_qh_lvl = 0;
  short sum_bao_shi_lvl = 0;
  player_obj *player = player_mgr::instance()->find(this->char_id_);
  if (player == NULL)
  {
    if (cbi != NULL)
    {
      all_qh_lvl = cbi->all_qh_lvl_;
      sum_bao_shi_lvl = cbi->sum_bao_shi_lvl_;
    }
  }else
  {
    all_qh_lvl = player->all_qh_lvl();
    sum_bao_shi_lvl = player->sum_bao_shi_lvl();
  }
  if (cbi == NULL)
    os << stream_ostr("null", 4);
  else
  {
    vip = cbi->vip_;
    sin_val = cbi->sin_val_;
    os << stream_ostr(cbi->name_, ::strlen(cbi->name_));
  }

  os << this->career_
    << this->lvl_
    << sin_val;
  guild_module::do_fetch_char_guild_info(this->guild_id_, os);
  os << vip
    << cbi->title_cid_
    << fa_bao_module::fa_bao_dj(this)
    << all_qh_lvl
    << sum_bao_shi_lvl;
}
void player_copy_mst::do_build_snap_pos_info(out_stream &os)
{
  os << T_POS_INFO
    << this->dir_
    << this->coord_x_
    << this->coord_y_
    << this->move_speed();
}
void player_copy_mst::do_build_snap_status_info(out_stream &os)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    os << T_STATUS << (char)SNAP_LIFE << (char)1;
  else
    os << T_STATUS << (char)SNAP_LIFE << (char)0;
}
void player_copy_mst::do_build_snap_equip_info(out_stream &os)
{
  int zhu_wu_cid = 0;
  int fu_wu_cid = 0;
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(this->char_id_);
  if (cbi != NULL)
  {
    zhu_wu_cid = cbi->zhu_wu_cid_;
    fu_wu_cid = cbi->fu_wu_cid_;
  }
  os << T_EQUIP_INFO << zhu_wu_cid << fu_wu_cid;
}
skill_info *player_copy_mst::get_skill(const int skill_cid)
{
  for (ilist_node<skill_info *> *itor = this->skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == skill_cid)
      return itor->value_;
  }
  return NULL;
}
skill_info *player_copy_mst::do_select_skill(const time_value &now,
                                             char_obj *target,
                                             int &result,
                                             int &skill_hurt_delay,
                                             const skill_detail *&sd)
{
  if (this->jing_ji_end_time_ != 0)
    return NULL;

  skill_info *select_skill_si = NULL;
  if (this->career_ == CAREER_LI_LIANG)
    select_skill_si = this->do_select_skill_for_c1(now, target, result);
  else if (this->career_ == CAREER_MIN_JIE)
    select_skill_si = this->do_select_skill_for_c2(now, target, result);
  else if (this->career_ == CAREER_ZHI_LI)
    select_skill_si = this->do_select_skill_for_c3(now, target, result);
  
  if (select_skill_si == NULL)
    return NULL;

  const skill_cfg_obj *sco = skill_config::instance()->get_skill(select_skill_si->cid_);
  if (sco == NULL
      || (sd = sco->get_detail(select_skill_si->lvl_)) == NULL)
    return NULL;
  skill_hurt_delay = sco->hurt_delay_;

  result = this->can_use_skill(sd,
                               select_skill_si,
                               target->block_radius(),
                               now,
                               coord_t(target->coord_x(), target->coord_y()));
  if (result == 0) return select_skill_si;

  return NULL;
}
void player_copy_mst::on_use_skill_ok(skill_info *si,
                                      const skill_detail *sd,
                                      const int skill_hurt_delay,
                                      const time_value &now)
{
  super::on_use_skill_ok(si, sd, skill_hurt_delay, now);
  this->last_use_skill_cid_ = si->cid_;
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
void player_copy_mst::on_attr_update(const int , const int)
{
  this->broadcast_hp_mp();
}
skill_info *player_copy_mst::do_select_skill_for_c1(const time_value &now,
                                                    char_obj *target,
                                                    int &result)
{
  skill_info *select_skill_si = NULL;
  static int s_skill_list[] = {
    31110011, // 战斗怒吼
    31110021, // 镜像分身
    31110013, // 剑刃风暴
  };
  if (this->has_buff(BF_SK_XUAN_FENG))
  {
    skill_info *si = this->get_skill(31110013);
    if (si == NULL) return NULL;
    const skill_detail *sd = skill_config::instance()->get_detail(si->cid_, si->lvl_);
    if (sd == NULL) return NULL;
    if (util::is_out_of_distance(this->coord_x_,
                                 this->coord_y_,
                                 target->coord_x(),
                                 target->coord_y(),
                                 sd->param_1_))
    {
      // 不能立即追，这样看起来逼真些
      this->attack_done_time_ = time_value(now.sec(), now.usec() + 800000);
      result = ERR_SKILL_OUT_OF_DISTANCE;
    }
    return NULL;
  }

  for (size_t i = 0; i < sizeof(s_skill_list)/sizeof(s_skill_list[0]); ++i)
  {
    skill_info *si = this->get_skill(s_skill_list[i]);
    if (si == NULL) continue;
    const skill_detail *sd = skill_config::instance()->get_detail(si->cid_, si->lvl_);
    if (sd == NULL) continue;
    if (si->cid_ == 31110011 // 战斗怒吼
        && sd->param_1_ > 0
        && util::is_out_of_distance(this->coord_x_,
                                    this->coord_y_,
                                    target->coord_x(),
                                    target->coord_y(),
                                    sd->param_1_))
    {
      result = ERR_SKILL_OUT_OF_DISTANCE;
      break;
    }
    if ((now - time_value(si->use_time_, si->use_usec_)).msec() < sd->cd_)
      continue;

    select_skill_si = si;
    break;
  }

  if (select_skill_si == NULL
      && result != ERR_SKILL_OUT_OF_DISTANCE)
    select_skill_si = this->do_select_common_skill(now);
  return select_skill_si;
}
skill_info *player_copy_mst::do_select_skill_for_c2(const time_value &now,
                                                    char_obj *target,
                                                    int &result)
{
  skill_info *select_skill_si = NULL;
  static int s_skill_list[] = {
    31210012, // 疾风猎刃
    31210021, // 恶魔变身
    31210011, // 破空之刃
    31210013, // 烈焰火环
  };
  if (this->has_buff(BF_SK_BIAN_SHEN))
    return this->get_skill(MIN_JIE_BIAN_SHEN_COMMON_SKILL);

  for (size_t i = 0; i < sizeof(s_skill_list)/sizeof(s_skill_list[0]); ++i)
  {
    skill_info *si = this->get_skill(s_skill_list[i]);
    if (si == NULL) continue;
    const skill_detail *sd = skill_config::instance()->get_detail(si->cid_, si->lvl_);
    if (sd == NULL) continue;
    if (sd->param_1_ > 0
        && util::is_out_of_distance(this->coord_x_,
                                    this->coord_y_,
                                    target->coord_x(),
                                    target->coord_y(),
                                    sd->param_1_))
    {
      result = ERR_SKILL_OUT_OF_DISTANCE;
      break;
    }
    if ((now - time_value(si->use_time_, si->use_usec_)).msec() < sd->cd_)
      continue;

    select_skill_si = si;
    break;
  }
  if (select_skill_si == NULL
      && result != ERR_SKILL_OUT_OF_DISTANCE)
    select_skill_si = this->do_select_common_skill(now);
  return select_skill_si;
}
skill_info *player_copy_mst::do_select_skill_for_c3(const time_value &now,
                                                    char_obj *target,
                                                    int &result)
{
  skill_info *select_skill_si = NULL;
  static int s_skill_list[] = {
    31310021, // 寒冰护体
    31310012, // 冰锥术
    31310013, // 暴风雪
    31310011, // 龙破斩
  };

  for (size_t i = 0; i < sizeof(s_skill_list)/sizeof(s_skill_list[0]); ++i)
  {
    skill_info *si = this->get_skill(s_skill_list[i]);
    if (si == NULL) continue;
    const skill_detail *sd = skill_config::instance()->get_detail(si->cid_, si->lvl_);
    if (sd == NULL) continue;
    if (sd->param_1_ > 0
        && util::is_out_of_distance(this->coord_x_,
                                    this->coord_y_,
                                    target->coord_x(),
                                    target->coord_y(),
                                    sd->param_1_))
    {
      result = ERR_SKILL_OUT_OF_DISTANCE;
      break;
    }
    if ((now - time_value(si->use_time_, si->use_usec_)).msec() < sd->cd_)
      continue;

    select_skill_si = si;
    break;
  }
  if (select_skill_si == NULL
      && result != ERR_SKILL_OUT_OF_DISTANCE)
    select_skill_si = this->do_select_common_skill(now);

  // 远离对手
  const int max_distance = 3;
  if ((target->career() == CAREER_MIN_JIE || target->career() == CAREER_LI_LIANG)
      && util::is_inside_of_redius(this->coord_x_,
                                   this->coord_y_,
                                   target->coord_x(),
                                   target->coord_y(),
                                   max_distance)
      && rand() % 4 == 0)
  {
    this->move_path_.clear();
    short goto_dir = util::calc_next_dir(target->coord_x(),
                                         target->coord_y(),
                                         this->coord_x_,
                                         this->coord_y_);
    short cur_x = this->coord_x_;
    short cur_y = this->coord_y_;
    for (int i = 0; i < max_distance; ++i)
    {
      cur_x += s_dir_step[goto_dir][0];
      cur_y += s_dir_step[goto_dir][1];
      if (scene_config::instance()->can_move(this->scene_cid_, cur_x, cur_y))
        this->move_path_.push_back(coord_t(cur_x, cur_y));
      else
        break;
    }
    if (!this->move_path_.empty())
    {
      if (goto_dir != this->dir_)
        this->do_adjust_dir(cur_x, cur_y);
      this->att_obj_id_ = 0;
      this->to_patrol();
    }
  }
  return select_skill_si;
}
void player_copy_mst::to_clear_zhu_zai_fen_shen()
{
  if (!this->zhuzai_fen_shen_.empty())
  {
    int ls = this->zhuzai_fen_shen_.size();
    for (int i = 0; i < ls; ++i)
    {
      int fen_shen_id = this->zhuzai_fen_shen_.pop_front();
      char_obj *fen_shen = fighter_mgr::instance()->find(fen_shen_id);
      if (fen_shen != NULL)
      {
        fen_shen->post_aev(AEV_ZHUZAI_FEN_SHEN_TO_DESTROY, NULL);
        this->zhuzai_fen_shen_.push_back(fen_shen_id);
      }
    }
  }
}
skill_info *player_copy_mst::do_select_common_skill(const time_value &now)
{
  skill_info *select_skill_si = NULL;
  int *birth_skill_list = skill_module::get_common_skills(this->career_);
  if (this->last_use_skill_cid_ == birth_skill_list[0])
    select_skill_si = this->get_skill(birth_skill_list[1]);
  else if (this->last_use_skill_cid_ == birth_skill_list[1])
    select_skill_si = this->get_skill(birth_skill_list[2]);

  if (select_skill_si == NULL)
    select_skill_si = this->get_skill(birth_skill_list[0]);

  if (select_skill_si != NULL) // 选中普通技能
  {
    const skill_detail *sd = skill_config::instance()->get_detail(select_skill_si->cid_,
                                                                  select_skill_si->lvl_);
    if (sd == NULL
        || ((now - time_value(select_skill_si->use_time_, select_skill_si->use_usec_)).msec() < sd->cd_))
      select_skill_si = NULL;
  }
  return select_skill_si;
}
