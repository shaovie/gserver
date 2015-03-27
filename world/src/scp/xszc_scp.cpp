#include "xszc_scp.h"
#include "global_param_cfg.h"
#include "monster_pos_cfg.h"
#include "scp_config.h"
#include "scp_mgr.h"
#include "scp_module.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "activity_mgr.h"
#include "activity_obj.h"
#include "activity_cfg.h"
#include "activity_module.h"
#include "scene_config.h"
#include "spawn_monster.h"
#include "time_util.h"
#include "mail_info.h"
#include "mail_module.h"
#include "mail_config.h"
#include "package_module.h"
#include "battle_notice_module.h"
#include "sys_log.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("scp");
static ilog_obj *e_log = err_log::instance()->get_ilog("scp");

xszc_scp_obj::xszc_scp_obj(const int scene_cid)
  : scp_obj(scene_cid, 0, 0, 0)
{ }
sub_scp_obj *xszc_scp_obj::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new xszc_sub_scp_obj(new_scp_id, scene_cid);
}

xszc_sub_scp_obj::xszc_sub_scp_obj(const int scene_id, const int scene_cid) :
  sub_scp_obj(scene_id, scene_cid),
  status_(S_XSZC_READY),
  last_time_(0),
  last_spawn_mst_time_(0),
  jinwei_kill_count_(0),
  tianzai_kill_count_(0)
{ }
xszc_sub_scp_obj::~xszc_sub_scp_obj()
{
  while (!this->xszc_player_info_list_.empty())
    delete this->xszc_player_info_list_.pop_front();
}
void xszc_sub_scp_obj::enter_sub_scp(const int char_id)
{
  if (this->status_ == S_XSZC_READY)
  {
    this->status_ = S_XSZC_RUNNING;
    this->open_time_ = time_util::now;
  }

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;

  int left_time = this->last_time_ - (time_util::now - this->open_time_)
    - global_param_cfg::over_battle_time;
  if (left_time < 0)
    left_time = 0;

  for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    player_obj *member = player_mgr::instance()->find(iter->value_->char_id_);
    if (member != NULL)
    {
      out_stream g_os(client::send_buf, client::send_buf_len);
      g_os << player->id() << player->group();
      member->send_request(NTF_PLAYER_GROUP, &g_os);
    }
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->group()
    << this->jinwei_kill_count_
    << this->tianzai_kill_count_
    << left_time;
  char *n = os.wr_ptr();
  os << (char)0;
  for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    player_obj *member = player_mgr::instance()->find(iter->value_->char_id_);
    if (member != NULL)
    {
      (*n)++;
      os << player->id() << player->group();
    }
  }

  xszc_player_info *xpi = new xszc_player_info();
  xpi->char_id_ = char_id;
  this->xszc_player_info_list_.push_back(xpi);

  player->send_request(NTF_XSZC_SCP_INFO, &os);

  battle_notice_module::char_enter_battle(this->scene_id_,
                                          char_id,
                                          player->name());
}
void xszc_sub_scp_obj::exit_sub_scp(const int char_id)
{
  int size = this->xszc_player_info_list_.size();
  for (int i = 0; i < size; ++i)
  {
    xszc_player_info *xpi = this->xszc_player_info_list_.pop_front();
    if (xpi->char_id_ == char_id)
    {
      delete xpi;
      break;
    }
    this->xszc_player_info_list_.push_back(xpi);
  }

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player != NULL) player->group(0);

  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao != NULL && ao->is_opened())
  {
    char bf[16] = {0};
    mblock mb(bf, sizeof(bf));
    mb.data_type(ACT_EV_XSZC_EXIT_SCP);
    mb << char_id << this->scene_id_;
    ao->do_something(&mb, NULL, NULL);
  }

  battle_notice_module::char_exit_battle(this->scene_id_,
                                         char_id,
                                         player->name());
}
int xszc_sub_scp_obj::open(scp_obj *master_scp)
{
  this->last_time_ = scp_config::instance()->last_time(master_scp->scp_cid(),
                                                       this->scene_cid_);
  spawn_monster::spawn_scene_monster(this->scene_id_,
                                     this->scene_cid_,
                                     0,
                                     NULL,
                                     NULL);

  return super::open(master_scp);
}
int xszc_sub_scp_obj::run(const int now)
{
  if (now - this->open_time_ > this->last_time_)
  {
    this->master_scp_->put_aev_to_all_char(AEV_KICK_FROM_SCP, NULL);
    activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
    if (ao != NULL && ao->is_opened())
    {
      char bf[8] = {0};
      mblock mb(bf, sizeof(bf));
      mb.data_type(ACT_EV_XSZC_END);
      mb << this->scene_id_;
      ao->do_something(&mb, NULL, NULL);
    }
    return -1;
  }

  if (this->status_ == S_XSZC_RUNNING)
  {
    if (now - this->last_spawn_mst_time_ >= global_param_cfg::battle_mst_fresh_time)
    {
      this->do_spawn_monster();
      this->last_spawn_mst_time_ = now;
    }
    if (now - this->open_time_ >= this->last_time_ - global_param_cfg::over_battle_time)
      this->xszc_end(0, 0);
  }
  return 0;
}
void xszc_sub_scp_obj::do_something(mblock *mb, int *, mblock *)
{
  if (mb->data_type() == SCP_EV_XSZC_BARRACK_DEAD)
  {
    int cid = 0;
    int killer_id = 0;
    *mb >> cid >> killer_id;
    this->dead_barrack_list_.push_back(cid);
    this->add_char_honor(killer_id, global_param_cfg::battle_kill_barracks_honor);
    this->fight_battle_notice(SCP_EV_XSZC_BARRACK_DEAD, killer_id);
  }else if (mb->data_type() == SCP_EV_XSZC_MAIN_DEAD)
  {
    char group = 0;
    int killer_id = 0;
    *mb >> group >> killer_id;
    this->xszc_end(group, killer_id);
  }else if (mb->data_type() == SCP_EV_KILL_SOMEBODY)
  {
    int char_id = 0;
    *mb >> char_id;
    this->add_kill_count(char_id);
    this->add_char_honor(char_id, global_param_cfg::battle_kill_role_honor);
  }else if (mb->data_type() == SCP_EV_XSZC_DEFENDER_DEAD)
  {
    int killer_id = *(int *)mb->rd_ptr();
    this->add_char_honor(killer_id, global_param_cfg::battle_kill_tower_honor);
    this->fight_battle_notice(SCP_EV_XSZC_DEFENDER_DEAD, killer_id);
  }
  else if (mb->data_type() == SCP_EV_XSZC_ARM_DEAD)
  {
    int killer_id = *(int *)mb->rd_ptr();
    this->add_char_honor(killer_id, global_param_cfg::battle_kill_mst_honor);
  }else if (mb->data_type() == SCP_EV_CHAR_DEAD)
  {
    int killer_id = 0;
    int killed_id = 0;
    *mb >> killed_id >> killer_id;
    this->fight_battle_notice(SCP_EV_CHAR_DEAD, killer_id, killed_id);
  }
}
void xszc_sub_scp_obj::do_spawn_monster()
{
  mst_spawn_list *msl = monster_pos_cfg::instance()->get_mst_spawn_list(this->scene_cid_);
  if (msl == NULL) return ;

  for (ilist_node<mst_spawn_list::_info *> *iter = msl->spawn_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    mst_spawn_list::_info *info = iter->value_;
    if (this->dead_barrack_list_.find(info->param_))
      continue;

    ilist_node<coord_t> *idx = info->tar_coord_.head();
    if (idx == NULL || idx->next_ == NULL) continue;
    coord_t target_pos = idx->next_->value_;

    int birth_dir = util::calc_next_dir(idx->value_.x_,
                                        idx->value_.y_,
                                        target_pos.x_,
                                        target_pos.y_);

    for (int i = 0; i < info->monster_cnt_; ++i)
    {
      coord_t birth_pos = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                                   idx->value_.x_,
                                                                   idx->value_.y_,
                                                                   2);

      spawn_monster::spawn_one(iter->value_->monster_cid_,
                               0, 
                               this->scene_id_,
                               this->scene_cid_,
                               birth_dir,
                               birth_pos.x_,
                               birth_pos.y_);
    }
  }
}
void xszc_sub_scp_obj::xszc_end(const int lose_group, const int char_id)
{
  if (this->status_ != S_XSZC_RUNNING) return;

  this->add_char_honor(char_id, global_param_cfg::battle_kill_build_honor);

  this->status_ = S_XSZC_END;
  this->end_award(lose_group);

  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao != NULL && ao->is_opened())
  {
    char bf[8] = {0};
    mblock mb(bf, sizeof(bf));
    mb.data_type(ACT_EV_XSZC_END);
    mb << this->scene_id_;
    ao->do_something(&mb, NULL, NULL);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << lose_group;
  for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    player_obj *player = player_mgr::instance()->find(iter->value_->char_id_);
    if (player == NULL) continue;
    player->send_request(NTF_XSZC_SCP_END, &os);
  }
}
void xszc_sub_scp_obj::add_char_honor(const int char_id, const short honor)
{
  if (this->status_ != S_XSZC_RUNNING) return;

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;

  player->xszc_honor(player->xszc_honor() + honor);
  player->db_save_char_extra_info();
  player->do_notify_xszc_honor_to_clt();

  out_stream os(client::send_buf, client::send_buf_len);
  for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    if (iter->value_->char_id_ == char_id)
    {
      iter->value_->honor_ += honor;
      os << iter->value_->kill_count_ << iter->value_->honor_ << player->xszc_honor();
      player->send_request(NTF_XSZC_PERSON_INFO, &os);
      break;
    }
  }
}
void xszc_sub_scp_obj::add_kill_count(const int char_id)
{
  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL) return ;

  if (player->group() == GROUP_1)
    this->jinwei_kill_count_++;
  else if (player->group() == GROUP_2)
    this->tianzai_kill_count_++;

  for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
       iter != NULL;
       iter = iter->next_)
  {
    if (iter->value_->char_id_ == char_id)
      iter->value_->kill_count_++;

    player_obj *player = player_mgr::instance()->find(char_id);
    if (player == NULL) continue;
    out_stream os(client::send_buf, client::send_buf_len);
    os << this->jinwei_kill_count_ << this->tianzai_kill_count_;
    player->send_request(NTF_UPDATE_KILL_COUNT, &os);
  }
}
void xszc_sub_scp_obj::end_award(const int lose_group)
{
  // 平局
  if (lose_group == 0)
  {
    for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
         iter != NULL;
         iter = iter->next_)
    {
      player_obj *player = player_mgr::instance()->find(iter->value_->char_id_);
      if (player == NULL) continue;
      player->xszc_honor(player->xszc_honor() + global_param_cfg::battle_draw_honor);
      player->db_save_char_extra_info();
      player->do_notify_xszc_honor_to_clt();

      const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::XSZC_DRAW);
      if (mo == NULL) continue;
      item_obj *io = package_module::alloc_new_item(player->id(),
                                                    global_param_cfg::battle_draw_gift,
                                                    1,
                                                    BIND_TYPE);
      item_obj il[MAX_ATTACH_NUM];
      il[0] = *io;
      package_module::release_item(io);
      mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                                mo->sender_name_, mo->title_, mo->content_,
                                mail_info::MAIL_TYPE_XSZC,
                                0, 0, 0,
                                1, il,
                                player->db_sid(), time_util::now);
      mail_module::do_notify_haved_new_mail(player, 1);
    }
  }else
  {
    for (ilist_node<xszc_player_info *> *iter = this->xszc_player_info_list_.head();
         iter != NULL;
         iter = iter->next_)
    {
      player_obj *player = player_mgr::instance()->find(iter->value_->char_id_);
      if (player == NULL) continue;

      int honor   = 0;
      int mail_id = 0;
      int gift    = 0;
      if (lose_group == player->group())
      {
        honor   = global_param_cfg::battle_failure_honor;
        mail_id = mail_config::XSZC_FAIL;
        gift    = global_param_cfg::battle_failure_gift;
      }else
      {
        honor   = global_param_cfg::battle_win_honor;
        mail_id = mail_config::XSZC_WIN;
        gift    = global_param_cfg::battle_win_gift;
      }
      player->xszc_honor(player->xszc_honor() + honor);
      player->db_save_char_extra_info();
      player->do_notify_xszc_honor_to_clt();


      const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_id);
      if (mo == NULL) continue;
      item_obj *io = package_module::alloc_new_item(player->id(),
                                                    gift,
                                                    1,
                                                    BIND_TYPE);
      item_obj il[MAX_ATTACH_NUM];
      il[0] = *io;
      package_module::release_item(io);
      mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                                mo->sender_name_, mo->title_, mo->content_,
                                mail_info::MAIL_TYPE_XSZC,
                                0, 0, 0,
                                1, il,
                                player->db_sid(), time_util::now);
      mail_module::do_notify_haved_new_mail(player, 1);
    }
  }
}
void xszc_sub_scp_obj::fight_battle_notice(const int type, const int killer_id, const int killed_id)
{
  if (type == SCP_EV_XSZC_BARRACK_DEAD
      || type == SCP_EV_XSZC_DEFENDER_DEAD)
  {
    player_obj *player = player_mgr::instance()->find(killer_id);
    if (player == NULL) return ;
    if (player->group() == GROUP_1)
      battle_notice_module::char_kill_tz_build(this->scene_id_, killer_id, player->name());
    else
      battle_notice_module::char_kill_jw_build(this->scene_id_, killer_id, player->name());
  }else if (type == SCP_EV_CHAR_DEAD)
  {
    player_obj *killed = player_mgr::instance()->find(killed_id);
    if (killed == NULL) return ;
    player_obj *killer = player_mgr::instance()->find(killer_id);
    if (killer != NULL)
      battle_notice_module::char_killed_by_char(this->scene_id_,
                                                killed_id,
                                                killed->name(),
                                                killer_id,
                                                killer->name());
    else
    {
      if (killed->group() == GROUP_1)
        battle_notice_module::char_killed_by_tz(this->scene_id_, killed_id, killed->name());
      else
        battle_notice_module::char_killed_by_jw(this->scene_id_, killed_id, killed->name());
    }
  }
}
