#include "ghz_module.h"
#include "behavior_log.h"
#include "all_char_info.h"
#include "scene_config.h"
#include "global_param_cfg.h"
#include "activity_module.h"
#include "ghz_activity_obj.h"
#include "notice_module.h"
#include "player_mgr.h"
#include "player_obj.h"
#include "activity_mgr.h"
#include "activity_obj.h"
#include "activity_cfg.h"
#include "guild_module.h"
#include "spawn_monster.h"
#include "package_module.h"
#include "rank_module.h"
#include "mail_info.h"
#include "mail_config.h"
#include "mail_module.h"
#include "time_util.h"
#include "client.h"
#include "istream.h"
#include "sys_log.h"
#include "service_info.h"
#include "sys.h"

// Lib header
#include <map>
#include "mblock.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("ghz");
static ilog_obj *e_log = err_log::instance()->get_ilog("ghz");

static coord_t s_attacker_enter_pos;
static coord_t s_defender_enter_pos;

typedef std::map<int/*char_id*/, int/*cnt*/> kill_map_t;
typedef std::map<int/*char_id*/, int/*cnt*/>::iterator kill_map_itor;
static kill_map_t s_kill_map;

int ghz_module::parse_cfg_param(const char *param)
{
  if (::sscanf(param, "%hd,%hd:%hd,%hd",
               &s_attacker_enter_pos.x_,
               &s_attacker_enter_pos.y_,
               &s_defender_enter_pos.x_,
               &s_defender_enter_pos.y_) != 4)
    return -1;
  if (!scene_config::instance()->can_move(global_param_cfg::ghz_scene_cid,
                                          s_attacker_enter_pos.x_,
                                          s_attacker_enter_pos.y_)
      || !scene_config::instance()->can_move(global_param_cfg::ghz_scene_cid,
                                             s_defender_enter_pos.x_,
                                             s_defender_enter_pos.y_))
  {
    e_log->rinfo("ghz enter pos can not move!");
    return -1;
  }
  return 0;
}
void ghz_module::on_ghz_open()
{
  s_kill_map.clear();
}
void ghz_module::on_char_login(player_obj *player)
{
  if (player->scene_cid() == global_param_cfg::ghz_scene_cid
      && activity_mgr::instance()->is_opened(ACTIVITY_GHZ))
  {
    scene_coord_t to_relive_coord;
    to_relive_coord = scene_config::instance()->relive_coord(global_param_cfg::ghz_scene_cid);
    if (to_relive_coord.cid_ != 0)
    {
      player->do_reset_position(to_relive_coord.cid_,
                                to_relive_coord.coord_.x_,
                                to_relive_coord.coord_.y_);
    }
  }
}
void ghz_module::on_enter_game(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() < activity_cfg::instance()->open_lvl(ACTIVITY_GHZ))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void ghz_module::on_char_lvl_up(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() != activity_cfg::instance()->open_lvl(ACTIVITY_GHZ))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void ghz_module::on_transfer_scene(player_obj *player, const int to_scene_cid)
{
  if (to_scene_cid == global_param_cfg::ghz_scene_cid)
  {
    activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
    if (ao != NULL && ao->is_opened())
    {
      behavior_log::instance()->store(BL_JOIN_ACTVITY,
                                      time_util::now,
                                      "%d|%d",
                                      player->id(), ACTIVITY_GHZ);

      mblock mb(client::send_buf, client::send_buf_len);
      mb.wr_ptr(sizeof(proto_head));
      mb << ao->left_time();
      player->do_delivery(NTF_GHZ_ACT_LEFT_TIME, &mb);
    }
  }
}
void ghz_module::on_kill_somebody(player_obj *player)
{
  if (player->scene_cid() == global_param_cfg::ghz_scene_cid
      && activity_mgr::instance()->is_opened(ACTIVITY_GHZ))
  {
    kill_map_itor itor = s_kill_map.find(player->id());
    if (itor == s_kill_map.end())
      s_kill_map.insert(std::make_pair(player->id(), 1));
    else
      itor->second += 1;
  }
}
void ghz_module::on_wang_zuo_activated(const int id)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL) return ;
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_GHZ_WANG_ZUO_ACTIVATED);
  mb << id;
  ao->do_something(&mb, NULL, NULL);
}
void ghz_module::on_wang_zuo_dead(const int killer_id)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL) return ;
  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_GHZ_WANG_ZUO_DEAD);
  mb << killer_id;
  ao->do_something(&mb, NULL, NULL);
}
void ghz_module::on_shou_wei_dead(const int shou_wei_id,
                                  const int shou_wei_cid,
                                  const int killer_id)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL) return ;

  //
  {
    char bf[8] = {0};
    mblock mb(bf, sizeof(bf));
    mb.data_type(ACT_EV_GHZ_SHOU_WEI_DEAD);
    mb << shou_wei_id << shou_wei_cid;
    ao->do_something(&mb, NULL, NULL);
  }

  player_obj *player = player_mgr::instance()->find(killer_id);
  if (player != NULL
      && player->guild_id() != 0)
    notice_module::ghz_shou_wei_dead(guild_module::get_guild_name(player->guild_id()),
                                     player->id(),
                                     player->name(),
                                     ghz_activity_obj::get_shou_wei_idx(shou_wei_cid));
}
void ghz_module::on_shou_wei_live(player_obj *player, const int id, const int cid)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL) return ;

  char bf[8] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_GHZ_SHOU_WEI_LIVE);
  mb << id << cid;
  ao->do_something(&mb, NULL, NULL);

  notice_module::ghz_shou_wei_live(guild_module::get_guild_name(player->guild_id()),
                                   player->id(),
                                   player->name(),
                                   ghz_activity_obj::get_shou_wei_idx(cid));
}
void ghz_module::on_ghz_over(const int win_guild_id, const int killer_id)
{
  int last_winner = guild_module::get_ghz_winner();
  if (win_guild_id != 0
      || last_winner != 0)
  {
    if (win_guild_id == 0)
      guild_module::on_ghz_over(last_winner);
    else
      guild_module::on_ghz_over(win_guild_id);

    // notice
    if (last_winner != win_guild_id)
    {
      player_obj *player = player_mgr::instance()->find(killer_id);
      if (player != NULL)
        notice_module::ghz_over_win(guild_module::get_guild_name(win_guild_id),
                                    player->id(),
                                    player->name());
    }else if (win_guild_id == 0)
      notice_module::ghz_over_wei_mian(guild_module::get_guild_name(last_winner));
  }

  sys::svc_info->ghz_closed_time = time_util::now;
  sys::update_svc_info();

  ghz_module::do_give_winner_award(win_guild_id);
}
void ghz_module::do_give_winner_award(const int /*win_guild_id*/)
{
  player_mgr::instance()->post_aev_to_scene(global_param_cfg::ghz_scene_cid,
                                            AEV_GHZ_OVER_AWARD,
                                            NULL);
}
void player_obj::aev_ghz_over_award(mblock *)
{
  if (this->guild_id() == 0) return ;

  int winner_char_id = guild_module::get_ghz_winner_chairman();
  int winner_guild_id = guild_module::get_ghz_winner();
  const char *winner_guild_name = guild_module::get_guild_name(winner_guild_id);
  int diamond = global_param_cfg::ghz_over_award[0];
  int coin = global_param_cfg::ghz_over_award[1];
  int contrib = global_param_cfg::ghz_over_award[2];
  int gift_cid = global_param_cfg::ghz_over_award[3];
  int mail_id = mail_config::GHZ_OVER_AWARD;
  if (this->guild_id() == winner_guild_id)
  {
    diamond = global_param_cfg::ghz_winner_award[0];
    coin = global_param_cfg::ghz_winner_award[1];
    contrib = global_param_cfg::ghz_winner_award[2];
    gift_cid = global_param_cfg::ghz_winner_award[3];
    mail_id = mail_config::GHZ_WINNER_AWARD;
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << (char)(this->guild_id() == winner_guild_id ? 1 : 0)
    << stream_ostr(winner_guild_name, ::strlen(winner_guild_name));
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(winner_char_id);
  if (cbi == NULL)
    os << stream_ostr("null", 4);
  else
    os << stream_ostr(cbi->name_, ::strlen(cbi->name_));
  os << s_kill_map[this->id()];
  this->send_respond_ok(NTF_GHZ_OVER, &os);

  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_id);
  if (mo == NULL) return ;
  
  item_obj *io = package_module::alloc_new_item(this->id_,
                                                gift_cid,
                                                1,
                                                BIND_TYPE);
  item_obj il[MAX_ATTACH_NUM];
  il[0] = *io;
  package_module::release_item(io);

  mail_module::do_send_mail(this->id_, mail_info::MAIL_SEND_SYSTEM_ID,
                            mo->sender_name_, mo->title_, mo->content_,
                            mail_info::MAIL_TYPE_GUILD,
                            coin, 0, diamond,
                            1, il,
                            this->db_sid_, time_util::now);
  mail_module::do_notify_haved_new_mail(this, 1);
  guild_module::do_got_contrib(this->guild_id(), this->id_, contrib);
}
int ghz_module::do_enter_ghz(player_obj *player)
{
  if (player->scene_id() != player->scene_cid()) // in scp
    return ERR_CAN_NOT_TRANSFER;

  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL
      || !ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  if (player->scene_cid() == global_param_cfg::ghz_scene_cid)
    return 0;

  int ret = player->can_transfer_to(global_param_cfg::ghz_scene_cid);
  if (ret != 0) return ret;

  ret = ao->can_enter(player);
  if (ret != 0) return ret;

  if (player->guild_id() == guild_module::get_ghz_winner())
  {
    ret = player->do_transfer_to(global_param_cfg::ghz_scene_cid,
                                 global_param_cfg::ghz_scene_cid,
                                 s_defender_enter_pos.x_,
                                 s_defender_enter_pos.y_);
    if (ret != 0) return ret;
  }else
  {
    ret = player->do_transfer_to(global_param_cfg::ghz_scene_cid,
                                 global_param_cfg::ghz_scene_cid,
                                 s_attacker_enter_pos.x_,
                                 s_attacker_enter_pos.y_);
    if (ret != 0) return ret;
  }

  return 0;
}
int ghz_module::do_activate_shou_wei(player_obj *player, const int cid)
{
  if (ghz_activity_obj::get_shou_wei_state(cid) == 1)
    return ERR_SHOU_WEI_IS_ACTIVITED;

  int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND,
                                    global_param_cfg::ghz_activate_shou_wei_cost);
  if (ret != 0) return ret;

  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL
      || !ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  if (ghz_activity_obj::get_shou_wei_idx(cid) <= 0)
    return ERR_CLIENT_OPERATE_ILLEGAL;

  coord_t pos;
  char dir = DIR_DOWN;
  if (ghz_activity_obj::get_shou_wei_pos(cid, dir, pos) != 0)
    return ERR_CONFIG_NOT_EXIST;

  int mst_id = spawn_monster::spawn_one(cid,
                                        0,
                                        global_param_cfg::ghz_scene_cid,
                                        global_param_cfg::ghz_scene_cid,
                                        dir,
                                        pos.x_,
                                        pos.y_);

  if (mst_id < 0) return ERR_CONFIG_NOT_EXIST;

  player->do_lose_money(global_param_cfg::ghz_activate_shou_wei_cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_ACTIVATE_SHOU_WEI,
                        cid,
                        0,
                        0);
  ghz_module::on_shou_wei_live(player, mst_id, cid);
  return 0;
}
void ghz_module::do_calc_world_arerage_zhan_li(int &hp, int &gj, int &fy)
{
  ilist<int> char_list;
  rank_module::get_char_list_before_rank(rank_module::RANK_ZHANLI,
                                         char_list,
                                         global_param_cfg::ghz_lv_average);
  if (char_list.empty()) return ;
  int total_hp = 0;
  int total_gj = 0;
  int total_fy = 0;
  for (ilist_node<int> *itor = char_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(itor->value_);
    if (cbi == NULL) continue;
    player_obj *player = player_mgr::instance()->find(itor->value_);
    if (player != NULL)
    {
      total_hp += player->obj_attr_.total_hp();
      total_gj += player->obj_attr_.gong_ji();
      total_fy += player->obj_attr_.fang_yu();
    }else
    {
      total_hp += cbi->attr_[ATTR_T_HP];
      total_gj += cbi->attr_[ATTR_T_GONG_JI];
      total_fy += cbi->attr_[ATTR_T_FANG_YU];
    }
  }
  hp = total_hp / char_list.size();
  gj = total_gj / char_list.size();
  fy = total_fy / char_list.size();
}
int ghz_module::do_obtain_fighting_info(player_obj *player, out_stream &os)
{
  if (player->scene_cid() != global_param_cfg::ghz_scene_cid)
    return ERR_CLIENT_OPERATE_ILLEGAL;
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL
      || !ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  mblock mb(os.wr_ptr(), os.space());
  mb.data_type(ACT_EV_GHZ_OBTAIN_FIGHTING_INFO);
  ao->do_something(&mb, NULL, &mb);
  os.wr_ptr(mb.length());
  return 0;
}
int ghz_module::can_transfer_to(player_obj *player, const int to_scene_cid)
{
  if (to_scene_cid != global_param_cfg::ghz_scene_cid) return 0;
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GHZ);
  if (ao == NULL) return 0;
  return ao->can_enter(player);
}
int ghz_module::can_buy_item(player_obj *player, const int npc_cid)
{
  if (npc_cid != global_param_cfg::ghz_shop_npc)
    return 0;

  if (player->guild_id() == 0)
    return ERR_CLIENT_OPERATE_ILLEGAL;
  if (player->guild_id() != guild_module::get_ghz_winner())
    return ERR_CAN_NOT_BUY_ITEM;
  return 0;
}
