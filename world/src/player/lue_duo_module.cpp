#include "lue_duo_module.h"
#include "player_obj.h"
#include "message.h"
#include "error.h"
#include "client.h"
#include "istream.h"
#include "sys_log.h"
#include "global_param_cfg.h"
#include "rank_module.h"
#include "all_char_info.h"
#include "scp_module.h"
#include "jing_ji_module.h"
#include "lueduo_item.h"
#include "lueduo_log.h"
#include "package_module.h"
#include "player_mgr.h"
#include "db_proxy.h"
#include "clsid.h"
#include "time_util.h"
#include "behavior_id.h"
#include "char_db_msg_queue.h"
#include "char_msg_queue_module.h"
#include "mail_module.h"
#include "mail_config.h"
#include "mail_info.h"
#include "notice_module.h"
#include "vip_module.h"
#include "lvl_param_cfg.h"
#include "scp_mgr.h"
#include "monster_mgr.h"
#include "monster_obj.h"
#include "lue_duo_config.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("lue_duo");
static ilog_obj *e_log = err_log::instance()->get_ilog("lue_duo");

class ld_info
{
public:
  ld_info(const int char_id, const int monster_id, const int material_id)
    : char_id_(char_id), monster_id_(monster_id), material_id_(material_id)
  { }

  int char_id_;
  int monster_id_;
  int material_id_;
};
typedef std::map<int, ld_info > lue_duo_map_t;
typedef std::map<int, ld_info >::iterator lue_duo_map_iter;
static lue_duo_map_t s_lue_duo_combat;

typedef std::map<int, ilist<lueduo_item>* > lueduo_item_map_t;
typedef std::map<int, ilist<lueduo_item>* >::iterator lueduo_item_map_iter;
static lueduo_item_map_t s_lueduo_item_cache;

void lue_duo_module::destroy(player_obj *player)
{
  while (!player->lueduo_log_list_.empty())
    delete player->lueduo_log_list_.pop_front();
}
int lue_duo_module::dispatch_msg(player_obj *player,
                                 const int msg_id,
                                 const char *msg,
                                 const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = lue_duo_module::FUNC(player, msg, len);      \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch(msg_id)
  {
    SHORT_CODE(REQ_GET_MATE_HOLDER_LIST,  clt_get_material_holder_list);
    SHORT_CODE(REQ_ROB_SOMEONE,           clt_rob_someone);
    SHORT_CODE(REQ_GET_LOG_ROBBED,        clt_get_log_robbed);
    SHORT_CODE(REQ_LUEDUO_REVENGE,        clt_revenge);
    SHORT_CODE(REQ_LUEDUO_FANPAI,         clt_fanpai);

    SHORT_DEFAULT;
  }
  return ret;
}

int lue_duo_module::clt_get_material_holder_list(player_obj *player,
                                                 const char *msg,
                                                 const int len)
{
  in_stream is(msg, len);
  int material_cid = 0;
  is >> material_cid;

  if (!lue_duo_module::is_material_ok(material_cid))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const int need_cnt = 4;
  int find_player[need_cnt] = {0};
  int find_cnt = rank_module::rand_players_for_lue_duo(player,
                                                       material_cid,
                                                       &lue_duo_module::has_material,
                                                       need_cnt,
                                                       find_player);

  out_stream os(client::send_buf, client::send_buf_len);
  os << material_cid;
  char *cnt = os.wr_ptr();
  os << char(0);
  for (int i = 0; i < find_cnt; ++i)
  {
    char_brief_info *info = all_char_info::instance()->get_char_brief_info(find_player[i]);
    if (info == NULL) continue;

    os << find_player[i]
      << stream_ostr(info->name_, ::strlen(info->name_))
      << info->career_
      << info->zhan_li_
      << info->lvl_
      << lue_duo_module::material_cnt(find_player[i], material_cid);

    ++(*cnt);
  }
  return player->send_respond_ok(RES_GET_MATE_HOLDER_LIST, &os);
}
int lue_duo_module::clt_rob_someone(player_obj *player,
                                    const char *msg,
                                    const int len)
{
  in_stream is(msg, len);
  int material_cid = 0, char_id = 0;
  is >> char_id >> material_cid;

  if (!clsid::is_world_scene(player->scene_cid()))
    return player->send_respond_err(RES_ROB_SOMEONE, ERR_CLIENT_OPERATE_ILLEGAL);
  if (player->lvl() < global_param_cfg::lue_duo_lvl)
    return player->send_respond_err(RES_ROB_SOMEONE, ERR_LVL_NOT_ENOUGH);
  if (!lue_duo_module::has_material(char_id, material_cid))
    return player->send_respond_err(RES_ROB_SOMEONE, ERR_TARGET_DONT_HAVE_MATERIAL);
  if (player->jing_li() < global_param_cfg::jingli_cost)
    return player->send_respond_err(RES_ROB_SOMEONE, ERR_JING_LI_NOT_ENOUGH);

  int ret = lue_duo_module::do_rob_someone(player, char_id, material_cid);
  if (ret != 0)
    return player->send_respond_err(RES_ROB_SOMEONE, ret);
  return player->send_respond_ok(RES_ROB_SOMEONE);
}
int lue_duo_module::clt_get_log_robbed(player_obj *player,
                                       const char *,
                                       const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  char *cnt = os.wr_ptr();
  os << char(0);
  ilist_node<lueduo_log *> *iter = player->lueduo_log_list_.head();
  for (; (*cnt) < 10 && iter != NULL; iter = iter->next_)
  {
    char_brief_info *info
      = all_char_info::instance()->get_char_brief_info(iter->value_->robber_);
    if (info == NULL) continue;

    os << iter->value_->robber_
      << stream_ostr((const char *)info->name_, ::strlen(info->name_))
      << iter->value_->time_
      << iter->value_->result_
      << iter->value_->material_
      << iter->value_->amount_
      << iter->value_->if_revenge_;

    ++(*cnt);
  }
  return player->send_respond_ok(RES_GET_LOG_ROBBED, &os);
}
int lue_duo_module::clt_revenge(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int time = 0, robber = 0;
  is >> time >> robber;

  ilist_node<lueduo_log *> *iter = player->lueduo_log_list_.head();
  for (; iter != NULL; iter = iter->next_)
  {
    if (iter->value_->robber_ == robber
        && iter->value_->time_ == time)
      break;
  }

  if (iter == NULL
      || iter->value_->amount_ == 0
      || iter->value_->if_revenge_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (!clsid::is_world_scene(player->scene_cid()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  if (player->jing_li() < global_param_cfg::jingli_cost)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_JING_LI_NOT_ENOUGH);

  int ret = lue_duo_module::do_rob_someone(player, robber, iter->value_->material_);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  iter->value_->if_revenge_ = char(1);
  stream_ostr log_so((const char *)iter->value_, sizeof(lueduo_log));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << log_so;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_LUEDUO_LOG, &os);

  return player->send_respond_ok(RES_LUEDUO_REVENGE);
}
int lue_duo_module::clt_fanpai(player_obj *player, const char *, const int )
{
  if (player->scene_cid() != global_param_cfg::lue_duo_map)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  ld_obj *ldo = lue_duo_config::instance()->get_random_ld_obj();
  if (ldo == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  // Must check at last
  // The status will be changed when event `SCP_EV_LUEDUO_FANPAI` occur
  int res = 0;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.data_type(SCP_EV_LUEDUO_FANPAI);
  scp_mgr::instance()->do_something(player->scene_id(), &mb, &res, NULL);
  if (res != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, res);

  if (item_config::item_is_money(ldo->item_cid_))
    player->do_got_item_money(ldo->item_cid_,
                              ldo->item_cnt_,
                              MONEY_GOT_LUEDUO_FANPAI,
                              0);
  else if (!package_module::would_be_full(player,
                                          PKG_PACKAGE,
                                          ldo->item_cid_,
                                          ldo->item_cnt_,
                                          BIND_TYPE))
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   ldo->item_cid_,
                                   ldo->item_cnt_,
                                   BIND_TYPE,
                                   ITEM_GOT_LUEDUO_FANPAI,
                                   0,
                                   0);
  out_stream os(client::send_buf, client::send_buf_len);
  os << ldo->item_cid_ << ldo->item_cnt_;
  return player->send_respond_ok(RES_LUEDUO_FANPAI, &os);
}

bool lue_duo_module::is_material_ok(const int material_cid)
{
  for (ilist_node<int> *iter = global_param_cfg::robbed_items.head();
       iter != NULL;
       iter = iter->next_)
  {
    if (iter->value_ == material_cid)
      return true;
  }
  return false;
}
bool lue_duo_module::has_material(const int char_id,
                                  const int material_cid)
{
  return lue_duo_module::material_cnt(char_id, material_cid) != 0;
}
int lue_duo_module::material_cnt(const int char_id,
                                 const int material_cid)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player != NULL)
    return package_module::calc_item_amount(player, material_cid);

  lueduo_item_map_iter iter = s_lueduo_item_cache.find(char_id);
  if (iter == s_lueduo_item_cache.end()) return 0;

  ilist<lueduo_item> * ldil = iter->second;
  for (ilist_node<lueduo_item> *note = ldil->head();
       note != NULL;
       note = note->next_)
  {
    if (material_cid == note->value_.cid_)
      return note->value_.bind_amount_ + note->value_.unbind_amount_;
  }
  return 0;
}
bool lue_duo_module::rand_if_get_material(player_obj *player, const int other_id)
{
  int xishu = 0;
  player_obj *other = player_mgr::instance()->find(other_id);
  if (other != NULL)
    xishu = int(other->zhan_li() * 100.0 / player->zhan_li());
  else
  {
    char_brief_info *info = all_char_info::instance()->get_char_brief_info(other_id);
    if (info != NULL)
      xishu = int(info->zhan_li_ * 100.0 / player->zhan_li());
  }

  ilist_node<rob_probability> *in = global_param_cfg::rob_prob_list.head();
  for (; in != NULL; in = in->next_)
  {
    if (in->value_.min_per_ <= xishu && in->value_.max_per_ > xishu)
    {
      if (rand() % 100 < in->value_.prob_)
        return true;
      break;
    }
  }
  return false;
}
int lue_duo_module::transfer_item_robbed(player_obj *player,
                                         const int other_id,
                                         const int material_cid)
{
  int b_cnt = 0, unb_cnt = 0;
  ilist_node<lueduo_item> *node = NULL;

  player_obj *other = player_mgr::instance()->find(other_id);
  if (other != NULL)
  {
    b_cnt = package_module::calc_item_amount(player, material_cid, BIND_TYPE);
    unb_cnt = package_module::calc_item_amount(player, material_cid, UNBIND_TYPE);
  }else
  {
    lueduo_item_map_iter iter = s_lueduo_item_cache.find(other_id);
    if (iter == s_lueduo_item_cache.end()) return 0;

    for (node = iter->second->head(); node != NULL; node = node->next_)
    {
      lueduo_item &item = node->value_;
      if (material_cid == item.cid_)
      {
        b_cnt = item.bind_amount_, unb_cnt = item.unbind_amount_;
        break;
      }
    }
  }
  if (b_cnt + unb_cnt == 0) return 0;

  int percent = rand() % (global_param_cfg::max_robbed_percent - global_param_cfg::min_robbed_percent + 1)
    + global_param_cfg::min_robbed_percent;

  int got_bind_cnt = 0, got_unbind_cnt = 0;
  int got_cnt = (b_cnt + unb_cnt) * percent / 100;
  if (got_cnt == 0) got_cnt = 1;
  if (got_cnt <= b_cnt)
    got_bind_cnt = got_cnt;
  else
  {
    got_bind_cnt = b_cnt;
    got_unbind_cnt = got_cnt - got_bind_cnt;
  }

  const mail_obj *mo
    = mail_config::instance()->get_mail_obj(mail_config::LUEDUO_PLUNDER);
  if (mo == NULL)
  {
    e_log->error("Cannot find mail obj when char:%d lueduo %d material(s):%d",
                 other_id, got_cnt, material_cid);
    return 0;
  }

  if (other != NULL)
    package_module::do_remove_item(other, material_cid, got_cnt, ITEM_LOSE_LUEDUO, player->id());
  else
  {
    node->value_.bind_amount_ -= got_bind_cnt;
    node->value_.unbind_amount_ -= got_unbind_cnt;
    stream_ostr item_so((const char *)&node->value_, sizeof(lueduo_item));
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << item_so;
    db_proxy::instance()->send_request(other_id, REQ_UPDATE_LUEDUO_ITEM, &os);
  }

  int attach_num = 0;
  item_obj il[MAX_ATTACH_NUM];
  if (got_bind_cnt != 0)
  {
    item_obj *io = package_module::alloc_new_item(player->id(),
                                                  material_cid,
                                                  got_bind_cnt,
                                                  BIND_TYPE);
    il[attach_num ++] = *io;
    package_module::release_item(io);
  }
  if (got_unbind_cnt != 0)
  {
    item_obj *io = package_module::alloc_new_item(player->id(),
                                                  material_cid,
                                                  got_unbind_cnt,
                                                  UNBIND_TYPE);
    il[attach_num ++] = *io;
    package_module::release_item(io);
  }
  int size = 1;
  ::strncpy(mail_module::replace_str[0], MAIL_P_ITEM, sizeof(mail_module::replace_str[0]) - 1);
  notice_module::build_item_str(&il[0], mail_module::replace_value[0], MAX_REPLACE_STR + 1);
  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);
  mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                            mo->sender_name_, mo->title_, content,
                            mail_info::MAIL_TYPE_GM,
                            0, 0, 0,
                            attach_num, il,
                            player->db_sid(), time_util::now);

  mail_module::do_notify_haved_new_mail(player, 1);
  return got_cnt;
}
void lue_duo_module::save_lueduo_log(player_obj *player,
                                     const int other_id,
                                     const int material_cid,
                                     const int got_cnt,
                                     const bool fail,
                                     const int time)
{
  lueduo_log *ll = lueduo_log_pool::instance()->alloc();
  ll->char_id_ = player->id();
  ll->robber_  = other_id;
  ll->time_    = time != 0 ? time : time_util::now;
  ll->material_ = material_cid;
  ll->amount_  = got_cnt;
  ll->result_  = fail ? char(1) : char(0);

  player->lueduo_log_list_.push_front(ll);

  stream_ostr log_so((const char *)ll, sizeof(lueduo_log));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << log_so;
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_LUEDUO_LOG, &os);
}
void lue_duo_module::on_lue_duo_end(const int char_id, const bool win)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL)
  {
    e_log->error("can not find char %d, result:%s", char_id, win?"win":"lose");
    return;
  }

  int other_id = 0, material_cid = 0, got_cnt = 0;
  lue_duo_map_iter iter = s_lue_duo_combat.find(char_id);
  if (iter == s_lue_duo_combat.end())
    e_log->error("can not find char %d's combat info, result:%s",
                 char_id, win?"win":"lose");
  else
  {
    other_id = iter->second.char_id_, material_cid = iter->second.material_id_;

    if (win
        && player->zhan_li() != 0
        && lue_duo_module::rand_if_get_material(player, other_id))
    {
      got_cnt = lue_duo_module::transfer_item_robbed(player, other_id, material_cid);
    }

    player_obj *other = player_mgr::instance()->find(other_id);
    if (other != NULL)
      lue_duo_module::save_lueduo_log(other, char_id, material_cid, got_cnt, win);
    else
    {
      char_db_msg_queue cdmq;
      cdmq.char_id_ = other_id;
      cdmq.msg_id_  = CMQ_LUEDUO_RESULT;
      ::snprintf(cdmq.param_, sizeof(cdmq.param_),
                 "%d,%d,%d,%d,%d",
                 char_id, material_cid, got_cnt, time_util::now, win?1:0);
      out_stream os(client::send_buf, client::send_buf_len);
      os << 0 << stream_ostr((char *)&cdmq, sizeof(char_db_msg_queue));
      db_proxy::instance()->send_request(other_id, REQ_INSERT_CHAR_DB_MSG, &os);
    }

    int coin = lvl_param_cfg::instance()->rob_coin(player->lvl());
    if (!player->is_money_upper_limit(M_COIN, coin))
      player->do_got_money(coin, M_COIN, MONEY_GOT_LUEDUO, other_id);
    player->do_got_exp(lvl_param_cfg::instance()->rob_exp(player->lvl()));
  }
  s_lue_duo_combat.erase(char_id);
  out_stream os(client::send_buf, client::send_buf_len);
  os << (win?char(1):char(0)) << material_cid << got_cnt;
  player->send_request(NTF_LUEDUO_RESULT, &os);
}
void lue_duo_module::deal_preload(const int char_id,
                                  const int cid,
                                  const int b_cnt,
                                  const int unb_cnt)
{
  ilist<lueduo_item> * ldil = NULL;
  lueduo_item_map_iter iter = s_lueduo_item_cache.find(char_id);
  if (iter != s_lueduo_item_cache.end())
    ldil = iter->second;
  else
  {
    ldil = new ilist<lueduo_item>();
    s_lueduo_item_cache.insert(std::make_pair(char_id, ldil));
  }

  ldil->push_back(lueduo_item(char_id, cid, b_cnt, unb_cnt));
}
void lue_duo_module::on_char_logout(player_obj *player)
{
  ilist<lueduo_item> * ldil = NULL;
  lueduo_item_map_iter iter = s_lueduo_item_cache.find(player->id());
  if (iter != s_lueduo_item_cache.end())
    ldil = iter->second;
  else
  {
    ldil = new ilist<lueduo_item>();
    s_lueduo_item_cache.insert(std::make_pair(player->id(), ldil));
  }

  ilist_node<lueduo_item> * liter = ldil->head();
  for (; liter != NULL; liter = liter->next_)
  {
    lueduo_item &litem = liter->value_;
    int b_cnt = package_module::calc_item_amount(player, litem.cid_, BIND_TYPE);
    int unb_cnt = package_module::calc_item_amount(player, litem.cid_, UNBIND_TYPE);
    if (litem.bind_amount_ != b_cnt || litem.unbind_amount_ != unb_cnt)
    {
      litem.bind_amount_ = b_cnt;
      litem.unbind_amount_ = unb_cnt;

      stream_ostr item_so((const char *)&litem, sizeof(lueduo_item));
      out_stream os(client::send_buf, client::send_buf_len);
      os << player->db_sid() << item_so;
      db_proxy::instance()->send_request(player->id(), REQ_UPDATE_LUEDUO_ITEM, &os);
    }
  }

  ilist_node<int> *ri = global_param_cfg::robbed_items.head();
  for (; ri != NULL; ri = ri->next_)
  {
    liter = ldil->head();
    for (; liter != NULL; liter = liter->next_)
    {
      if (ri->value_ == liter->value_.cid_)
        break;
    }
    if (liter == NULL)
    {
      int b_cnt = package_module::calc_item_amount(player, ri->value_, BIND_TYPE);
      int unb_cnt = package_module::calc_item_amount(player, ri->value_, UNBIND_TYPE);
      if (b_cnt != 0 || unb_cnt != 0)
      {
        lueduo_item item(player->id(), ri->value_, b_cnt, unb_cnt);
        ldil->push_back(item);

        stream_ostr item_so((const char *)&item, sizeof(lueduo_item));
        out_stream os(client::send_buf, client::send_buf_len);
        os << player->db_sid() << item_so;
        db_proxy::instance()->send_request(player->id(), REQ_INSERT_LUEDUO_ITEM, &os);
      }
    }
  }

  while (!player->lueduo_log_list_.empty())
    lueduo_log_pool::instance()->release(player->lueduo_log_list_.pop_front());
}
void lue_duo_module::on_enter_scene(player_obj *player)
{
  if (player->scene_cid() != global_param_cfg::lue_duo_map)
    return ;

  lue_duo_map_iter itor = s_lue_duo_combat.find(player->id());
  if (itor != s_lue_duo_combat.end())
  {
    monster_obj *mo = monster_mgr::instance()->find(itor->second.monster_id_);
    if (mo == NULL) return ;
    mo->post_aev(AEV_TO_PATROL, NULL);
  }
}
void lue_duo_module::handle_db_get_lueduo_log_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(lueduo_log) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    lueduo_log *info = (lueduo_log *)bf;

    lueduo_log *new_log = lueduo_log_pool::instance()->alloc();
    ::memcpy(new_log, info, sizeof(lueduo_log));
    player->lueduo_log_list_.push_back(new_log);
  }
}
int lue_duo_module::do_rob_someone(player_obj *player,
                                   const int char_id,
                                   const int material_cid)
{
  int ret = scp_module::do_enter_single_scp(player, global_param_cfg::lue_duo_map);
  if (ret < 0) return ret;

  ret = jing_ji_module::create_competitor(player,
                                          char_id,
                                          global_param_cfg::jing_ji_mst);
  if (ret < 0) return ret;

  lue_duo_map_iter iter = s_lue_duo_combat.find(player->id());
  if (iter == s_lue_duo_combat.end())
    s_lue_duo_combat.insert(std::make_pair(player->id(),
                                           ld_info(char_id, ret, material_cid)));
  else
  {
    e_log->error("char %d lue duo char %d donot be deleted!");
    iter->second.char_id_ = char_id;
    iter->second.monster_id_ = ret;
    iter->second.material_id_ = material_cid;
  }

  if ((player->do_add_hp(player->total_hp()) | player->do_add_mp(player->total_mp())) != 0)
    player->broadcast_hp_mp();

  int jingli_upper_limit
    = global_param_cfg::jingli_value + vip_module::to_get_jingli_add_upper_limit(player);
  if (player->jing_li() == jingli_upper_limit)
    player->char_extra_info_->jing_li_time_ = time_util::now;
  player->jing_li(player->jing_li() - global_param_cfg::jingli_cost);
  player->db_save_char_extra_info();

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->jing_li();
  player->send_request(NTF_UPDATE_JING_LI, &os);

  return 0;
}
