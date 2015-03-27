#include "social_module.h"
#include "message.h"
#include "istream.h"
#include "player_obj.h"
#include "error.h"
#include "player_mgr.h"
#include "db_proxy.h"
#include "social_info.h"
#include "global_macros.h"
#include "sys_log.h"
#include "all_char_info.h"
#include "global_param_cfg.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "clsid.h"
#include "task_module.h"

// Lib header
#include <tr1/unordered_map>

#define R_FRIEND                               1
#define R_BLACK                                2

static ilog_obj *s_log = sys_log::instance()->get_ilog("social");
static ilog_obj *e_log = err_log::instance()->get_ilog("social");

typedef std::tr1::unordered_map<int/*char_id*/, social_info *> social_map_t;
typedef std::tr1::unordered_map<int/*char_id*/, social_info *>::iterator social_map_iter;

/**
 * @class social_relation
 *
 * @brief
 */
class social_relation
{
public:
  social_relation() :
    friend_map_(64),
    black_map_(64)
  { }
  ~social_relation()
  {
    social_map_iter f_iter = this->friend_map_.begin();
    for (; f_iter != this->friend_map_.end(); ++f_iter)
      social_info_pool::instance()->release(f_iter->second);
    social_map_iter b_iter = this->black_map_.begin();
    for (; b_iter != this->black_map_.end(); ++b_iter)
      social_info_pool::instance()->release(b_iter->second);
  }

  social_map_t friend_map_;
  social_map_t black_map_;
};
void social_module::destroy(player_obj *p)
{
  delete p->social_relation_;
  p->social_relation_ = NULL;
}
void social_module::on_enter_game(player_obj *player)
{
  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_GET_SOCIAL_LIST, &db_os);
}
void social_module::do_notify_socialer_my_state(player_obj *player,
                                                const int socialer_id,
                                                const char state)
{
  player_obj *p = player_mgr::instance()->find(socialer_id);
  if (p == NULL) return ;

  char relation = 0;
  if (social_module::is_friend(p, player->id()))
    relation = R_FRIEND;
  else if (social_module::is_black(p, player->id()))
    relation = R_BLACK;
  else
    return;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << player->id() << state << relation;
  p->do_delivery(NTF_SOCIAL_OTHER_CHAR_STATE, &mb);
}
void social_module::on_char_logout(player_obj *player)
{
  ilist<int> *socialer_list = all_char_info::instance()->get_socialer_list(player->id());
  if (socialer_list == NULL) return ;
  for (ilist_node<int> *itor = socialer_list->head();
       itor != NULL;
       itor = itor->next_)
    social_module::do_notify_socialer_my_state(player, itor->value_, 0);
}
void social_module::on_social_list_load_ok(player_obj *player)
{
  ilist<int> *socialer_list = all_char_info::instance()->get_socialer_list(player->id());
  if (socialer_list == NULL) return ;
  for (ilist_node<int> *itor = socialer_list->head();
       itor != NULL;
       itor = itor->next_)
    social_module::do_notify_socialer_my_state(player, itor->value_, 1);

  social_module::do_send_client_all_social_info(player);
}
static int build_one_socialer_info(const int socialer_id, out_stream &os)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(socialer_id);
  if (cbi == NULL) return -1;
  os << cbi->char_id_
    << stream_ostr(cbi->name_, ::strlen(cbi->name_))
    << cbi->career_
    << cbi->lvl_
    << cbi->zhan_li_
    << (player_mgr::instance()->find(cbi->char_id_) == NULL ? (char)0 : (char)1);
  return 0;
}
static int build_one_social_info(social_map_t &sm, out_stream &os)
{
  int *cnt = (int *)os.wr_ptr();
  os << 0;
  for (social_map_iter iter = sm.begin();
       iter != sm.end();
       ++iter)
  {
    if (build_one_socialer_info(iter->second->socialer_id_, os) == 0)
      (*cnt)++;
  }
  return *cnt;
}
int social_module::handle_db_get_social_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  // 数据库加载完成后再new出social_relation防止玩家的社群操作使数据库个内存不同
  social_module::destroy(player);
  player->social_relation_ = new social_relation();

  for (int i = 0; i < cnt; ++i)
  {
    char buff_bf[sizeof(social_info) + 4] = {0};
    stream_istr buff_si(buff_bf, sizeof(buff_bf));
    is >> buff_si;

    social_info *si = social_info_pool::instance()->alloc();
    ::memcpy(si, buff_bf, sizeof(social_info));

    social_module::do_insert_socialer(player, si);
  }

  social_module::on_social_list_load_ok(player);
  return 0;
}
void social_module::do_insert_socialer(player_obj *player, social_info *si)
{
  if (si->relation_ == R_FRIEND)
    player->social_relation_->friend_map_.insert(std::make_pair(si->socialer_id_, si));
  else if (si->relation_ == R_BLACK)
    player->social_relation_->black_map_.insert(std::make_pair(si->socialer_id_, si));
}
void social_module::do_erase_socialer(player_obj *player,
                                      const int socialer_id,
                                      const char relation)
{
  social_info *si = NULL;
  if (relation == R_FRIEND)
  {
    social_map_iter itor = player->social_relation_->friend_map_.find(socialer_id);
    if (itor != player->social_relation_->friend_map_.end())
    {
      si = itor->second;
      player->social_relation_->friend_map_.erase(itor);
    }
  }else if (relation == R_BLACK)
  {
    social_map_iter itor = player->social_relation_->black_map_.find(socialer_id);
    if (itor != player->social_relation_->black_map_.end())
    {
      si = itor->second;
      player->social_relation_->black_map_.erase(itor);
    }
  }
  if (si != NULL) social_info_pool::instance()->release(si);
}
int social_module::do_send_client_all_social_info(player_obj *player)
{
  int cnt = 0;
  out_stream os(client::send_buf, client::send_buf_len);
  os << (char)R_FRIEND;
  cnt += build_one_social_info(player->social_relation_->friend_map_, os);
  os << (char)R_BLACK;
  cnt += build_one_social_info(player->social_relation_->black_map_, os);

  if (cnt == 0) return 0;
  return player->send_request(NTF_SOCIAL_LIST, &os);
}
int social_module::dispatch_msg(player_obj *player,
                                const int msg_id,
                                const char *msg,
                                const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = social_module::FUNC(player, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  if (player->social_relation_ == NULL) return 0;

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_ADD_FRIEND,            clt_add_friend);
    SHORT_CODE(REQ_REMOVE_FRIEND,         clt_remove_friend);
    SHORT_CODE(REQ_ADD_BLACK,             clt_add_black);
    SHORT_CODE(REQ_REMOVE_BLACK,          clt_remove_black);
    SHORT_CODE(REQ_SEARCH_SOCIAL_BY_NAME, clt_search_social_by_name);

    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle social msg %d return %d", msg_id, ret);
  return ret;
}
int social_module::clt_add_friend(player_obj *player,
                                  const char *msg,
                                  const int len)
{
  int inviter_id = 0;
  in_stream is(msg, len);
  is >> inviter_id;
  int ret = social_module::add_friend_i(player, inviter_id);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int social_module::add_friend_i(player_obj *player, const int inviter_id)
{
  if (inviter_id == player->id()) return ERR_CLIENT_OPERATE_ILLEGAL;
  if (player->social_relation_->friend_map_.size()
      >= global_param_cfg::max_friend_cnt + vip_module::to_get_add_friend_amt(player))
    return ERR_OVER_MAX_SOCIAL_F_SIZE;
  if (social_module::is_friend(player, inviter_id))
    return ERR_HAS_BEEN_FRIEND;

  social_module::do_add_socialer(player, inviter_id, R_FRIEND);
  if (social_module::is_black(player, inviter_id))
    social_module::do_remove_socialer(player, inviter_id, R_BLACK);

  player_obj *inviter = player_mgr::instance()->find(inviter_id);
  if (inviter == NULL) return 0;

  if (social_module::is_friend(inviter, player->id()))
    return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr name(player->name(), ::strlen(player->name()));
  os << player->id()
    << name
    << player->career()
    << player->lvl();
  inviter->send_request(NTF_ADD_FRIEND, &os);

  cheng_jiu_module::on_friend_count(player, player->social_relation_->friend_map_.size());
  task_module::on_add_friends(player);
  return 0;
}
int social_module::clt_remove_friend(player_obj *player, const char *msg, const int len)
{
  int cnt = 0;
  int friend_id = 0;
  in_stream is(msg, len);
  is >> cnt;
  if (cnt <= 0 || cnt > MAX_ONCE_SOCIAL_OPERATE)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  for (int i = 0; i < cnt; ++i)
  {
    is >> friend_id;
    int ret = social_module::remove_friend_i(player, friend_id);
    if (ret != 0 && i == 0)
      player->send_respond_err(NTF_OPERATE_RESULT, ret);
  }

  return 0;
}
int social_module::remove_friend_i(player_obj *player, const int friend_id)
{
  if (!social_module::is_friend(player, friend_id))
    return ERR_CLIENT_OPERATE_ILLEGAL;

  social_module::do_remove_socialer(player, friend_id, R_FRIEND);
  return 0;
}
int social_module::clt_add_black(player_obj *player, const char *msg, const int len)
{
  int black_id = 0;
  in_stream is(msg, len);
  is >> black_id;

  int ret = social_module::add_black_i(player, black_id);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int social_module::add_black_i(player_obj *player, const int black_id)
{
  if (black_id == player->id())
    return ERR_CLIENT_OPERATE_ILLEGAL;
  if (player->social_relation_->black_map_.size()
      >= (unsigned)global_param_cfg::max_blacklist_cnt)
    return ERR_OVER_MAX_SOCIAL_B_SIZE;
  if (social_module::is_black(player, black_id))
    return ERR_HAS_BEEN_BLACK;

  social_module::remove_friend_i(player, black_id);
  social_module::do_add_socialer(player, black_id, R_BLACK);
  return 0;
}
int social_module::clt_remove_black(player_obj *player, const char *msg, const int len)
{
  int black_id = 0;
  in_stream is(msg, len);
  is >> black_id;

  if (!social_module::is_black(player, black_id))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  social_module::do_remove_socialer(player, black_id, R_BLACK);
  return 0;
}
int social_module::clt_search_social_by_name(player_obj *player, const char *msg, const int len)
{
  char name[MAX_NAME_LEN+1] = {0};
  stream_istr si(name, sizeof(name));
  in_stream is(msg, len);
  is >> si;
  if (si.str_len() == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (!util::verify_name(si.str()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_INPUT_IS_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid()
    << stream_ostr(name, ::strlen(name));
  db_proxy::instance()->send_request(player->id(), REQ_GET_SEARCH_INFO, &os);

  return 0;
}
int social_module::handle_db_get_search_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  if (cnt <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_HAVE_CHAR);

  out_stream os(client::send_buf, client::send_buf_len);
  int *count = (int *)os.wr_ptr();
  os << 0;
  for (int i = 0; i < cnt; ++i)
  {
    char buff_bf[sizeof(int) + 4] = {0};
    stream_istr buff_si(buff_bf, sizeof(buff_bf));
    is >> buff_si;
    int char_id = 0;
    ::memcpy(&char_id, buff_bf, sizeof(int));
    if (char_id == player->id()) continue;
    if (build_one_socialer_info(char_id, os) == 0)
      (*count)++;
  }

  return player->send_respond_ok(RES_SEARCH_SOCIAL_BY_NAME, &os);
}
void social_module::do_notify_socialer_lvl_up(player_obj *player, const int socialer_id)
{
  player_obj *p = player_mgr::instance()->find(socialer_id);
  if (p == NULL) return ;

  char relation = 0;
  if (social_module::is_friend(p, player->id()))
    relation = R_FRIEND;
  else if (social_module::is_black(p, player->id()))
    relation = R_BLACK;
  else
    return;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << player->id() << player->lvl() << relation;
  p->do_delivery(NTF_SOCIAL_OTHER_CHAR_LVL_UP, &mb);
}
void social_module::do_notify_socialer_zhan_li_update(player_obj *player, const int socialer_id)
{
  player_obj *p = player_mgr::instance()->find(socialer_id);
  if (p == NULL) return ;

  char relation = 0;
  if (social_module::is_friend(p, player->id()))
    relation = R_FRIEND;
  else if (social_module::is_black(p, player->id()))
    relation = R_BLACK;
  else
    return;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << player->id() << player->zhan_li() << relation;
  p->do_delivery(NTF_SOCIAL_OTHER_CHAR_ZHAN_LI_UPDATE, &mb);
}
void social_module::on_char_lvl_up(player_obj *player)
{
  ilist<int> *socialer_list = all_char_info::instance()->get_socialer_list(player->id());
  if (socialer_list == NULL) return ;
  for (ilist_node<int> *itor = socialer_list->head();
       itor != NULL;
       itor = itor->next_)
    social_module::do_notify_socialer_lvl_up(player, itor->value_);
}
void social_module::on_char_zhan_li_update(player_obj *player)
{
  ilist<int> *socialer_list = all_char_info::instance()->get_socialer_list(player->id());
  if (socialer_list == NULL) return ;
  for (ilist_node<int> *itor = socialer_list->head();
       itor != NULL;
       itor = itor->next_)
    social_module::do_notify_socialer_zhan_li_update(player, itor->value_);
}
bool social_module::is_friend(player_obj *player, const int char_id)
{
  if (player->social_relation_ == NULL) return false;
  return player->social_relation_->friend_map_.find(char_id) !=
    player->social_relation_->friend_map_.end();
}
bool social_module::is_black(player_obj *player, const int char_id)
{
  if (player->social_relation_ == NULL) return false;
  return player->social_relation_->black_map_.find(char_id) !=
    player->social_relation_->black_map_.end();
}
void social_module::do_add_socialer(player_obj *player, const int socialer_id, const char relation)
{
  if (player->social_relation_ == NULL) return ;

  social_info *si = social_info_pool::instance()->alloc();
  si->char_id_      = player->id();
  si->relation_     = relation;
  si->socialer_id_  = socialer_id;
  social_module::do_insert_socialer(player, si);

  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr so((char *)si, sizeof(social_info));
  os << player->db_sid() << so;
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_SOCIAL, &os);

  out_stream clt_os(client::send_buf, client::send_buf_len);
  clt_os << relation;
  if (build_one_socialer_info(socialer_id, clt_os) == 0)
    player->send_request(NTF_ADD_SOCIALER, &clt_os);

  all_char_info::instance()->on_i_add_socialer(player->id(), socialer_id);
}
void social_module::do_remove_socialer(player_obj *player,
                                       const int socialer_id,
                                       const char relation)
{
  if (player->social_relation_ == NULL) return ;

  social_module::do_erase_socialer(player, socialer_id, relation);

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << relation << socialer_id;
  db_proxy::instance()->send_request(player->id(), REQ_DELETE_SOCIAL, &os);

  os.reset(client::send_buf, client::send_buf_len);
  os << relation << socialer_id;
  player->send_request(NTF_REMOVE_SOCIALER, &os);

  all_char_info::instance()->on_i_del_socialer(player->id(), socialer_id);
}
