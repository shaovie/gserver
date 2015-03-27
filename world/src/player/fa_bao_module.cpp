#include "fa_bao_module.h"
#include "istream.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "client.h"
#include "clsid.h"
#include "player_obj.h"
#include "package_module.h"
#include "fa_bao_config.h"
#include "behavior_id.h"
#include "time_util.h"
#include "global_param_cfg.h"
#include "all_char_info.h"
#include "scp_config.h"
#include "kai_fu_act_obj.h"

// Lib header

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("bao_shi");
static ilog_obj *e_log = err_log::instance()->get_ilog("bao_shi");

class fa_bao_attr
{
public:
  fa_bao_attr() :
    fa_bao_dj_(0),
    last_fabao_recovery_time_(time_util::now),
    hp_recover_(0),
    mp_recover_(0),
    add_hurt_(0),
    reduce_hurt_(0)
  { }

  char fa_bao_dj_;
  int last_fabao_recovery_time_;
  int hp_recover_;
  int mp_recover_;
  int add_hurt_;
  int reduce_hurt_;
};
void fa_bao_module::destroy(char_obj *co)
{
  delete co->fa_bao_attr_;
}
void fa_bao_module::parse_fa_bao_value(const char *fa_bao, int *fa_bao_info)
{
  char bf[64] = {0};
  ::strncpy(bf, fa_bao, sizeof(bf));
  char *tok_p = NULL;
  char *token = NULL;
  for (token = ::strtok_r(bf, ",", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, ",", &tok_p))
  {
    int p = ::atoi(token);
    if (p > 0 && p <= FA_BAO_PART_CNT)
      fa_bao_info[p] = 1;
  }
}
void fa_bao_module::set_part_value(player_obj *player, const int part)
{
  int fa_bao_info[FA_BAO_PART_CNT + 1] = {0};
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  fa_bao_module::parse_fa_bao_value(player->fa_bao(), fa_bao_info);
  fa_bao_info[part] = 1;

  player->do_save_fa_bao_info(fa_bao_info);
}
void player_obj::do_save_fa_bao_info(const int *fa_bao_info)
{
  ::memset(this->char_extra_info_->fa_bao_, 0, sizeof(this->char_extra_info_->fa_bao_));
  char wbf[FA_BAO_PART_CNT * 2] = {0};
  int len = 0;
  for (int i = 1; i <= FA_BAO_PART_CNT; ++i)
  {
    if (fa_bao_info[i] != 0)
      len += ::snprintf(wbf + len, sizeof(wbf) - len, "%d,", i);
  }
  if (len > 0)
  {
    wbf[len - 1] = '\0';
    ::strncpy(this->char_extra_info_->fa_bao_, wbf, sizeof(this->char_extra_info_->fa_bao_) - 1);
  }
  this->db_save_char_extra_info();
}
char fa_bao_module::fa_bao_dj(char_obj *co)
{
  if (co->fa_bao_attr_ == NULL) return 0;
  return co->fa_bao_attr_->fa_bao_dj_;
}
void player_obj::do_notify_fa_bao_info_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->fa_bao_dj();

  out_stream os(mb.wr_ptr(), mb.space());
  os << stream_ostr(this->char_extra_info_->fa_bao_, ::strlen(this->char_extra_info_->fa_bao_));
  mb.wr_ptr(os.length());
  this->do_delivery(NTF_FA_BAO_EQUIP_INFO, &mb);
}
int fa_bao_module::do_calc_hurt_for_attacker(char_obj *co, const int hurt)
{
  if (co->fa_bao_attr_ == NULL
      || co->fa_bao_attr_->add_hurt_ == 0)
    return hurt;
  int real_hurt = hurt + (int)::ceil((double)hurt * co->fa_bao_attr_->add_hurt_ / 10000.0);
  return real_hurt;
}
int fa_bao_module::do_calc_hurt_for_defender(char_obj *co, const int hurt)
{
  if (co->fa_bao_attr_ == NULL
      || co->fa_bao_attr_->reduce_hurt_ == 0)
    return hurt;
  int real_hurt = hurt - (int)::ceil((double)hurt * co->fa_bao_attr_->reduce_hurt_ / 10000.0);
  return real_hurt;
}
void fa_bao_module::on_enter_game(player_obj *player)
{
  fa_bao_module::do_build_fa_bao_attr(player, player->fa_bao_dj(), player->fa_bao());
}
void fa_bao_module::on_transfer_scene(player_obj *player, const int to_scene_cid)
{
  if (clsid::is_tui_tu_scp_scene(to_scene_cid)
      || (clsid::is_scp_scene(to_scene_cid)
          && scp_config::instance()->control_type(to_scene_cid) == scp_cfg_obj::SCP_CLT_IMPL)
     )
  {
    if (player->fa_bao_attr_ != NULL
        && (player->fa_bao_attr_->hp_recover_ > 0
            || player->fa_bao_attr_->mp_recover_ > 0)
       )
    {
      out_stream os(client::send_buf, client::send_buf_len);
      os << player->fa_bao_attr_->hp_recover_
        << player->fa_bao_attr_->mp_recover_;
      player->send_request(NTF_FA_BAO_RECOVERY_ATTR, &os);
    }
  }
}
void fa_bao_module::do_build_fa_bao_attr(char_obj *co,
                                         const char fa_bao_dj,
                                         const char *fa_bao)
{
  if (fa_bao_dj == 0
      && ::strlen(fa_bao) == 0)
    return ;
  if (co->fa_bao_attr_ == NULL)
    co->fa_bao_attr_ = new fa_bao_attr();
  co->fa_bao_attr_->last_fabao_recovery_time_ = time_util::now;
  co->fa_bao_attr_->fa_bao_dj_ = fa_bao_dj;

  fa_bao_dj_cfg_obj *fbdco = fa_bao_dj_cfg::instance()->get_dj_info(fa_bao_dj);
  if (fbdco == NULL) return ;

  co->fa_bao_attr_->hp_recover_ += fbdco->hp_recover_;
  co->fa_bao_attr_->mp_recover_ += fbdco->mp_recover_;
  co->fa_bao_attr_->add_hurt_ += fbdco->add_extra_hurt_;
  co->fa_bao_attr_->reduce_hurt_ += fbdco->reduce_hurt_;

  int fa_bao_info[FA_BAO_PART_CNT + 1] = {0};
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  fa_bao_module::parse_fa_bao_value(fa_bao, fa_bao_info);
  for (int i = 1; i <= FA_BAO_PART_CNT; ++i)
  {
    if (fa_bao_info[i] != 0)
    {
      fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao(fbdco->part_fa_bao_cid_[i]);
      if (fbco != NULL)
      {
        co->fa_bao_attr_->hp_recover_ += fbco->hp_recover_;
        co->fa_bao_attr_->mp_recover_ += fbco->mp_recover_;
        co->fa_bao_attr_->add_hurt_ += fbco->add_extra_hurt_;
        co->fa_bao_attr_->reduce_hurt_ += fbco->reduce_hurt_;
      }
    }
  }
}
void fa_bao_module::do_timeout(char_obj *co, const int now)
{
  if (co->fa_bao_attr_ == NULL
      || now - co->fa_bao_attr_->last_fabao_recovery_time_ < global_param_cfg::fabao_recovery_interval)
    return ;

  if (BIT_ENABLED(co->unit_status(), OBJ_DEAD)
      || clsid::is_tui_tu_scp_scene(co->scene_cid())
      || (clsid::is_scp_scene(co->scene_cid())
          && scp_config::instance()->control_type(co->scene_cid()) == scp_cfg_obj::SCP_CLT_IMPL)
     )
    return ;
  co->fa_bao_attr_->last_fabao_recovery_time_ = now;
  bool to_broadcast = false;
  if (co->fa_bao_attr_->hp_recover_ > 0
      && co->hp() < co->total_hp())
  {
    co->do_add_hp((int)::ceil((double)co->total_hp() * co->fa_bao_attr_->hp_recover_ / 10000.0));
    to_broadcast = true;
  }
  if (co->fa_bao_attr_->mp_recover_ > 0
      && co->mp() < co->total_mp())
  {
    co->do_add_mp((int)::ceil((double)co->total_mp() * co->fa_bao_attr_->mp_recover_ / 10000.0));
    to_broadcast = true;
  }
  if (to_broadcast)
    co->broadcast_hp_mp();
}
int fa_bao_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = fa_bao_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_TAKEUP_FA_BAO,          clt_takeup_fa_bao);
    SHORT_CODE(REQ_HE_CHENG_FA_BAO,        clt_he_cheng_fa_bao);
    SHORT_CODE(REQ_FA_BAO_JIN_JIE,         clt_fa_bao_jin_jie);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int fa_bao_module::clt_takeup_fa_bao(player_obj *player, const char *msg, const int len)
{
  int fa_bao_id = 0;
  in_stream is(msg, len);
  is >> fa_bao_id;

  item_obj *fa_bao_in_pkg = package_module::find_item(player, fa_bao_id);
  if (fa_bao_in_pkg == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_ITEM_NOT_EXIST);

  const int fa_bao_cid = fa_bao_in_pkg->cid_;
  const fa_bao_cfg_obj *fco = item_config::instance()->get_fa_bao(fa_bao_cid);
  if (fco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  fa_bao_dj_cfg_obj *fbdco = fa_bao_dj_cfg::instance()->get_dj_info(player->fa_bao_dj());
  if (fbdco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  bool b_find = false;
  for (int i = 1; i <= FA_BAO_PART_CNT; ++i)
  {
    if (fbdco->part_fa_bao_cid_[i] == fa_bao_cid)
      b_find = true;
  }
  if (!b_find)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (fa_bao_in_pkg->pkg_ != PKG_PACKAGE
      || fco->use_lvl_ > player->lvl())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int fa_bao_info[FA_BAO_PART_CNT + 1] = {0};
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  fa_bao_module::parse_fa_bao_value(player->fa_bao(), fa_bao_info);
  if (fa_bao_info[(int)fco->part_] != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  package_module::do_remove_item(player,
                                 fa_bao_in_pkg,
                                 fa_bao_in_pkg->amount_,
                                 ITEM_LOSE_FA_BAO_TAKEUP, 0);
  fa_bao_module::set_part_value(player, fco->part_);
  char_brief_info::on_char_fa_bao_update(player->id(), player->fa_bao_dj(), player->fa_bao());
  fa_bao_module::do_build_fa_bao_attr(player, player->fa_bao_dj(), player->fa_bao());

  player->do_calc_attr_affected_by_fa_bao();
  player->on_attr_update(ZHAN_LI_TAKEUP_FA_BAO, fa_bao_cid);

  player->do_notify_fa_bao_info_to_clt();
  return 0;
}
int fa_bao_module::clt_he_cheng_fa_bao(player_obj *player, const char *msg, const int len)
{
  int fa_bao_cid = 0;
  in_stream is(msg, len);
  is >> fa_bao_cid;

  const fa_bao_he_cheng_cfg_obj *p = fa_bao_he_cheng_cfg::instance()->get_he_cheng_info(fa_bao_cid);
  if (p == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (player->is_money_enough(M_COIN, p->cost_))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_COIN_NOT_ENOUGH);
  ilist_node<pair_t<int> > *cur_mat = p->material_.head();
  for (; cur_mat != NULL; cur_mat = cur_mat->next_)
  {
    if (package_module::calc_item_amount(player, cur_mat->value_.first_)
        < cur_mat->value_.second_)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAI_LIAO_NOT_ENOUGH);
  }

  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    fa_bao_cid,
                                    1,
                                    BIND_TYPE))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  cur_mat = p->material_.head();
  for (; cur_mat != NULL; cur_mat = cur_mat->next_)
  {
    package_module::do_remove_item(player,
                                   cur_mat->value_.first_,
                                   cur_mat->value_.second_,
                                   ITEM_LOSE_FA_BAO_HE_CHENG,
                                   fa_bao_cid);
  }
  player->do_lose_money(p->cost_, M_COIN, MONEY_LOSE_FA_BAO_HE_CHENG, 0, 0, 0);

  package_module::do_insert_item(player,
                                 PKG_PACKAGE,
                                 fa_bao_cid,
                                 1,
                                 BIND_TYPE,
                                 ITEM_GOT_FA_BAO_HE_CHENG,
                                 0,
                                 0);

  out_stream os(client::send_buf, client::send_buf_len);
  os << fa_bao_cid;
  return player->send_respond_ok(RES_HE_CHENG_FA_BAO, &os);
}
int fa_bao_module::clt_fa_bao_jin_jie(player_obj *player, const char *, const int )
{
  if (player->fa_bao_dj() >= FA_BAO_MAX_DJ)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int fa_bao_info[FA_BAO_PART_CNT + 1] = {0};
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  fa_bao_module::parse_fa_bao_value(player->fa_bao(), fa_bao_info);
  for (int i = 1; i <= FA_BAO_PART_CNT; ++i)
  {
    if (fa_bao_info[i] == 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }

  player->fa_bao_dj(player->fa_bao_dj() + 1);
  ::memset(fa_bao_info, 0, sizeof(fa_bao_info));
  player->do_save_fa_bao_info(fa_bao_info);
  char_brief_info::on_char_fa_bao_update(player->id(), player->fa_bao_dj(), player->fa_bao());
  fa_bao_module::do_build_fa_bao_attr(player, player->fa_bao_dj(), player->fa_bao());

  kai_fu_act_obj::on_fa_bao_dj_update(player, player->fa_bao_dj());

  player->do_calc_attr_affected_by_fa_bao();
  player->on_attr_update(ZHAN_LI_FA_BAO_JIN_JIE, 0);

  player->do_notify_fa_bao_info_to_clt();
  player->broadcast_fa_bao_info();
  return player->send_respond_ok(RES_FA_BAO_JIN_JIE, NULL);
}
void player_obj::broadcast_fa_bao_info()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id() << this->fa_bao_dj();
  ph->set(0, NTF_BROADCAST_FA_BAO_INFO_UPDATE, 0, mb.length());
  this->do_broadcast(&mb, false);
}
