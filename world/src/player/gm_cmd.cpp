#include "player_obj.h"
#include "gm_cmd.h"
#include "sys_log.h"
#include "def.h"
#include "util.h"
#include "clsid.h"
#include "istream.h"
#include "message.h"
#include "error.h"
#include "mblock_pool.h"
#include "item_config.h"
#include "item_obj.h"
#include "item_extra_info_opt.h"
#include "skill_config.h"
#include "skill_module.h"
#include "spawn_monster.h"
#include "package_module.h"
#include "behavior_id.h"
#include "task_module.h"
#include "scene_config.h"
#include "global_param_cfg.h"
#include "all_char_info.h"
#include "title_module.h"
#include "player_copy_mst.h"
#include "scp_module.h"
#include "equip_module.h"
#include "lvl_param_cfg.h"
#include "attr_module.h"

// Lib header
#include "json/json.h"

#define MAX_GM_CMD_PARAM_CNT    16

static ilog_obj *e_log = err_log::instance()->get_ilog("player");
static ilog_obj *s_log = sys_log::instance()->get_ilog("player");

int player_obj::clt_gm_cmd(const char *msg, const int len)
{
#ifdef PUBLISH
  return 0;
#endif
  char bf[256] = {0};
  stream_istr bf_si(bf, sizeof(bf));

  in_stream is(msg, len);
  is >> bf_si;

  //s_log->rinfo("%d gm cmd [%s]", this->id_, bf);

  char *params[MAX_GM_CMD_PARAM_CNT];
  int  number = 0;

  char *tok_p = NULL;
  char *token = NULL;
  for (token = ::strtok_r(bf, " ", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, " ", &tok_p))
  {
    params[number] = ::strdup(token);
    number++;
    if (number >= MAX_GM_CMD_PARAM_CNT)
      return this->send_respond_err(RES_GM_CMD, ERR_GM_PARAM_INVALID);
  }
  if (number == 0)
    return this->send_respond_err(RES_GM_CMD, ERR_GM_PARAM_INVALID);

  if (::strcmp(params[0], "addexp") == 0)
  {
    // @addexp number
    gm_cmd::add_exp(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "addskill") == 0)
  {
    // @addskill cid
    gm_cmd::add_skill(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "additem") == 0)
  {
    // @additem cid
    // @additem cid amount
    // @additem cid amount bind
    gm_cmd::add_item(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "addmoney") == 0)
  {
    // @addmoney value 
    // @addmoney value type
    gm_cmd::add_money(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "addmonster") == 0)
  {
    // @addmonster cid
    // @addmonster cid cnt
    // @addmonster cid cnt dir
    gm_cmd::add_monster(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "accepttask") == 0)
  {
    // @accepttask cid
    gm_cmd::accept_task(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "submittask") == 0)
  {
    // @submittask cid
    gm_cmd::submit_task(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "move") == 0)
  {
    // @move x y
    // @move scene_cid x y
    gm_cmd::move(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "pk") == 0)
  {
    // @pk
    gm_cmd::switch_pk(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "addsin") == 0)
  {
    // @addsin number
    gm_cmd::add_sin(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "addtitle") == 0)
  {
    // @addtitle title
    gm_cmd::add_title(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "copy") == 0)
  {
    // @copy
    gm_cmd::copy_self(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "clearscp") == 0)
  {
    // @clearscp
    gm_cmd::clear_scp(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "enterscp") == 0)
  {
    // @enterscp 3001
    gm_cmd::enter_scp(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "mpfull") == 0)
  {
    // @mpfull
    gm_cmd::mpfull(this, (const char **)params, number - 1);
  }else if (::strcmp(params[0], "gm") == 0)
  {
    // @gm 30
    gm_cmd::gm(this, (const char **)params, number - 1);
  }else
    return this->send_respond_err(RES_GM_CMD, ERR_GM_PARAM_INVALID);

  for (int i = 0; i < number; ++i)
    ::free(params[i]);
  return 0;
}
void gm_cmd::add_exp(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt != 1) return ;
  int exp = ::atoi(params[1]);
  if (exp <= 0 ) return ;
  mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
  *mb << exp;
  player->post_aev(AEV_GOT_EXP, mb);
  return ;
}
void gm_cmd::add_skill(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt != 1) return ;
  int skill_cid = ::atoi(params[1]);

  if (clsid::is_char_zhu_dong_skill(skill_cid))
  {
    const skill_cfg_obj *sco = skill_config::instance()->get_skill(skill_cid);
    if (sco == NULL || sco->career_ != player->career()) return ;

    const skill_detail* sd = skill_config::instance()->get_detail(skill_cid, 1);
    if (sd == NULL) return ;

    skill_map_itor itor = player->skill_map_.find(skill_cid);
    if (itor != player->skill_map_.end()) return ;

    skill_module::do_learn_skill(player, skill_cid);
  }
}
void gm_cmd::add_monster(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int monster_cid = ::atoi(params[1]);
  int monster_cnt = 1;
  int dir = DIR_XX;
  if (param_cnt == 2)
    monster_cnt = ::atoi(params[2]);
  if (param_cnt == 3)
    dir = ::atoi(params[3]);
  for (int i = 0; i < monster_cnt; ++i)
  {
    coord_t pos = scene_config::instance()->get_random_pos(player->scene_cid(),
                                                           player->coord_x(),
                                                           player->coord_y(),
                                                           4);
    if (spawn_monster::spawn_one(monster_cid,
                                 500,
                                 player->scene_id(),
                                 player->scene_cid(),
                                 dir,
                                 pos.x_,
                                 pos.y_) == -1)
    {
      e_log->wning("gm spawn mster %d %d failed!",
                   monster_cid, monster_cnt);
      break;
    }
  }
  return ;
}
void gm_cmd::add_item(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int item_cid = ::atoi(params[1]);
  if (!item_config::instance()->find(item_cid))
    return ;
#if 0
  if (item_config::instance()->get_equip(item_cid) != NULL)
    item_cid = clsid::get_equip_cid_by_career(player->career(), item_cid);
#endif

  int amount = 1;
  if (param_cnt >= 2)
    amount = ::atoi(params[2]);
  if (amount < 1) return ;

  char bind = UNBIND_TYPE;
  if (param_cnt == 3)
    bind = ::atoi(params[3]);
  if (bind != UNBIND_TYPE
      && bind != BIND_TYPE)
    return ;

  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    item_cid,
                                    amount,
                                    bind))
    return ;
  package_module::do_insert_item(player,
                                 PKG_PACKAGE,
                                 item_cid,
                                 amount,
                                 bind,
                                 0,
                                 0,
                                 0);
  return ;
}
void gm_cmd::add_money(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int money = ::atoi(params[1]);

  int type = M_DIAMOND;
  if (param_cnt == 2)
  {
    type = ::atoi(params[2]);
    if (player->is_money_upper_limit(type, money))
      return ;
    player->do_got_money(money, type, MONEY_GOT_GM, 0);
  }else
  {
    if (player->is_money_upper_limit(M_DIAMOND, money)
        || player->is_money_upper_limit(M_BIND_DIAMOND, money)
        || player->is_money_upper_limit(M_COIN, money))
      return ;
    player->do_got_money(money, M_DIAMOND, MONEY_GOT_GM, 0);
    player->do_got_money(money, M_BIND_DIAMOND, MONEY_GOT_GM, 0);
    player->do_got_money(money, M_COIN, MONEY_GOT_GM, 0);
  }

  return ;
}
void gm_cmd::accept_task(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int task_cid = ::atoi(params[1]);

  task_module::gm_add_task(player, task_cid);
  return ;
}
void gm_cmd::submit_task(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int task_cid = ::atoi(params[1]);

  task_module::gm_submit_task(player, task_cid);
  return ;
}
void gm_cmd::move(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 2) return ;
  int scene_cid = player->scene_cid();
  short x = 0;
  short y = 0;
  if (param_cnt == 3)
  {
    scene_cid = ::atoi(params[1]);
    if (!clsid::is_world_scene(scene_cid))
    {
      player->send_respond_err(RES_GM_CMD, ERR_GM_PARAM_INVALID);
      return ;
    }
    x = ::atoi(params[2]);
    y = ::atoi(params[3]);
  }else
  {
    x = ::atoi(params[1]);
    y = ::atoi(params[2]);
  }
  if (!scene_config::instance()->can_move(scene_cid,
                                          x,
                                          y))
    return ;
  player->do_transfer_to(scene_cid, scene_cid, x, y);
  return ;
}
void gm_cmd::switch_pk(player_obj *player, const char *[], const int )
{
  player->clt_change_pk_mode(NULL, 0);
  return ;
}
void gm_cmd::add_sin(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int value = ::atoi(params[1]);

  player->char_info_->sin_val_ += value;
  if (player->char_info_->sin_val_ > global_param_cfg::sin_val_upper_limit)
    player->char_info_->sin_val_ = global_param_cfg::sin_val_upper_limit;
  player->db_save_char_info();
  char_brief_info::on_char_sin_val_update(player->id(), player->char_info_->sin_val_);
  player->broadcast_sin_val();
  return ;
}
void gm_cmd::add_title(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int title_cid = ::atoi(params[1]);

  title_module::add_new_title(player, title_cid);
  return ;
}
void gm_cmd::copy_self(player_obj *player, const char *[], const int )
{
  coord_t pos = scene_config::instance()->get_random_pos(player->scene_cid(),
                                                         player->coord_x(),
                                                         player->coord_y(),
                                                         4);

  player_copy_mst *p = new player_copy_mst();
  if (p->init(player->id(),
              player->scene_id(),
              player->scene_cid(),
              player->dir(),
              pos.x_,
              pos.y_) != 0
      || p->do_activate(0) != 0)
  {
    e_log->rinfo("%d gm copy_self failed!", player->id());
    delete p;
    return ;
  }
  p->master_id(player->id());

  for (skill_map_itor itor = player->skill_map_.begin();
       itor != player->skill_map_.end();
       ++itor)
  {
    mblock *mb = mblock_pool::instance()->alloc(sizeof(int)*2);
    *mb << itor->second->cid_ << itor->second->lvl_;
    p->post_aev(AEV_PUSH_SKILL, mb);
  }
}
void gm_cmd::clear_scp(player_obj *player, const char *[], const int )
{
  scp_module::on_new_day(player, false);
  return ;
}
void gm_cmd::enter_scp(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int scp_cid = ::atoi(params[1]);

  scp_module::clt_enter_scp(player, (char *)&scp_cid, sizeof(int));
  return ;
}
void gm_cmd::mpfull(player_obj *player, const char *[], const int )
{
  if (player->mp() == player->total_mp()) return ;
  if (player->do_add_mp(player->total_mp()) != 0)
    player->broadcast_hp_mp();
  return ;
}
void gm_cmd::gm(player_obj *player, const char *params[], const int param_cnt)
{
  if (param_cnt < 1) return ;
  int gm_lvl = ::atoi(params[1]);

  if (gm_lvl < 1)
    gm_lvl = 1;
  //else if (gm_lvl > 40)
    //gm_lvl = 39;
  else if (gm_lvl >= global_param_cfg::lvl_limit)
    gm_lvl = global_param_cfg::lvl_limit - 1;

  while (player->lvl() < gm_lvl)
  {
    int64_t add_exp = lvl_param_cfg::instance()->lvl_up_exp(player->lvl());
    if (add_exp > 0)
      player->do_got_exp(add_exp);
  }

#if 0
  ilist<item_obj *> elist;
  package_module::find_all_item_in_pkg(player, PKG_EQUIP, elist);
  while (!elist.empty())
  {
    item_obj *io = elist.pop_front();
    package_module::do_remove_item(player,
                                   io,
                                   io->amount_,
                                   ITEM_LOSE_DESTROY,
                                   0);
  }
  const int equip_arr[] = {11140101, 11140201, 11140301, 11140401, 11140501, 11140601, 11140701, 11140801};
  for (size_t i = 0; i < sizeof(equip_arr)/sizeof(equip_arr[0]); ++i)
  {
    int item_cid = clsid::get_equip_cid_by_career(player->career(), equip_arr[i]);
    item_obj *io = package_module::alloc_new_item(player->id(),
                                                  item_cid,
                                                  1,
                                                  BIND_TYPE);
    Json::Value extra_json;
    item_extra_info_opt::decode_extra_info(io, extra_json);
    item_extra_info_opt::set_qh_lvl(io, gm_lvl, extra_json);
    package_module::do_insert_item(player,
                                   PKG_EQUIP,
                                   io,
                                   ITEM_GOT_PICKUP,
                                   0,
                                   0);
  }
#else

  for (int part = PART_ZHU_WU; part < PART_END; ++part)
  {
    item_obj *equip = package_module::find_item_by_part(player, part);
    if (equip == NULL) continue;

    Json::Value extra_json;
    item_extra_info_opt::decode_extra_info(equip, extra_json);
    item_extra_info_opt::set_qh_lvl(equip, gm_lvl, extra_json);

    int item_update_size = 0;
    g_item_update_info[item_update_size++] = package_module::UPD_EXTRA_INFO;
    package_module::on_item_update(player,
                                   UPD_ITEM,
                                   equip,
                                   g_item_update_info,
                                   item_update_size,
                                   0);
  }
#endif
  player->do_calc_attr_affected_by_equip();
  player->on_attr_update(0, 0);
  attr_module::on_all_qh_update(player);
}
