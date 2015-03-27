#include "kai_fu_act_obj.h"
#include "activity_cfg.h"
#include "service_info.h"
#include "mail_module.h"
#include "mail_config.h"
#include "player_obj.h"
#include "mail_info.h"
#include "global_param_cfg.h"
#include "kai_fu_act_log.h"
#include "kai_fu_act_cfg.h"
#include "rank_module.h"
#include "guild_module.h"
#include "jing_ji_module.h"
#include "all_char_info.h"
#include "activity_mgr.h"
#include "player_mgr.h"
#include "db_proxy.h"
#include "istream.h"
#include "sys_log.h"
#include "sys.h"

// Lib header
#include "ev_handler.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("act");
static ilog_obj *e_log = err_log::instance()->get_ilog("act");

static int s_kai_fu_act_last_days = 6;

class give_award_info
{
public:
  give_award_info()
    : char_id_(0),
    idx_(0)
  { }
  int char_id_;
  int idx_;
  ilist<item_amount_bind_t> award_list_;
};
class kai_fu_award_timer : public ev_handler
{
public:
  kai_fu_award_timer(const int mail_id,
                     ilist<give_award_info *> *give_award_list) :
    mail_id_(mail_id),
    give_award_list_(give_award_list)
  { }
  ~kai_fu_award_timer()
  {
    if (this->give_award_list_ != NULL)
      delete this->give_award_list_;
  }
  virtual int handle_timeout(const time_value &)
  {
    for (int i = 1; i <= 25; ++i)
    {
      if (this->give_award_list_->empty()) return -1;

      give_award_info *p = this->give_award_list_->pop_front();
      kai_fu_act_obj::do_give_act_award(p->char_id_,
                                        this->mail_id_,
                                        p->idx_,
                                        p->award_list_);
      delete p;
    }
    return 0;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    delete this;
    return 0;
  }
private:
  int mail_id_;
  ilist<give_award_info *> *give_award_list_;
};

void kai_fu_act_obj::destroy(player_obj *player)
{
  while (!player->kai_fu_act_log_.empty())
  {
    kai_fu_act_log *p = player->kai_fu_act_log_.pop_front();
    delete p;
  }
}
kai_fu_act_obj::kai_fu_act_obj() :
  activity_obj(ACTIVITY_KAI_FU)
{ }
int kai_fu_act_obj::parse_cfg_param(const char *cfg)
{
  if (cfg == NULL) return -1;
  s_kai_fu_act_last_days = ::atoi(cfg) - 1;
  if (s_kai_fu_act_last_days < 1
      || s_kai_fu_act_last_days > 30)
    return -1;
  return 0;
}
bool kai_fu_act_obj::do_check_if_opened(const int now)
{
  this->left_time_ = 0;
  activity_cfg_obj *aco = activity_cfg::instance()->get_act_cfg_obj(this->act_id_);
  if (aco == NULL || !aco->is_valid_) return false;

  if (!sys::svc_info->opened) return false;
  int lt = s_kai_fu_act_last_days*86400 - (now - sys::svc_info->open_time);
  if (lt <= 0) return false;
  this->left_time_ = lt;
  return true;
}
void kai_fu_act_obj::on_enter_game(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_KAI_FU);
  if (ao == NULL
      || !ao->is_opened())
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void kai_fu_act_obj::build_rank_award_info(const int delay,
                                           const int mail_id,
                                           ilist<kai_fu_act_cfg_obj *> *lp,
                                           ilist<int> &char_list)
{
  ilist<give_award_info *> *give_award_list = new ilist<give_award_info *>();
  for (ilist_node<kai_fu_act_cfg_obj *> *k_itor = lp->head();
       k_itor != NULL;
       k_itor = k_itor->next_)
  {
    ilist_node<int> *c_itor = char_list.head();
    for (int n = 1; c_itor != NULL; c_itor = c_itor->next_, ++n)
    {
      if (k_itor->value_->open_
          && k_itor->value_->param_ == n)
      {
        give_award_info *gai = new give_award_info();
        give_award_list->push_back(gai);
        gai->idx_ = n;
        gai->char_id_ = c_itor->value_;
        for (ilist_node<item_amount_bind_t> *i_itor = k_itor->value_->award_list_.head();
             i_itor != NULL;
             i_itor = i_itor->next_)
          gai->award_list_.push_back(i_itor->value_);
      }
    }
  }
  kai_fu_award_timer *kp = new kai_fu_award_timer(mail_id, give_award_list);
  sys::r->schedule_timer(kp, time_value(0, delay*1000), time_value(0, 500*1000));
}
int kai_fu_act_obj::close()
{
  //== give rank award
  int delay = 0;
  { // rank lvl 
    delay += 200;
    ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::RANK_LVL);
    if (lp != NULL)
    {
      ilist<int> char_list;
      rank_module::get_char_list_before_rank(rank_module::RANK_LVL,
                                             char_list,
                                             lp->size());
      kai_fu_act_obj::build_rank_award_info(delay,
                                            mail_config::KAI_FU_ACT_LVL_RANK,
                                            lp,
                                            char_list);
    }
  }
  { // rank zhan li
    delay += 200;
    ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::RANK_ZHAN_LI);
    if (lp != NULL)
    {
      ilist<int> char_list;
      rank_module::get_char_list_before_rank(rank_module::RANK_ZHANLI,
                                             char_list,
                                             lp->size());
      kai_fu_act_obj::build_rank_award_info(delay,
                                            mail_config::KAI_FU_ACT_ZHAN_LI_RANK,
                                            lp,
                                            char_list);
    }
  }
  { // rank jing ji
    delay += 200;
    ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::RANK_JING_JI);
    if (lp != NULL)
    {
      ilist<int> char_list;
      jing_ji_module::get_char_list_before_rank(char_list, lp->size());
      kai_fu_act_obj::build_rank_award_info(delay,
                                            mail_config::KAI_FU_ACT_JING_JI_RANK,
                                            lp,
                                            char_list);
    }
  }
  { // rank tui tu
    delay += 200;
    ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::RANK_TUI_TU);
    if (lp != NULL)
    {
      ilist<int> char_list;
      rank_module::get_char_list_before_rank(rank_module::RANK_MSTAR,
                                             char_list,
                                             lp->size());
      kai_fu_act_obj::build_rank_award_info(delay,
                                            mail_config::KAI_FU_ACT_TUI_TU_RANK,
                                            lp,
                                            char_list);
    }
  }
  {
    // rank guild
    delay += 200;
    ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::RANK_GUILD);
    if (lp != NULL)
    {
      ilist<int> id_list;
      guild_module::get_guild_list_before_rank(id_list, lp->size());
      int n = 1;
      while (!id_list.empty())
      {
        int guild_id = id_list.pop_front();
        ilist<int> char_list;
        guild_module::get_member_list(guild_id, char_list);
        delay += 200;
        kai_fu_act_obj::build_rank_award_info(delay,
                                              mail_config::KAI_FU_ACT_GUILD_RANK,
                                              lp,
                                              char_list);

        // for chairman
        for (ilist_node<kai_fu_act_cfg_obj *> *k_itor = lp->head();
             k_itor != NULL;
             k_itor = k_itor->next_)
        {
          if (k_itor->value_->open_
              && k_itor->value_->param_ == n)
          {
            kai_fu_act_obj::do_give_act_award(guild_module::get_chairman(guild_id),
                                              mail_config::KAI_FU_ACT_GUILD_RANK_FOR_LEADER,
                                              n,
                                              k_itor->value_->award_list_2_);
            break;
          }
        }
        n++;
      }
    }
  }

  return 0;
}
void kai_fu_act_obj::do_give_act_award(const int char_id,
                                       const int mail_id,
                                       const int value,
                                       ilist<item_amount_bind_t> &award_list)
{
  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_id);
  if (mo == NULL) return ;
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return ;

  int size = 1;
  ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
  if (mail_id == mail_config::KAI_FU_ACT_ALL_EQUIP_DJ)
  {
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_EQUIP_DJ, value);
  }else
  {
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_NUMBER, value);
  }
  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);

  mail_module::do_send_mail(char_id,
                            cbi->career_,
                            0,
                            mail_info::MAIL_TYPE_GM,
                            mo->sender_name_, mo->title_, content,
                            award_list);
}
void kai_fu_act_obj::handle_db_get_kai_fu_act_log_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(kai_fu_act_log) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    kai_fu_act_log *info = (kai_fu_act_log *)bf;

    if (kai_fu_act_cfg::instance()->get_cfg(info->act_type_) == NULL)
      continue;

    kai_fu_act_log *new_info = new kai_fu_act_log();
    ::memcpy(new_info, info, sizeof(kai_fu_act_log));
    player->kai_fu_act_log_.push_back(new_info);
  }
}
bool kai_fu_act_obj::had_got_act_award(player_obj *player,
                                       const int act_type,
                                       const int value)
{
  for (ilist_node<kai_fu_act_log *> *itor = player->kai_fu_act_log_.head();
       itor != NULL;
       itor = itor->next_)
    if (itor->value_->act_type_ == act_type
        && itor->value_->value_ == value)
      return true;
  return false;
}
void kai_fu_act_obj::do_update_act_log(player_obj *player,
                                       const int mail_id,
                                       const int act_type,
                                       const int value)
{
  ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(act_type);
  if (lp == NULL) return ;
  if (kai_fu_act_obj::had_got_act_award(player,
                                        act_type,
                                        value))
    return ;

  ilist_node<kai_fu_act_cfg_obj *> *itor = lp->head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_->open_
        && itor->value_->param_ == value)
      break;
  }
  if (itor == NULL) return ;

  kai_fu_act_log *kp = new kai_fu_act_log();
  kp->char_id_ = player->id();
  kp->act_type_ = act_type;
  kp->value_ = value;
  player->kai_fu_act_log_.push_back(kp);
  kai_fu_act_obj::db_insert_log(player, kp);
  kai_fu_act_obj::do_give_act_award(player->id(), mail_id, value, itor->value_->award_list_);
}
void kai_fu_act_obj::on_char_lvl_up(player_obj *player)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;
  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_LVL_UP,
                                    kai_fu_act_cfg::LVL_UP,
                                    player->lvl());
}
void kai_fu_act_obj::on_all_qh_lvl_update(player_obj *player, const int new_qh_lvl)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;

  ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::EQUIP_ALL_QH);
  if (lp == NULL
      || lp->empty())
    return ;

  int arr_param[64] = {0};
  if (lp->size() >= (int)(sizeof(arr_param)/sizeof(arr_param[0])))
  {
    e_log->rinfo("kai fu act equip all qh log is two much!");
    return ;
  }

  int size = lp->size();
  ilist_node<kai_fu_act_cfg_obj *> *itor = lp->head();
  for (size--; itor != NULL; itor = itor->next_, size--)
    arr_param[size] = itor->value_->param_;
  for (int i = lp->size() - 1; i >= 0; i--)
  {
    if (new_qh_lvl >= arr_param[i])
    {
      kai_fu_act_obj::do_update_act_log(player,
                                        mail_config::KAI_FU_ACT_ALL_QH_LVL,
                                        kai_fu_act_cfg::EQUIP_ALL_QH,
                                        arr_param[i]);
    }
  }
}
void kai_fu_act_obj::on_all_equip_color_update(player_obj *player, const int color)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;

  ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::EQUIP_JIN_JIE);
  if (lp == NULL
      || lp->empty())
    return ;

  int arr_param[64] = {0};
  if (lp->size() >= (int)(sizeof(arr_param)/sizeof(arr_param[0])))
  {
    e_log->rinfo("kai fu act equip all qh log is two much!");
    return ;
  }

  int size = lp->size();
  ilist_node<kai_fu_act_cfg_obj *> *itor = lp->head();
  for (size--; itor != NULL; itor = itor->next_, size--)
    arr_param[size] = itor->value_->param_;
  for (int i = lp->size() - 1; i >= 0; i--)
  {
    if (color >= arr_param[i])
    {
      kai_fu_act_obj::do_update_act_log(player,
                                        mail_config::KAI_FU_ACT_ALL_EQUIP_DJ,
                                        kai_fu_act_cfg::EQUIP_JIN_JIE,
                                        arr_param[i]);
    }
  }
}
void kai_fu_act_obj::on_all_bao_shi_lvl_update(player_obj *player, const int all_lvl)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;

  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_ALL_BAO_SHI_LVL,
                                    kai_fu_act_cfg::ALL_BAO_SHI_LVL,
                                    all_lvl);
}
void kai_fu_act_obj::on_all_pskill_lvl_update(player_obj *player, const int all_lvl)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;

  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_ALL_PSKILL_LVL,
                                    kai_fu_act_cfg::ALL_PSKILL_LVL,
                                    all_lvl);
}
void kai_fu_act_obj::on_all_tian_fu_lvl_update(player_obj *player, const int all_lvl)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;

  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_ALL_TIAN_FU_LVL,
                                    kai_fu_act_cfg::ALL_TIAN_FU_LVL,
                                    all_lvl);
}
void kai_fu_act_obj::on_fa_bao_dj_update(player_obj *player, const int dj)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;
  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_ALL_FA_BAO_LVL,
                                    kai_fu_act_cfg::FA_BAO_DJ,
                                    dj);
}
void kai_fu_act_obj::on_tui_tu_end(player_obj *player, const int tt_chapter)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;
  kai_fu_act_obj::do_update_act_log(player,
                                    mail_config::KAI_FU_ACT_TUI_TU_CHAPTER,
                                    kai_fu_act_cfg::TUI_TU_CHAPTER,
                                    tt_chapter);
}
void kai_fu_act_obj::db_insert_log(player_obj *player, kai_fu_act_log *kp)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << stream_ostr((char *)kp, sizeof(kai_fu_act_log));
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_KAI_FU_ACT_LOG, &os);
}
void kai_fu_act_obj::on_zhan_li_update(player_obj *player)
{
  if (!activity_mgr::instance()->is_opened(ACTIVITY_KAI_FU)) return ;
  ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(kai_fu_act_cfg::ZHAN_LI);
  if (lp == NULL
      || lp->empty())
    return ;

  int arr_param[64] = {0};
  if (lp->size() >= (int)(sizeof(arr_param)/sizeof(arr_param[0])))
  {
    e_log->rinfo("kai fu act zhan li log is two much!");
    return ;
  }

  int size = lp->size();
  ilist_node<kai_fu_act_cfg_obj *> *itor = lp->head();
  for (size--; itor != NULL; itor = itor->next_, size--)
    arr_param[size] = itor->value_->param_;
  for (int i = lp->size() - 1; i >= 0; i--)
  {
    if (player->zhan_li() >= arr_param[i])
    {
      kai_fu_act_obj::do_update_act_log(player,
                                        mail_config::KAI_FU_ACT_ZHAN_LI,
                                        kai_fu_act_cfg::ZHAN_LI,
                                        arr_param[i]);
    }
  }
}
int kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(const int id, ilist<int> &id_list)
{
  int n = 1;
  for (ilist_node<int> *itor = id_list.head();
       itor != NULL;
       itor = itor->next_, n++)
  {
    if (itor->value_ == id)
      return n;
  }
  return 0;
}
void kai_fu_act_obj::do_obtain_kai_fu_act_rank(player_obj *player, const int act_type)
{
  ilist<kai_fu_act_cfg_obj *> *lp = kai_fu_act_cfg::instance()->get_cfg(act_type);
  if (lp == NULL) return ;

  int rank = 0;
  char idx = 1;
  static ilist<int> id_list;
  id_list.clear();

  out_stream os(client::send_buf, client::send_buf_len);
  os << act_type;
  if (act_type == kai_fu_act_cfg::RANK_ZHAN_LI)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_ZHANLI,
                                           id_list,
                                           lp->size());
    rank = kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(player->id(), id_list);
    os << rank << (char)id_list.size();
    while (!id_list.empty())
    {
      int id = id_list.pop_front();
      os << idx++;
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(id);
      if (cbi == NULL)
      {
        os << stream_ostr("null", 4) << 0;
      }else
      {
        os << stream_ostr(cbi->name_, ::strlen(cbi->name_));
        player_obj *him = player_mgr::instance()->find(id);
        if (him != NULL)
          os << him->zhan_li();
        else
          os << cbi->zhan_li_;
      }
    }
  }else if (act_type == kai_fu_act_cfg::RANK_LVL)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_LVL,
                                           id_list,
                                           lp->size());
    rank = kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(player->id(), id_list);
    os << rank << (char)id_list.size();
    while (!id_list.empty())
    {
      int id = id_list.pop_front();
      os << idx++;
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(id);
      if (cbi == NULL)
      {
        os << stream_ostr("null", 4) << 0;
      }else
      {
        os << stream_ostr(cbi->name_, ::strlen(cbi->name_));
        player_obj *him = player_mgr::instance()->find(id);
        if (him != NULL)
          os << (int)him->lvl();
        else
          os << (int)cbi->lvl_;
      }
    }
  }else if (act_type == kai_fu_act_cfg::RANK_JING_JI)
  {
    jing_ji_module::get_char_list_before_rank(id_list, lp->size());
    rank = kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(player->id(), id_list);
    os << rank << (char)id_list.size();
    while (!id_list.empty())
    {
      int id = id_list.pop_front();
      os << idx;
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(id);
      if (cbi == NULL)
        os << stream_ostr("null", 4) << (int)idx;
      else
        os << stream_ostr(cbi->name_, ::strlen(cbi->name_)) << (int)idx;
      ++idx;
    }
  }else if (act_type == kai_fu_act_cfg::RANK_TUI_TU)
  {
    rank_module::get_char_list_before_rank(rank_module::RANK_MSTAR,
                                           id_list,
                                           lp->size());
    rank = kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(player->id(), id_list);
    os << rank << (char)id_list.size();
    while (!id_list.empty())
    {
      int id = id_list.pop_front();
      os << idx++;
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(id);
      if (cbi == NULL)
      {
        os << stream_ostr("null", 4) << 0;
      }else
      {
        os << stream_ostr(cbi->name_, ::strlen(cbi->name_));
        player_obj *him = player_mgr::instance()->find(id);
        if (him != NULL)
          os << (int)him->total_mstar();
        else
          os << (int)cbi->total_mstar_;
      }
    }
  }else if (act_type == kai_fu_act_cfg::RANK_GUILD)
  {
    guild_module::get_guild_list_before_rank(id_list, lp->size());
    rank = kai_fu_act_obj::do_obtain_kai_fu_act_rank_i(player->guild_id(), id_list);
    os << rank << (char)id_list.size();
    while (!id_list.empty())
    {
      int id = id_list.pop_front();
      os << idx++;
      const char *gname = guild_module::get_guild_name(id);
      if (gname == NULL)
        os << stream_ostr("null", 4);
      else
        os << stream_ostr(gname, ::strlen(gname));
      os << guild_module::get_guild_zhan_li(id);
    }
  }else
    return ;

  player->send_respond_ok(RES_OBTAIN_KAI_FU_ACT_RANK, &os);
}
void kai_fu_act_obj::do_obtain_kai_fu_act_state(player_obj *player, const int act_type)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << act_type;
  char *cnt = os.wr_ptr();
  *cnt = 0;
  os << *cnt;
  for (ilist_node<kai_fu_act_log *> *itor = player->kai_fu_act_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->act_type_ == act_type)
    {
      ++(*cnt);
      os << itor->value_->value_;
    }
  }
  player->send_respond_ok(RES_OBTAIN_KAI_FU_ACT_STATE, &os);
}
