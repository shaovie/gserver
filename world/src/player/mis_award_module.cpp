#include "mis_award_module.h"
#include "istream.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "client.h"
#include "sys.h"
#include "time_util.h"
#include "player_obj.h"
#include "global_param_cfg.h"
#include "behavior_id.h"
#include "vip_module.h"
#include "item_obj.h"
#include "clsid.h"
#include "mis_award_cfg.h"
#include "def.h"
#include "item_obj.h"
#include "package_module.h"
#include "vip_config.h"
#include "vip_module.h"
#include "huo_yue_du_module.h"
#include "gift_config.h"
#include "notice_module.h"

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("award");
static ilog_obj *e_log = err_log::instance()->get_ilog("award");

enum seven_day_login_state
{
  SDL_ST_NULL  = 0, // 初始
  SDL_ST_LOGIN = 1, // 签到
  SDL_ST_AWARD = 2, // 领取
};
void mis_award_module::on_new_day(player_obj *player, const bool when_login)
{
  const int c_days = time_util::diff_days(player->char_info_->c_time_);
  if (c_days < 7
      && player->lvl() >= global_param_cfg::seven_day_login_open_at_lvl)
  {
    mis_award_module::do_set_seven_day_login(player, c_days + 1, SDL_ST_LOGIN);
    if (!when_login)
      mis_award_module::do_notify_seven_day_login_info(player);
  }

  if (!when_login)
    mis_award_module::do_notify_login_award_get_info(player);
}
void mis_award_module::on_enter_game(player_obj *player)
{
  if (player->daily_clean_info_->ol_award_start_time_ == 0)
  {
    player->daily_clean_info_->ol_award_start_time_ = time_util::now;
    player->db_save_daily_clean_info();
  }

  mis_award_module::do_notify_login_award_get_info(player);
  mis_award_module::do_notify_lvl_award_info(player);
  if (mis_award_module::to_get_online_award_left_time(player) == 0)
    mis_award_module::do_notify_online_award_get_info(player);

  if (mis_award_module::to_get_seven_day_login(player, 1) == '\0')
    mis_award_module::do_set_seven_day_login(player, 1, SDL_ST_LOGIN);
  if (mis_award_module::if_need_notify_seven_day_login_to_clt(player))
    mis_award_module::do_notify_seven_day_login_info(player);
}
void mis_award_module::on_char_lvl_up(player_obj *player)
{
  if (player->lvl() == global_param_cfg::seven_day_login_open_at_lvl)
    mis_award_module::do_notify_seven_day_login_info(player);
}
void mis_award_module::do_notify_online_award_get_info(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  player->do_delivery(NTF_ONLINE_AWARD, &mb);
}
int mis_award_module::to_get_online_award_left_time(player_obj *player)
{
  const char next_get_idx = player->daily_clean_info_->ol_award_get_idx_ + 1;
  const int min_time = online_award_cfg::instance()->get_award_min_time(player->lvl(), next_get_idx);
  if (min_time == MAX_INVALID_INT) return -1;
  const int last_time = time_util::now - player->daily_clean_info_->ol_award_start_time_;
  return last_time < min_time ? (min_time - last_time) : 0;
}
void mis_award_module::do_notify_login_award_get_info(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << player->daily_clean_info_->login_award_if_get_
    << (char)time_util::wday();
  player->do_delivery(NTF_LOGIN_AWARD_INFO, &mb);
}
void mis_award_module::do_notify_lvl_award_info(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << player->char_extra_info_->get_lvl_award_idx_;
  player->do_delivery(NTF_LVL_AWARD_INFO, &mb);
}
short mis_award_module::to_get_lvl_award_min_lvl(const short get_lvl)
{
  int min_lvl = 0;
  for (ilist_node<pair_t<int> > *itor = global_param_cfg::lvl_gift_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_.first_ <= get_lvl) continue;
    min_lvl == 0 ? min_lvl = itor->value_.first_ : 0;
    itor->value_.first_ < min_lvl ? min_lvl = itor->value_.first_ : 0;
  }
  return min_lvl;
}
void mis_award_module::do_gen_lvl_award(const short lvl, ilist<item_amount_bind_t> &award_list)
{
  int gift_cid = 0;
  for (ilist_node<pair_t<int> > *itor = global_param_cfg::lvl_gift_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_.first_ == lvl)
    {
      gift_cid = itor->value_.second_;
      break;
    }
  }
  if (gift_cid == 0) return ;

  gift_obj *go = gift_config::instance()->get_gift(gift_cid);
  if (go == NULL || go->gift_list_.empty()) return ;

  for (ilist_node<gift_info *> *itor = go->gift_list_.head();
       itor != NULL;
       itor = itor->next_)
    award_list.push_back(item_amount_bind_t(itor->value_->item_cid_,
                                            itor->value_->max_amt_,
                                            itor->value_->bind_));
}
void player_obj::do_check_online_award(const int now)
{
  const char next_get_idx = this->daily_clean_info_->ol_award_get_idx_ + 1;
  const int min_time = online_award_cfg::instance()->get_award_min_time(this->lvl(), next_get_idx);
  if (min_time == MAX_INVALID_INT) return ;

  if (min_time == now - this->daily_clean_info_->ol_award_start_time_)
    mis_award_module::do_notify_online_award_get_info(this);
}
void mis_award_module::do_notify_seven_day_login_info(player_obj *player)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << (char)(time_util::diff_days(player->char_info_->c_time_) + 1);
  for (int i = 1; i <= 7; ++i)
    mb << mis_award_module::to_get_seven_day_login(player, i);
  player->do_delivery(NTF_SEVEN_DAY_LOGIN, &mb);
}
bool mis_award_module::if_need_notify_seven_day_login_to_clt(player_obj *player)
{
  if (player->lvl() < global_param_cfg::seven_day_login_open_at_lvl)
    return false;
  if (time_util::diff_days(player->char_info_->c_time_) < 7)
    return true;
  for (int i = 0; i < 7; ++i)
  {
    if (player->char_extra_info_->seven_day_login_[i] == SDL_ST_LOGIN + '0')
      return true;
  }
  return false;
}
void mis_award_module::do_set_seven_day_login(player_obj *player, const char day, const char st)
{
  if (day <= 0 || day > 7)
    return ;
  player->char_extra_info_->seven_day_login_[day - 1] = st + '0';
  for (int i = day - 2; i >= 0; i--)
  {
    if (player->char_extra_info_->seven_day_login_[i] != '\0')
      break;
    player->char_extra_info_->seven_day_login_[i] = SDL_ST_NULL + '0';
  }
  player->db_save_char_extra_info();
}
char mis_award_module::to_get_seven_day_login(player_obj *player, const char day)
{
  if (day <= 0 || day > 7)
    return SDL_ST_NULL;
  if (player->char_extra_info_->seven_day_login_[day - 1] == '\0')
    return SDL_ST_NULL;
  return player->char_extra_info_->seven_day_login_[day - 1] - '0';
}

int mis_award_module::dispatch_msg(player_obj *player, const int msg_id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = mis_award_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_ONLINE_AWARD,       clt_obtain_online_award);
    SHORT_CODE(REQ_GET_ONLINE_AWARD,          clt_get_online_award);
    SHORT_CODE(REQ_GET_LOGIN_AWARD,           clt_get_login_award);
    SHORT_CODE(REQ_GET_LVL_AWARD,             clt_get_lvl_award);
    SHORT_CODE(REQ_GET_SEVEN_DAY_LOGIN_AWARD, clt_get_seven_day_login_award);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int mis_award_module::clt_obtain_online_award(player_obj *player, const char *, const int )
{
  const int ret = mis_award_module::to_get_online_award_left_time(player);
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->daily_clean_info_->ol_award_get_idx_ << ret; // left_time
  return player->send_request(RES_OBTAIN_ONLINE_AWARD, &os);
}
int mis_award_module::clt_get_online_award(player_obj *player, const char *, const int )
{
  int ret = mis_award_module::to_get_online_award_left_time(player);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_NO_REACH_TIME);

  const char next_get_idx = player->daily_clean_info_->ol_award_get_idx_ + 1;
  ilist<item_amount_bind_t> *list = online_award_cfg::instance()->award_list(player->lvl(), next_get_idx);
  if (list == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  ret = package_module::do_insert_award_item_list(player,
                                                  list,
                                                  blog_t(ITEM_GOT_ONLINE_AWARD, 0, 0));
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->daily_clean_info_->ol_award_get_idx_ = next_get_idx;
  player->daily_clean_info_->ol_award_start_time_ = time_util::now;
  player->db_save_daily_clean_info();

  out_stream os(client::send_buf, client::send_buf_len);
  os << next_get_idx;
  return player->send_respond_ok(RES_GET_ONLINE_AWARD, &os);
}
int mis_award_module::clt_get_login_award(player_obj *player, const char *, const int )
{
  if (player->daily_clean_info_->login_award_if_get_ != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_AWARD_HAD_GET);

  ilist<item_amount_bind_t> *list = login_award_cfg::instance()->award_list(time_util::wday());
  if (list == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  item_amount_bind_t *vip_award = vip_config::instance()->login_award(vip_module::vip_lvl(player),
                                                                      time_util::wday());
  if (vip_award != NULL) //insert vip must delete after use
    list->push_front(*vip_award);
  const int ret = package_module::do_insert_award_item_list(player,
                                                            list,
                                                            blog_t(ITEM_GOT_LOGIN_AWARD, 0, 0));
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  if (vip_award != NULL) // delete vip
    list->pop_front();

  huo_yue_du_module::on_login_award(player);

  player->daily_clean_info_->login_award_if_get_ = 1;
  player->db_save_daily_clean_info();
  return player->send_respond_ok(RES_GET_LOGIN_AWARD);
}
int mis_award_module::clt_get_lvl_award(player_obj *player, const char *, const int )
{
  const short had_get_lvl = player->char_extra_info_->get_lvl_award_idx_;
  const short can_get_lvl = mis_award_module::to_get_lvl_award_min_lvl(had_get_lvl);
  if (can_get_lvl > player->lvl())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_LVL_NOT_ENOUGH);

  ilist<item_amount_bind_t> list;
  mis_award_module::do_gen_lvl_award(can_get_lvl, list);
  if (list.empty())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  const int ret = package_module::do_insert_award_item_list(player,
                                                            &list,
                                                            blog_t(ITEM_GOT_LVL_AWARD, 0, 0));
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  player->char_extra_info_->get_lvl_award_idx_ = can_get_lvl;
  player->db_save_char_extra_info();
  mis_award_module::do_notify_lvl_award_info(player);

  out_stream os(client::send_buf, client::send_buf_len);
  os << can_get_lvl;
  return player->send_respond_ok(RES_GET_LVL_AWARD, &os);
}
int mis_award_module::clt_get_seven_day_login_award(player_obj *player, const char *msg, const int len)
{
  char get_day = 0;
  in_stream is(msg, len);
  is >> get_day;

  if (mis_award_module::to_get_seven_day_login(player, get_day) != SDL_ST_LOGIN)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  ilist<item_amount_bind_t> *al = seven_day_login_cfg::instance()->award_list(get_day);
  if (al == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  const int ret = package_module::do_insert_award_item_list(player,
                                                            al,
                                                            blog_t(ITEM_GOT_SEVEN_DAY_LOGIN, 0, 0));
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  // notice
  for (ilist_node<item_amount_bind_t> *itor = al->head();
       itor != NULL;
       itor = itor->next_)
  {
    item_amount_bind_t &io = itor->value_;
    if (clsid::is_char_equip(io.cid_))
    {
      item_obj notice_item;
      notice_item.cid_ = clsid::get_equip_cid_by_career(player->career(), io.cid_);
      notice_item.amount_ = io.amount_;
      notice_item.bind_ = io.bind_;
      notice_module::got_seven_day_login_award(player->id(), player->name(), &notice_item);
    }
  }

  mis_award_module::do_set_seven_day_login(player, get_day, SDL_ST_AWARD);
  mis_award_module::do_notify_seven_day_login_info(player);
  return 0;
}
