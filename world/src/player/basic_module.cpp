#include "player_obj.h"
#include "all_char_info.h"
#include "account_info.h"
#include "char_info.h"
#include "db_proxy.h"
#include "player_mgr.h"
#include "time_util.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "error.h"
#include "clsid.h"
#include "util.h"
#include "sys.h"
#include "sys_settings.h"
#include "fighter_mgr.h"
#include "scene_config.h"
#include "lvl_param_cfg.h"
#include "global_param_cfg.h"
#include "task_module.h"
#include "social_module.h"
#include "buff_module.h"
#include "skill_module.h"
#include "passive_skill_module.h"
#include "exit_transfer_cfg.h"
#include "package_module.h"
#include "shop_config.h"
#include "behavior_id.h"
#include "wild_boss_refresh.h"
#include "daily_clean_info.h"
#include "rank_module.h"
#include "cache_module.h"
#include "vip_module.h"
#include "tui_tu_module.h"
#include "cheng_jiu_module.h"
#include "player_copy_mst.h"
#include "monster_cfg.h"
#include "dropped_item.h"
#include "dropped_item_mgr.h"
#include "scp_module.h"
#include "jing_ji_module.h"
#include "activity_module.h"
#include "activity_cfg.h"
#include "activity_mgr.h"
#include "wild_boss_refresh.h"
#include "ltime_act_module.h"
#include "recharge_module.h"
#include "guild_module.h"
#include "mail_config.h"
#include "mobai_module.h"
#include "tianfu_skill_info.h"
#include "vip_config.h"
#include "ghz_module.h"
#include "xszc_activity_obj.h"
#include "lue_duo_module.h"
#include "fa_bao_module.h"
#include "mis_award_module.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

int player_obj::do_basic_module_msg(const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:       \
  ret = this->FUNC(msg, len);               \
  break
#define SHORT_DEFAULT default:              \
  e_log->error("unknow msg id %d", msg_id); \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_CHAR_MOVE, clt_move);
    SHORT_CODE(REQ_USE_SKILL, clt_use_skill);
    SHORT_CODE(REQ_USE_ITEM,  clt_use_item);
    SHORT_CODE(REQ_EXIT_TRANSFER, clt_exit_transfer);
    SHORT_CODE(REQ_RELIVE, clt_relive);
    SHORT_CODE(REQ_LOOK_OTHER_DETAIL_INFO, clt_look_other_detail_info);
    SHORT_CODE(REQ_REPLACE_EQUIP, clt_takeon_equip);
    SHORT_CODE(REQ_TAKEDOWN_EQUIP, clt_takedown_equip);
    SHORT_CODE(REQ_PICK_UP_ITEM, clt_pick_up_item);
    SHORT_CODE(REQ_WILD_BOSS_INFO, clt_wild_boss_info);
    SHORT_CODE(REQ_CHANGE_PK_MODE, clt_change_pk_mode);
    SHORT_CODE(REQ_BUY_ITEM, clt_buy_item);
    SHORT_CODE(REQ_BUY_AND_USE_ITEM, clt_buy_and_use_item);
    SHORT_CODE(REQ_DIAMOND_TO_COIN, clt_diamond_to_coin);
    SHORT_CODE(REQ_SELL_ITEM, clt_sell_item);
    SHORT_CODE(REQ_OBTAIN_MALL_BUY_LOG, clt_obtain_mall_buy_log);
    SHORT_CODE(REQ_OBTAIN_BAOWU_MALL_INFO, clt_obtain_baowu_mall_info);
    SHORT_CODE(REQ_REFRESH_BAOWU_MALL, clt_refresh_baowu_mall);
    SHORT_CODE(REQ_ZHAN_XING, clt_zhan_xing);
    SHORT_CODE(REQ_ZHAN_XING_TEN, clt_zhan_xing_ten);
    SHORT_CODE(REQ_MODIFY_SYS_SETTINGS, clt_modify_sys_settings);
    SHORT_CODE(REQ_GET_TITLE_LIST, clt_get_title_list);
    SHORT_CODE(REQ_USE_TITLE, clt_use_title);
    SHORT_CODE(REQ_CANCEL_TITLE, clt_cancel_cur_title);
    SHORT_CODE(REQ_GET_HUO_YUE_DU_LIST, clt_get_huo_yue_du_info);
    SHORT_CODE(REQ_GET_HUO_YUE_DU_AWARD, clt_get_huo_yue_du_award);
    SHORT_CODE(REQ_CLT_SYNC_HP_MP, clt_sync_hp_mp);
    SHORT_CODE(REQ_OBTAIN_SVC_TIME, clt_obtain_svc_time);
    SHORT_CODE(REQ_OBTAIN_WILD_BOSS_INFO, clt_obtain_wild_boss_info);
    SHORT_CODE(REQ_DIRECT_TRANSFER, clt_direct_transfer);
    SHORT_CODE(REQ_GET_TRADE_NO, clt_get_trade_no);
    SHORT_CODE(REQ_PRODUCE_ITEM, clt_produce_item);
    SHORT_CODE(REQ_ENTER_SCENE_OK, clt_enter_scene_ok);
    SHORT_CODE(REQ_BACK_TOWN, clt_back_town);
    SHORT_CODE(REQ_BUY_TI_LI, clt_buy_ti_li);
    SHORT_CODE(REQ_GET_FIRST_RECHARGE_AWARD, clt_get_first_recharge_award);
    SHORT_CODE(REQ_GOODS_AWARD_WEI_XIN_SHARED_OK, clt_goods_award_wei_xin_shared_ok);
    SHORT_CODE(REQ_GOODS_AWARD_LUCKY_TURN, clt_lucky_goods_turn);

    SHORT_CODE(REQ_GM_CMD, clt_gm_cmd);
    SHORT_CODE(REQ_TEST_ECHO, clt_test_echo);
    SHORT_DEFAULT;
  }
  return ret;
}
int player_obj::clt_heart_beat(const char *, const int )
{
  this->last_clt_heart_beat_time_ = time_util::now;

  out_stream os(client::send_buf, client::send_buf_len);
  os << time_util::now;
  return this->send_request(RES_CLT_HEART_BEAT, &os);
}
int player_obj::clt_move(const char *msg, const int len)
{
  char  dir = 0;
  short x   = 0;
  short y   = 0;
  in_stream is(msg, len);
  is >> dir >> x >> y;

  if (dir == this->dir_
      && x == this->coord_x_
      && y == this->coord_y_)
    return 0;

  int ret = this->can_move(x, y);
  if (ret == 0)
  {
    if (!scene_config::instance()->can_move(this->scene_cid_, x, y))
      ret = ERR_COORD_IS_CAN_NOT_MOVE;
    else if (!util::is_inside_of_redius(x, y, this->coord_x_, this->coord_y_, 2)
             || dir < DIR_UP
             || dir > DIR_LEFT_UP)
      ret = ERR_COORD_IS_ILLEGAL;
    else if (this->do_move(x, y, dir) != 0)
      ret = ERR_UNKNOWN;
  }

  if (ret != 0)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << this->dir_ << this->coord_x_ << this->coord_y_;
    return this->send_respond(RES_CHAR_MOVE, ret, &os);
  }

  this->broadcast_position();
  return 0;
}
void player_obj::broadcast_position()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_position_msg(mb);
  this->do_broadcast(&mb, false);
}
void player_obj::broadcast_speed()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_speed_msg(mb);
  this->do_broadcast(&mb, true);
}
void player_obj::broadcast_hp_mp()
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_hp_mp_msg(mb);
  this->do_broadcast(&mb, true);
}
void player_obj::broadcast_use_skill(const int skill_cid,
                                     const short skill_lvl,
                                     const int target_id,
                                     const short x,
                                     const short y)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_use_skill(mb,
                                     skill_cid,
                                     skill_lvl,
                                     target_id,
                                     x,
                                     y);
  this->do_broadcast(&mb, false);
}
void player_obj::broadcast_be_hurt_effect(const int skill_cid,
                                          const int hurt,
                                          const int tip,
                                          const int attacker_id)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_be_hurt_effect_msg(mb,
                                              skill_cid,
                                              hurt,
                                              tip,
                                              attacker_id);
  this->do_broadcast(&mb, true);
}
void player_obj::broadcast_tianfu_skill(const int skill_cid)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_tianfu_skill(mb, skill_cid);
  this->do_broadcast(&mb, true);
}
void player_obj::broadcast_equip_fino()
{
  int zhu_wu_cid = 0;
  int fu_wu_cid = 0;
  package_module::get_equip_for_view(this, zhu_wu_cid, fu_wu_cid);
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << zhu_wu_cid << fu_wu_cid;
  ph->set(0, NTF_BROADCAST_EQUIP_UPDATE, 0, mb.length());
  this->do_broadcast(&mb, true);
}
void player_obj::broadcast_sin_val()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_ << this->char_info_->sin_val_;
  ph->set(0, NTF_BROADCAST_SIN_VAL_UPDATE, 0, mb.length());
  this->do_broadcast(&mb, true);
}
int player_obj::clt_relive(const char *msg, const int len)
{
  if (BIT_DISABLED(this->unit_status_, OBJ_DEAD))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  char relive_mode = 0;
  in_stream is(msg, len);
  is >> relive_mode;

  int ret = this->do_relive(relive_mode);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int player_obj::do_relive(const char relive_mode)
{
  int relive_energy_per = 100;
  scene_coord_t to_relive_coord;
  int to_relive_scene_id = this->scene_id_;
  if (relive_mode == 1) // 复活点
  {
    to_relive_coord = xszc_activity_obj::get_relive_coord(this);
    if (to_relive_coord.cid_ == 0)
      to_relive_coord = scene_config::instance()->relive_coord(this->scene_cid_);
    if (!scene_config::instance()->can_move(to_relive_coord.cid_,
                                            to_relive_coord.coord_.x_,
                                            to_relive_coord.coord_.y_))
      return ERR_COORD_IS_ILLEGAL;
    relive_energy_per = global_param_cfg::relive_energy_p;
    if (!clsid::is_scp_scene(to_relive_coord.cid_)
        && !clsid::is_tui_tu_scp_scene(to_relive_coord.cid_))
      to_relive_scene_id = to_relive_coord.cid_;
  }else if (relive_mode == 3) // 回城
  {
    if (!scene_config::instance()->can_move(global_param_cfg::capital_relive_coord.cid_,
                                            global_param_cfg::capital_relive_coord.coord_.x_,
                                            global_param_cfg::capital_relive_coord.coord_.y_))
      return ERR_COORD_IS_ILLEGAL;
    relive_energy_per = global_param_cfg::relive_energy_p;
    to_relive_coord = global_param_cfg::capital_relive_coord;
    to_relive_scene_id = to_relive_coord.cid_;
  }else if (relive_mode == 2) // 原地
  {
    int cost = 0;
    if (vip_module::to_get_left_free_relive_cnt(this) > 0)
      vip_module::do_cost_free_relive(this);
    else if (this->free_relive_cnt() < global_param_cfg::free_relive_cnt)
    {
      this->free_relive_cnt(this->free_relive_cnt() + 1);
      this->db_save_daily_clean_info();
    }else // not free
    {
      if (package_module::calc_item_amount(this, global_param_cfg::relive_cid) == 0)
      {
        const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                                 global_param_cfg::relive_cid);
        if (go == NULL)
          return ERR_CONFIG_NOT_EXIST;
        cost = go->price_;
        int ret = this->is_money_enough(go->price_type_, cost);
        if (ret != 0)
          return ret;
      }else
        package_module::do_remove_item(this,
                                       global_param_cfg::relive_cid,
                                       1,
                                       ITEM_LOSE_USE,
                                       0);
    }
    if (cost > 0)
      this->do_lose_money(cost,
                          shop_config::instance()->price_type(global_param_cfg::mall_npc_cid,
                                                              global_param_cfg::relive_cid),
                          MONEY_LOSE_RELIVE,
                          this->scene_cid_,
                          0,
                          0);
    relive_energy_per = 100;
  }else
    return ERR_CLIENT_OPERATE_ILLEGAL;

  this->del_status(OBJ_DEAD);
  this->broadcast_unit_life_status(LIFE_V_ALIVE);

  this->do_add_hp(this->obj_attr_.total_hp() * relive_energy_per / 100);
  this->do_add_mp(this->obj_attr_.total_mp() * relive_energy_per / 100);
  this->broadcast_hp_mp();

  this->send_respond_ok(RES_RELIVE, NULL);
  if (to_relive_coord.cid_ != 0)
    this->do_transfer_to(to_relive_coord.cid_,
                         to_relive_scene_id,
                         to_relive_coord.coord_.x_,
                         to_relive_coord.coord_.y_);
  return 0;
}
void player_obj::do_resume_energy(const time_value &now)
{
  if (this->scene_cid_ != global_param_cfg::energy_resume_pos.cid_
      || BIT_ENABLED(this->unit_status_, OBJ_DEAD)
      || now < this->next_resume_energy_time_
      || (this->obj_attr_.total_hp() == this->obj_attr_.hp_
          && this->obj_attr_.total_mp() == this->obj_attr_.mp_)
      || util::is_out_of_distance(this->coord_x_,
                                  this->coord_y_,
                                  global_param_cfg::energy_resume_pos.coord_.x_,
                                  global_param_cfg::energy_resume_pos.coord_.y_,
                                  global_param_cfg::energy_resume_radius))
    return ;

  this->next_resume_energy_time_ = now + time_value(global_param_cfg::energy_resume_interval / 1000,
                                                    global_param_cfg::energy_resume_interval % 1000 * 1000);

  this->do_add_mp(this->obj_attr_.total_mp() * global_param_cfg::energy_resume_per / 100);
  this->do_add_hp(this->obj_attr_.total_hp() * global_param_cfg::energy_resume_per / 100);
  this->broadcast_hp_mp();
}
int player_obj::clt_test_echo(const char *msg, const int len)
{
  in_stream is(msg, len);
  out_stream os(client::send_buf, client::send_buf_len);
  os << is;
  this->send_respond_ok(RES_TEST_ECHO, &os);
  return 0;
}
void player_obj::broadcast_unit_life_status(const char st)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_unit_life_status(mb, st);
  this->do_broadcast(&mb, true);
}
int player_obj::clt_exit_transfer(const char *msg, const int len)
{
  int exit_id = 0;
  in_stream is(msg, len);
  is >> exit_id;

  scene_coord_t v = exit_transfer_cfg::instance()->get_target_coord(exit_id,
                                                                    this->scene_cid_);
  if (v.cid_ == 0)
    return this->send_respond_err(RES_EXIT_TRANSFER, ERR_CONFIG_NOT_EXIST);

  int ret = this->can_transfer_to(v.cid_);
  if (ret != 0)
    return this->send_respond_err(RES_EXIT_TRANSFER, ret);

  ret = this->do_transfer_to(v.cid_,
                             v.cid_,
                             v.coord_.x_,
                             v.coord_.y_);
  if (ret != 0)
    return this->send_respond_err(RES_EXIT_TRANSFER, ret);
  return 0;
}
int player_obj::clt_direct_transfer(const char *msg, const int len)
{
  int to_scene_cid = 0;
  short to_x = 0;
  short to_y = 0;
  in_stream is(msg, len);
  is >> to_scene_cid >> to_x >> to_y;

  if (!scene_config::instance()->can_move(to_scene_cid, to_x, to_y))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_COORD_IS_CAN_NOT_MOVE);

  int ret = this->can_transfer_to(to_scene_cid);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  ret = this->do_transfer_to(to_scene_cid,
                             to_scene_cid,
                             to_x,
                             to_y);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int player_obj::can_transfer_to(const int to_scene_cid)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CHAR_IS_DEAD;
  if (this->has_buff(BF_SK_DING_SHEN)
      || this->has_buff(BF_SK_XUAN_YUN))
    return ERR_CAN_NOT_TRANSFER;
  if (BIT_ENABLED(this->unit_status_, OBJ_FIGHTING))
    return ERR_CAN_NOT_TRANSFER_IN_FIGHTING;
  if (this->char_info_->lvl_ < scene_config::instance()->lvl_limit(to_scene_cid))
    return ERR_LVL_NOT_ENOUGH;
  return activity_module::can_transfer_to(this, to_scene_cid);
}
void player_obj::on_transfer_scene(const short old_x,
                                   const short old_y,
                                   const int old_scene_id,
                                   const int old_scene_cid)
{
  if (old_scene_cid != this->scene_cid_)
    this->enter_scene_ok_ = false;

  if (!clsid::is_scp_scene(old_scene_cid)
      && !clsid::is_tui_tu_scp_scene(old_scene_cid))
  {
    this->char_info_->last_coord_x_ = old_x;
    this->char_info_->last_coord_y_ = old_y;
    this->char_info_->last_scene_cid_ = old_scene_cid;
  }
  scp_module::on_transfer_scene(this,
                                old_scene_id,
                                old_scene_cid,
                                this->scene_id_,
                                this->scene_cid_);
  tui_tu_module::on_transfer_scene(this,
                                   old_scene_id,
                                   old_scene_cid,
                                   this->scene_id_,
                                   this->scene_cid_);
  activity_module::on_transfer_scene(this,
                                     old_scene_id,
                                     old_scene_cid,
                                     this->scene_id_,
                                     this->scene_cid_);
  mobai_module::on_transfer_scene(this,
                                  old_scene_cid,
                                  this->scene_cid_);
  fa_bao_module::on_transfer_scene(this, this->scene_cid_);
  this->db_save_char_info();
  this->do_clear_snap_units();
}
void player_obj::do_resume_last_position()
{
  this->scene_cid_  = this->char_info_->last_scene_cid_;
  this->scene_id_   = this->char_info_->last_scene_cid_;
  this->coord_x_    = this->char_info_->last_coord_x_;
  this->coord_y_    = this->char_info_->last_coord_y_;
}
void player_obj::do_reset_position(const int cid, const short x, const short y)
{
  this->scene_cid_  = cid;
  this->scene_id_   = cid;
  this->coord_x_    = x;
  this->coord_y_    = y;
}
int player_obj::do_got_exp(const int exp)
{
  if (exp <= 0) return 0;
  if (this->char_info_->lvl_ >= global_param_cfg::lvl_limit)
    return 0;

  int world_exp = 0;
  if (this->char_info_->lvl_ >= 30)
  {
    int diff = sys::world_lvl - this->char_info_->lvl_;
    if (diff >= 5 && diff <= 25)
      world_exp = exp * diff / 100;
  }
  int real_exp = world_exp + exp;

  short old_lvl = this->char_info_->lvl_;
  int64_t old_exp = this->char_info_->exp_;
  int64_t exp_up = lvl_param_cfg::instance()->lvl_up_exp(old_lvl);
  if (exp_up == -1)
  {
    e_log->wning("char %d get exp failed! lvl = %d", this->id_, old_lvl);
    return 0;
  }

  this->char_info_->exp_ += real_exp;
  while (this->char_info_->exp_ >= exp_up)
  {
    this->char_info_->exp_ -= exp_up;
    if (this->do_lvl_up() != 0) break;

    exp_up = lvl_param_cfg::instance()->lvl_up_exp(this->char_info_->lvl_);
    if (exp_up == -1) break;
  }

  this->db_save_char_info();

  rank_module::on_char_get_exp(this->id(), old_lvl, old_exp,
                               this->lvl(), this->char_info_->exp_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->char_info_->exp_ << real_exp << world_exp;
  this->send_request(NTF_UPDATE_EXP, &os);
  return 0;
}
int player_obj::do_lvl_up()
{
  if (this->char_info_->lvl_ >= global_param_cfg::lvl_limit)
    return -1;

  this->char_info_->lvl_ += 1;

  this->broadcast_lvl_up();

  this->do_calc_attr_affected_by_basic();
  this->do_add_hp(this->total_hp());
  this->do_add_mp(this->total_mp());
  this->on_attr_update(ZHAN_LI_LVLUP, this->char_info_->lvl_);

  this->do_add_ti_li((global_param_cfg::tili_val
                      + vip_module::to_get_tili_add_upper_limit(this)) * global_param_cfg::tili_recover / 100, true, true);

  char_brief_info::on_char_lvl_up(this->id_, this->char_info_->lvl_);
  task_module::on_char_lvl_up(this);
  social_module::on_char_lvl_up(this);
  skill_module::on_char_lvl_up(this);
  passive_skill_module::on_char_lvl_up(this);
  vip_module::on_char_lvl_up(this);
  cheng_jiu_module::on_char_lvl_up(this);
  jing_ji_module::on_char_lvl_up(this);
  activity_module::on_char_lvl_up(this);
  rank_module::update_world_lvl(this->char_info_->lvl_);
  mis_award_module::on_char_lvl_up(this);
  return 0;
}
void player_obj::broadcast_lvl_up()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id_
    << this->char_info_->lvl_;
  ph->set(0, NTF_BROADCAST_LVL_UP, 0, mb.length());
  this->do_broadcast(&mb, true);
}
int player_obj::do_dead(const int killer_id, const int killer_cid)
{
  this->del_status(OBJ_FIGHTING);
  this->add_status(OBJ_DEAD);

  this->broadcast_unit_life_status(LIFE_V_DEAD);

  int real_killer_id  = killer_id;
  int real_killer_cid = killer_cid;
  char_obj *killer = fighter_mgr::instance()->find(killer_id);
  if (killer != NULL)
    real_killer_id = killer->master_id();

  player_obj *real_killer = player_mgr::instance()->find(real_killer_id);
  if (real_killer != NULL)
  {
    real_killer_cid = real_killer_id;
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
    *mb << this->id();
    real_killer->post_aev(AEV_KILL_SOMEBODY, mb);
  }

  { // notify monster i dead
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
    *mb << this->id();
    this->do_post_aev_in_view_area(AEV_DEAD, mb, scene_unit::MONSTER);
  }

  {
    static ilist<item_amount_bind_t> drop_list;
    package_module::on_char_dead(this, drop_list);
    out_stream os(client::send_buf, client::send_buf_len);
    os << (real_killer_id == 0 ? killer_id : real_killer_id) << real_killer_cid
      << this->free_relive_cnt() << (char)drop_list.size();
    while (!drop_list.empty())
    {
      item_amount_bind_t v = drop_list.pop_front();
      os << v.cid_ << v.bind_ << v.amount_;
    }
    this->send_msg(NTF_CHAR_BE_KILLED, 0, &os);
  }

  buff_module::on_char_dead(this);
  scp_module::on_char_dead(this, real_killer_id);
  return 0;
}
void player_obj::on_kill_somebody(const int char_id)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;

  if (clsid::is_world_scene(this->scene_cid_)
      && (this->scene_cid_ != global_param_cfg::ghz_scene_cid
          || !activity_mgr::instance()->is_opened(ACTIVITY_GHZ)
         )
     )
  {
    // add sin
    if (player->sin_val() <= global_param_cfg::add_sin_val)
    {
      if (this->char_info_->sin_val_ < global_param_cfg::sin_val_upper_limit)
      {
        this->char_info_->sin_val_ += 1;
        this->db_save_char_info();
        char_brief_info::on_char_sin_val_update(this->id(), this->char_info_->sin_val_);
        this->broadcast_sin_val();
      }
    }

    // task
    task_module::on_kill_char(this);
    cheng_jiu_module::on_kill_char(this);
  }
  
  ghz_module::on_kill_somebody(this);
  scp_module::on_kill_somebody(this, char_id);
}
void player_obj::do_decrease_sin(const int val)
{
  this->char_info_->sin_val_ -= val;
  if (this->char_info_->sin_val_ < 0)
    this->char_info_->sin_val_ = 0;
  if (this->char_info_->sin_val_ == 0)
    this->char_info_->sin_ol_time_ = 0;
  this->db_save_char_info();
  char_brief_info::on_char_sin_val_update(this->id(), this->char_info_->sin_val_);
  this->broadcast_sin_val();
}
int player_obj::clt_look_other_detail_info(const char *msg, const int len)
{
  int target_id = 0;
  in_stream is(msg, len);
  is >> target_id;

  if (target_id == this->id_)
    return this->send_respond_err(NTF_OPERATE_RESULT,
                                  ERR_CLIENT_OPERATE_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  player_obj *player = player_mgr::instance()->find(target_id);
  if (player == NULL)
  {
    // or is play copy
    target_id = player_copy_mst::get_player_copy_id(target_id);
    if (all_char_info::instance()->get_char_brief_info(target_id) == NULL)
      return this->send_respond_err(NTF_OPERATE_RESULT,
                                    ERR_CLIENT_OPERATE_ILLEGAL);
    ilist<item_obj *> *elist = cache_module::get_player_equip(target_id);
    if (elist != NULL) // cache
      player_obj::do_build_look_detail_info(NULL, target_id, elist, os);
    else // miss
    {
      os << this->db_sid_ << target_id << (char)PKG_EQUIP;
      db_proxy::instance()->send_request(target_id, REQ_GET_ITEM_SINGLE_PKG, &os);
      return 0;
    }
  }else // online
  {
    static ilist<item_obj*> equip_list;
    equip_list.clear();
    package_module::find_all_item_in_pkg(player, PKG_EQUIP, equip_list);
    player_obj::do_build_look_detail_info(player, target_id, &equip_list, os);
  }
  return this->send_respond_ok(RES_LOOK_OTHER_DETAIL_INFO, &os);
}
void player_obj::handle_db_get_item_single_pkg_result(in_stream &is)
{
  int target_id = 0;
  int cnt = 0;
  is >> target_id >> cnt;

  ilist<item_obj*> *elist = new ilist<item_obj *>();
  for (int i = 0; i < cnt; ++i)
  {
    item_obj *io = package_module::alloc_new_item();
    is.rd_ptr(sizeof(short)); // Refer: db_proxy::proxy_obj.cpp::proc_result::for (mblock
    is >> io;
    elist->push_back(io);
  }
  cache_module::equip_cache_add_player(target_id, elist);

  out_stream os(client::send_buf, client::send_buf_len);
  elist = cache_module::get_player_equip(target_id);
  if (elist != NULL) // cache
    player_obj::do_build_look_detail_info(NULL, target_id, elist, os);
  this->send_respond_ok(RES_LOOK_OTHER_DETAIL_INFO, &os);
}
void player_obj::do_build_look_detail_info(player_obj *player,
                                           const int target_id,
                                           ilist<item_obj*> *list,
                                           out_stream &os)
{
  if (player != NULL)
  {
    os << player->id_
      << stream_ostr(player->char_info_->name_, ::strlen(player->char_info_->name_))
      << player->career()
      << player->lvl()
      << vip_module::vip_lvl(player);
    char *guild_name = guild_module::get_guild_name(player->guild_id_);
    if (guild_name == NULL)
      os << stream_ostr("", 0);
    else
      os << stream_ostr(guild_name, ::strlen(guild_name));

    os << (char)(list->size());
    ilist_node<item_obj*> *iter = list->head();
    for (; iter != NULL; iter = iter->next_)
      package_module::do_build_item_info(iter->value_, os);

    os << player->obj_attr_.total_hp()
      << player->obj_attr_.total_mp()
      << player->obj_attr_.gong_ji()
      << player->obj_attr_.fang_yu()
      << player->obj_attr_.ming_zhong()
      << player->obj_attr_.shan_bi()
      << player->obj_attr_.bao_ji()
      << player->obj_attr_.kang_bao()
      << player->obj_attr_.shang_mian()
      << player->char_info_->zhan_li_
      << player->char_info_->sin_val_
      << char(1); //在线

    os << (char)player->tianfu_skill_list_.size();
    ilist_node<tianfu_skill_info *> *t_iter = player->tianfu_skill_list_.head();
    for (; t_iter != NULL; t_iter = t_iter->next_)
      os << t_iter->value_->cid_ << t_iter->value_->lvl_;
  }else
  {
    char_brief_info *info = all_char_info::instance()->get_char_brief_info(target_id);
    if (info == NULL) return;

    os << target_id
      << stream_ostr(info->name_, ::strlen(info->name_))
      << info->career_
      << info->lvl_
      << info->vip_;
    int guild_id = guild_module::get_guild_id(target_id);
    char *guild_name = guild_module::get_guild_name(guild_id);
    if (guild_name == NULL)
      os << stream_ostr("", 0);
    else
      os << stream_ostr(guild_name, ::strlen(guild_name));

    os << (char)(list->size());
    ilist_node<item_obj*> *iter = list->head();
    for (; iter != NULL; iter = iter->next_)
      package_module::do_build_item_info(iter->value_, os);

    os << info->attr_[ATTR_T_HP]
      << info->attr_[ATTR_T_MP]
      << info->attr_[ATTR_T_GONG_JI]
      << info->attr_[ATTR_T_FANG_YU]
      << info->attr_[ATTR_T_MING_ZHONG]
      << info->attr_[ATTR_T_SHAN_BI]
      << info->attr_[ATTR_T_BAO_JI]
      << info->attr_[ATTR_T_KANG_BAO]
      << info->attr_[ATTR_T_SHANG_MIAN]
      << info->zhan_li_
      << info->sin_val_
      << char(0); //离线

    os << (char)info->tianfu_skill_list_.size();
    ilist_node<pair_t<int> > *t_iter = info->tianfu_skill_list_.head();
    for (; t_iter != NULL; t_iter = t_iter->next_)
      os << t_iter->value_.first_ << (short)t_iter->value_.second_;
  }
}
int player_obj::do_transfer_to(const int target_scene_cid,
                               const int target_scene_id,
                               const short target_x,
                               const short target_y)
{
  this->do_exit_scene();

  int old_scene_cid = this->scene_cid_;
  int old_scene_id  = this->scene_id_;
  short old_x       = this->coord_x_;
  short old_y       = this->coord_y_;

  this->scene_cid_  = target_scene_cid;
  this->scene_id_   = target_scene_id;
  this->coord_x_    = target_x;
  this->coord_y_    = target_y;

  if (this->do_enter_scene() != 0)
  {
    this->scene_cid_  = old_scene_cid;
    this->scene_id_   = old_scene_id;
    this->coord_x_    = old_x;
    this->coord_y_    = old_y;
    this->do_enter_scene();
    return ERR_UNKNOWN;
  }
  s_log->rinfo("char %s:%d from %d transfer to %d:%d ok!",
               this->account(),
               this->id_,
               old_scene_cid,
               target_scene_cid,
               target_scene_id);

  this->on_transfer_scene(old_x, old_y, old_scene_id, old_scene_cid);

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->scene_cid_ << this->coord_x_ << this->coord_y_;
  this->send_request(NTF_TRANSFER_SCENE, &os);
  return 0;
}
int player_obj::clt_wild_boss_info(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  wild_boss_refresh::instance()->fetch_wild_boss_info(os);
  return this->send_respond_ok(RES_WILD_BOSS_INFO, &os);;
}
int player_obj::clt_modify_sys_settings(const char *msg, const int len)
{
  sys_settings ss;
  ss.char_id_ = this->id();
  in_stream is(msg, len);
  stream_istr ss_si(ss.data_, sizeof(ss.data_));
  is >> ss_si;
  if (ss_si.str_len() == 0)
  {
    this->send_respond_err(NTF_OPERATE_RESULT, ERR_INPUT_IS_ILLEGAL);
    e_log->rinfo("system settings too long!");
    return 0;
  }

  if (!util::verify_db(ss.data_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_INPUT_IS_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << &ss;
  db_proxy::instance()->send_request(this->id(), REQ_UPDATE_SYS_SETTINGS, &os);
  return 0;
}
void player_obj::handle_db_get_sys_settings_result(sys_settings *ss)
{
  out_stream os(client::send_buf, client::send_buf_len);
  if (ss == NULL)
  {
    static const char default_ss[] = "system_settings={}";
    os << stream_ostr(default_ss, sizeof(default_ss) - 1);
  }else
    os << stream_ostr(ss->data_, ::strlen(ss->data_));
  this->send_request(NTF_PUSH_SYS_SETTINGS, &os);
}
int player_obj::clt_change_pk_mode(const char *, const int )
{
  if (this->char_info_->lvl_ < global_param_cfg::pk_lvl_limit)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CAN_NOT_CHANGE_PK_MODE);

  char pk_v = PK_MODE_PEACE;
  if (this->char_info_->pk_mode_ == PK_MODE_PEACE)
    pk_v = PK_MODE_FIGHT;

  this->char_info_->pk_mode_ = pk_v;
  this->db_save_char_info();
  this->do_notify_pk_mode_to_clt();
  return 0;
}
void player_obj::do_notify_pk_mode_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->char_info_->pk_mode_;
  this->do_delivery(NTF_CHANGE_PK_MODE, &mb);
}
void player_obj::do_timing_add_ti_li_value(const int now, const bool when_login)
{
  if (now - this->char_extra_info_->ti_li_time_ < global_param_cfg::tili_resume_interval)
    return;

  int cnt =
    (now - this->char_extra_info_->ti_li_time_) / global_param_cfg::tili_resume_interval;
  int left_time =
    (now - this->char_extra_info_->ti_li_time_) % global_param_cfg::tili_resume_interval;

  int add_ti_li = cnt * global_param_cfg::tili_resume_val;

  this->char_extra_info_->ti_li_time_ = now - left_time;

  this->do_add_ti_li(add_ti_li, false, !when_login);
}
void player_obj::do_add_ti_li(const short value, const bool can_overflow, const bool notify_to_clt)
{
  if (value <= 0) return ;
  int tili_upper_limit = global_param_cfg::tili_val + vip_module::to_get_tili_add_upper_limit(this);
  if (!can_overflow
      && this->char_extra_info_->ti_li_ >= tili_upper_limit)
    return ;
  this->char_extra_info_->ti_li_ += value;
  if (!can_overflow
      && this->char_extra_info_->ti_li_ >= tili_upper_limit)
    this->char_extra_info_->ti_li_ = tili_upper_limit;

  if (this->char_extra_info_->ti_li_ > 20000)
    this->char_extra_info_->ti_li_ = 20000;

  this->db_save_char_extra_info();

  if (notify_to_clt)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << this->char_extra_info_->ti_li_;
    this->send_request(NTF_UPDATE_TI_LI, &os);
  }
}
void player_obj::do_reduce_ti_li(const short value)
{
  if (value <= 0) return ;
  this->char_extra_info_->ti_li_ -= value;
  if (this->char_extra_info_->ti_li_ < 0)
    this->char_extra_info_->ti_li_ = 0;

  this->db_save_char_extra_info();

  out_stream os(client::send_buf, client::send_buf_len);
  os << this->char_extra_info_->ti_li_;
  this->send_request(NTF_UPDATE_TI_LI, &os);
}
void player_obj::do_timing_add_jing_li_value(const int now, const bool when_login)
{
  if (now - this->char_extra_info_->jing_li_time_ < global_param_cfg::jingli_resume_interval)
    return;

  int jingli_upper_limit = global_param_cfg::jingli_value + vip_module::to_get_jingli_add_upper_limit(this);
  if (this->char_extra_info_->jing_li_ >= jingli_upper_limit)
    return ;

  int cnt =
    (now - this->char_extra_info_->jing_li_time_) / global_param_cfg::jingli_resume_interval;

  this->char_extra_info_->jing_li_time_ += cnt * global_param_cfg::jingli_resume_interval;

  int add_jing_li = cnt * global_param_cfg::jingli_resume_value;
  if (this->char_extra_info_->jing_li_ + add_jing_li >= jingli_upper_limit)
    this->char_extra_info_->jing_li_ = jingli_upper_limit;
  else
    this->char_extra_info_->jing_li_ += add_jing_li;

  this->db_save_char_extra_info();

  if (!when_login)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << this->char_extra_info_->jing_li_;
    this->send_request(NTF_UPDATE_JING_LI, &os);
  }
}
void player_obj::on_clt_kill_mst(const int mst_cid,
                                 const short x,
                                 const short y,
                                 const int msg_id,
                                 bool &got_award)
{
  monster_cfg_obj *mco = monster_cfg::instance()->get_monster_cfg_obj(mst_cid);
  if (mco == NULL) return ;

  int award_exp = ltime_act_module::on_kill_mst_got_exp(mco->exp_);
  if (award_exp > 0)
  {
    if (clsid::is_common_scp_scene(this->scene_cid()))
    {
      this->scp_award_exp_ += award_exp;
      got_award = true;
    }else
    {
      mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
      *mb << award_exp;
      this->post_aev(AEV_GOT_EXP, mb);
    }
  }

  dropped_item::do_clear_dropped_item_list();
  int drop_times = ltime_act_module::on_drop_items();
  for (int i = 0; i < drop_times; ++i)
  {
    dropped_item::do_build_drop_item_list(this->id_,
                                          0,
                                          mst_cid,
                                          dropped_item::dropped_item_list);
  }

  if (clsid::is_common_scp_scene(this->scene_cid()))
  {
    while (!dropped_item::dropped_item_list.empty())
    {
      dropped_item *di = dropped_item::dropped_item_list.pop_front();
      ilist_node<item_amount_bind_t> *itor = this->scp_award_items_.head();
      for (; itor != NULL; itor = itor->next_)
      {
        if (itor->value_.cid_ == di->cid()
            && itor->value_.bind_ == di->bind_type_)
        {
          itor->value_.amount_ += di->amount_;
          got_award = true;
          break;
        }
      }
      if (itor == NULL)
      {
        got_award = true;
        this->scp_award_items_.push_back(item_amount_bind_t(di->cid(), di->amount_, di->bind_type_));
      }
      dropped_item_pool::instance()->release(di);
    }
  }else
  {
    tui_tu_module::on_kill_mst(this, mst_cid, dropped_item::dropped_item_list); //

    out_stream os(client::send_buf, client::send_buf_len);
    os << mst_cid << x << y;
    char *count = (char *)os.wr_ptr();
    os << (char)0;
    while (!dropped_item::dropped_item_list.empty())
    {
      dropped_item *di = dropped_item::dropped_item_list.pop_front();
      di->reset_pos(this->scene_id_,
                    this->scene_cid_,
                    x,
                    y);
      dropped_item_mgr::instance()->insert(di->id(), di);

      os << di->cid() << di->id();
      ++(*count);
    }
    if (*count > 0)
      this->send_request(msg_id, &os);
  }
}
int player_obj::clt_sync_hp_mp(const char *msg, const int len)
{
  if (!clsid::is_tui_tu_scp_scene(this->scene_cid_)
      && !clsid::is_scp_scene(this->scene_cid_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  in_stream is(msg, len);
  int hp = 0;
  int mp = 0;
  is >> hp >> mp;
  if (BIT_DISABLED(this->unit_status(), OBJ_DEAD))
  {
    int hp_v = hp - this->hp();
    if (hp > 0 && hp_v > 0)
      this->do_add_hp(util::min(this->total_hp(), hp_v));
    else if (hp >= 0 && hp_v < 0)
      this->do_reduce_hp(util::min(this->total_hp(), -hp_v));

    int mp_v = mp - this->mp();
    if (mp > 0 && mp_v > 0)
      this->do_add_mp(util::min(this->total_mp(), mp_v));
    else if (mp >= 0 && mp_v < 0)
      this->do_reduce_mp(util::min(this->total_mp(), -mp_v));
    this->broadcast_hp_mp();
  }
  if (this->obj_attr_.hp_ <= 0)
    this->add_status(OBJ_DEAD);
  return 0;
}
int player_obj::clt_obtain_svc_time(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << time_util::now;
  return this->send_respond_ok(RES_OBTAIN_SVC_TIME, &os);
}
int player_obj::clt_obtain_wild_boss_info(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  wild_boss_refresh::instance()->fetch_wild_boss_info(os);
  return this->send_respond_ok(RES_OBTAIN_WILD_BOSS_INFO, &os);
}
int player_obj::clt_enter_scene_ok(const char *, const int )
{
  if (this->enter_scene_ok_)
    e_log->rinfo("%s:%d, enter scene %d repeat!", this->name(), this->id(), this->scene_cid_);
  this->enter_scene_ok_ = true;
  jing_ji_module::on_enter_scene(this);
  lue_duo_module::on_enter_scene(this);
  return 0;
}
void player_obj::do_exchange_activation_code(const char *content,
                                             const int coin,
                                             const int b_diamond,
                                             ilist<item_amount_bind_t> &item_list)
{
  this->do_got_money(coin,
                     M_COIN,
                     MONEY_GOT_ACTIVATION_CODE,
                     0);
  this->do_got_money(b_diamond,
                     M_BIND_DIAMOND,
                     MONEY_GOT_ACTIVATION_CODE,
                     0);
  for (ilist_node<item_amount_bind_t> *itor = item_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    package_module::do_insert_or_mail_item(this,
                                           itor->value_.cid_,
                                           itor->value_.amount_,
                                           itor->value_.bind_,
                                           mail_config::PACKAGE_IS_FULL,
                                           MONEY_GOT_ACTIVATION_CODE,
                                           ITEM_GOT_ACTIVATION_CODE,
                                           0,
                                           0);
  }
  out_stream os(client::send_buf, client::send_buf_len);
  os << stream_ostr(content, ::strlen(content))
    << b_diamond
    << coin
    << (char)item_list.size();
  for (ilist_node<item_amount_bind_t> *itor = item_list.head();
       itor != NULL;
       itor = itor->next_)
    os << itor->value_.cid_ << itor->value_.amount_;

  this->send_request(NTF_EXCHANGE_ACTIVATION_CODE_AWARD, &os);
}
int player_obj::clt_back_town(const char *, const int )
{
  if (!scene_config::instance()->can_move(global_param_cfg::capital_relive_coord.cid_,
                                          global_param_cfg::capital_relive_coord.coord_.x_,
                                          global_param_cfg::capital_relive_coord.coord_.y_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_COORD_IS_ILLEGAL);
  int ret = this->can_transfer_to(global_param_cfg::capital_relive_coord.cid_);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  coord_t random_pos = scene_config::instance()->get_random_pos(global_param_cfg::capital_relive_coord.cid_,
                                                                global_param_cfg::capital_relive_coord.coord_.x_,
                                                                global_param_cfg::capital_relive_coord.coord_.y_,
                                                                4);

  int cost = 0;
  if (package_module::calc_item_amount(this, global_param_cfg::back_town_cid) == 0)
  {
    const goods_obj *go = shop_config::instance()->get_goods(global_param_cfg::mall_npc_cid,
                                                             global_param_cfg::back_town_cid);
    if (go == NULL)
      return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
    cost = go->price_;
    ret = this->is_money_enough(M_BIND_UNBIND_DIAMOND, cost);
    if (ret != 0)
      return this->send_respond_err(NTF_OPERATE_RESULT, ret);
  }else
    package_module::do_remove_item(this,
                                   global_param_cfg::back_town_cid,
                                   1,
                                   ITEM_LOSE_USE,
                                   0);
  if (cost > 0)
    this->do_lose_money(cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_BACK_TOWN,
                        0,
                        0,
                        0);
  this->do_transfer_to(global_param_cfg::capital_relive_coord.cid_,
                       global_param_cfg::capital_relive_coord.cid_,
                       random_pos.x_,
                       random_pos.y_);

  return 0;
}
int player_obj::clt_buy_ti_li(const char *, const int )
{
  if (vip_module::vip_lvl(this) == 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (vip_module::to_get_left_buy_ti_li_cnt(this) <= 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int buy_amt = 0;
  int cost = 0;
  if (vip_purchase_cfg::instance()->ti_li(vip_module::to_get_had_buy_ti_li_cnt(this) + 1,
                                          cost,
                                          buy_amt) != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);
  const int ret = this->is_money_enough(M_BIND_UNBIND_DIAMOND, cost);
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  this->do_lose_money(cost,
                      M_BIND_UNBIND_DIAMOND,
                      MONEY_LOSE_BUY_TI_LI,
                      0, 0, 0);

  vip_module::do_cost_buy_ti_li_cnt(this);
  this->do_add_ti_li(buy_amt, true, true);

  return 0;
}
void player_obj::do_notify_xszc_honor_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->char_extra_info_->zx_value_;
  this->do_delivery(NTF_XSZC_HONOR_VALUE, &mb);
}
