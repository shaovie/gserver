#include "zhuzai_fen_shen_mst.h"
#include "global_param_cfg.h"
#include "scene_config.h"
#include "fighter_mgr.h"
#include "mblock_pool.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

zhuzai_fen_shen_mst::zhuzai_fen_shen_mst() :
  pk_mode_(PK_MODE_PEACE),
  master_id_(0),
  team_id_(0),
  guild_id_(0)
{ }
zhuzai_fen_shen_mst::~zhuzai_fen_shen_mst()
{
  if (this->master_id_ != 0)
  {
    char_obj *co = fighter_mgr::instance()->find(this->master_id_);
    if (co != NULL)
    {
      mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*1);
      *mb << this->id_;
      co->post_aev(AEV_ZHUZAI_FEN_SHEN_DESTROY, mb);
    }
  }
}
int zhuzai_fen_shen_mst::can_attack(char_obj *target)
{
  if (target->master_id() == this->id_)
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  if (this->master_id() == target->id())
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  if (this->master_id_ == target->master_id())
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  char_obj *master = fighter_mgr::instance()->find(this->master_id_);
  if (master == NULL) return ERR_CAN_NOT_ATTACK_TARGET;
  return master->can_attack(target);
}
int zhuzai_fen_shen_mst::can_be_attacked(char_obj *attacker)
{
  int ret = super::can_be_attacked(attacker);
  if (ret != 0) return ret;
  if (this->master_id_ == attacker->master_id())
    return ERR_FRIENDS_CAN_NOT_ATTACK;
  char_obj *master = fighter_mgr::instance()->find(this->master_id_);
  if (master == NULL) return ERR_CAN_NOT_ATTACK_TARGET;
  return master->can_be_attacked(attacker);
}
int zhuzai_fen_shen_mst::do_think(const time_value &)
{
  char_obj *master = fighter_mgr::instance()->find(this->master_id_);
  if (master == NULL
      || BIT_ENABLED(master->unit_status(), OBJ_DEAD))
  {
    this->to_destroy();
    return -1;
  }
  return 0;
}
int zhuzai_fen_shen_mst::do_enter_scene()
{
  this->next_search_time_ = time_value::gettimeofday() + time_value(0, 900*1000);
  return super::do_enter_scene();
}
//= aev
void zhuzai_fen_shen_mst::dispatch_aev(const int aev_id, mblock *mb)
{
  if (aev_id == AEV_ZHUZAI_FEN_SHEN_INFO)
    this->aev_fen_shen_info(mb);
  else if (aev_id == AEV_NOTIFY_ATTACK_TARGET_ID)
    this->aev_notify_attack_target_id(mb);
  else if (aev_id == AEV_SYNC_MASTER_USE_SKILL)
    this->aev_sync_master_use_skill(mb);
  else if (aev_id == AEV_ZHUZAI_FEN_SHEN_TO_DESTROY)
    this->aev_zhuzai_fen_shen_to_destroy(mb);
  else
    super::dispatch_aev(aev_id, mb);
}
void zhuzai_fen_shen_mst::aev_fen_shen_info(mblock *mb)
{
  int total_hp = 0;
  int gong_ji = 0;
  int fang_yu = 0;
  int ming_zhong = 0;
  int shan_bi = 0;
  int bao_ji = 0;
  int kang_bao = 0;
  int shang_mian = 0;
  *mb >> this->master_id_
    >> this->pk_mode_
    >> this->team_id_
    >> this->guild_id_
    >> this->lvl_
    >> total_hp
    >> gong_ji
    >> fang_yu
    >> ming_zhong
    >> shan_bi
    >> bao_ji
    >> kang_bao
    >> shang_mian;

  this->obj_attr_.clear_value(ADD_T_BASIC);
  obj_attr::reset_attr_v_add();
  obj_attr::attr_v_add[ATTR_T_HP]              = total_hp;
  obj_attr::attr_v_add[ATTR_T_GONG_JI]         = gong_ji;
  obj_attr::attr_v_add[ATTR_T_FANG_YU]         = fang_yu;
  obj_attr::attr_v_add[ATTR_T_MING_ZHONG]      = ming_zhong;
  obj_attr::attr_v_add[ATTR_T_SHAN_BI]         = shan_bi;
  obj_attr::attr_v_add[ATTR_T_BAO_JI]          = bao_ji;
  obj_attr::attr_v_add[ATTR_T_KANG_BAO]        = kang_bao;
  obj_attr::attr_v_add[ATTR_T_SHANG_MIAN]      = shang_mian;
  this->obj_attr_.acc_v_attr(obj_attr::attr_v_add, ADD_T_BASIC);
  this->obj_attr_.hp_ = total_hp;
}
void zhuzai_fen_shen_mst::aev_notify_attack_target_id(mblock *mb)
{
  if (this->att_obj_id_ != 0) return ;
  this->att_obj_id_ = *((int *)mb->rd_ptr());
  this->to_attack();
}
void zhuzai_fen_shen_mst::aev_sync_master_use_skill(mblock *mb)
{
  time_value now = time_value::gettimeofday();

  int skill_cid = 0;
  short skill_lvl = 0;
  *mb >> skill_cid >> skill_lvl;
  const skill_cfg_obj *sco = skill_config::instance()->get_skill(skill_cid);
  if (sco == NULL) return ;
  const skill_detail *sd = sco->get_detail(skill_lvl);
  if (sd == NULL) return ;

  if (this->do_use_skill(this->att_obj_id_,
                         now,
                         this->coord_x_,
                         this->coord_y_,
                         skill_cid,
                         sco->hurt_delay_,
                         sd) == 0)
    this->on_use_skill_ok(NULL, sd, sco->hurt_delay_, now);
}
void zhuzai_fen_shen_mst::aev_zhuzai_fen_shen_to_destroy(mblock *)
{
  if (BIT_ENABLED(this->unit_status(), OBJ_DEAD))
    return ;
  this->to_destroy();
}
