#include "xszc_activity_obj.h"
#include "activity_cfg.h"
#include "activity_module.h"
#include "activity_mgr.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "global_param_cfg.h"
#include "scp_module.h"
#include "scp_obj.h"
#include "scene_config.h"
#include "time_util.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("act");
static ilog_obj *e_log = err_log::instance()->get_ilog("act");

static coord_t s_group_1_enter_pos;
static coord_t s_group_2_enter_pos;

class xszc_scp_info
{
public:
  xszc_scp_info(const int scene_id) :
    end_(false),
    scene_id_(scene_id)
  { }

  bool end_;
  int scene_id_;
  ilist<int> player_list_;
};

typedef std::map<int/*char_id*/, int/*time*/> enter_act_char_map_t;
typedef std::map<int/*char_id*/, int/*time*/>::iterator enter_act_char_map_iter;
static enter_act_char_map_t s_enter_act_char_map;


int xszc_activity_obj::parse_cfg_param(const char *param)
{
  if (::sscanf(param, "%hd,%hd:%hd,%hd",
               &s_group_1_enter_pos.x_,
               &s_group_1_enter_pos.y_,
               &s_group_2_enter_pos.x_,
               &s_group_2_enter_pos.y_) != 4)
    return -1;
  if (!scene_config::instance()->can_move(global_param_cfg::battle_scene_cid,
                                          s_group_1_enter_pos.x_,
                                          s_group_1_enter_pos.y_)
      || !scene_config::instance()->can_move(global_param_cfg::battle_scene_cid,
                                             s_group_2_enter_pos.x_,
                                             s_group_2_enter_pos.y_))
  {
    e_log->rinfo("xszc enter pos can not move!");
    return -1;
  }
  return 0;
}
scene_coord_t xszc_activity_obj::get_relive_coord(player_obj *player)
{
  if (player->scene_cid() != global_param_cfg::battle_scene_cid)
    return scene_coord_t();

  if (player->group() == GROUP_1)
    return scene_coord_t(global_param_cfg::battle_scene_cid,
                         s_group_1_enter_pos.x_,
                         s_group_1_enter_pos.y_);

  return scene_coord_t(global_param_cfg::battle_scene_cid,
                       s_group_2_enter_pos.x_,
                       s_group_2_enter_pos.y_);
}
void xszc_activity_obj::on_enter_game(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() < activity_cfg::instance()->open_lvl(ACTIVITY_XSZC))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void xszc_activity_obj::on_char_lvl_up(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() != activity_cfg::instance()->open_lvl(ACTIVITY_XSZC))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
int xszc_activity_obj::do_enter_xszc(player_obj *player, const int scene_id)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao == NULL && ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  char bf[16] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_XSZC_ENTER_SCP);
  mb << player->id() << scene_id;
  int ret = 0;
  ao->do_something(&mb, &ret, NULL);

  return ret;
}
int xszc_activity_obj::do_obtain_xszc_act_info(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao == NULL && ao->is_opened())
    return ERR_ACTIVITY_NOT_OPENED;

  char bf[4] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_XSZC_ACT_INFO);

  mblock ret_mb(client::send_buf, client::send_buf_len);
  ao->do_something(&mb, NULL, &ret_mb);

  out_stream os(client::send_buf, client::send_buf_len);
  os.wr_ptr(ret_mb.length());

  return player->send_respond_ok(RES_OBTAIN_XSZC_ACT_INFO, &os);
}
int xszc_activity_obj::do_obtain_xszc_group_pos(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_XSZC);
  if (ao == NULL && ao->is_opened())
    return  ERR_ACTIVITY_NOT_OPENED;

  char bf[16] = {0};
  mblock mb(bf, sizeof(bf));
  mb.data_type(ACT_EV_XSZC_GROUP_POS);
  mb << player->group() << player->scene_id();

  mblock ret_mb(client::send_buf, client::send_buf_len);
  ao->do_something(&mb, NULL, &ret_mb);

  out_stream os(client::send_buf, client::send_buf_len);
  os.wr_ptr(ret_mb.length());

  return player->send_respond_ok(RES_OBTAIN_XSZC_GROUP_POS, &os);
}

xszc_activity_obj::xszc_activity_obj() :
  activity_obj(ACTIVITY_XSZC)
{ }
xszc_activity_obj::~xszc_activity_obj()
{
  while(!this->scp_list_.empty())
    delete this->scp_list_.pop_front();
}
int xszc_activity_obj::open()
{
  while(!this->scp_list_.empty())
    delete this->scp_list_.pop_front();
  s_enter_act_char_map.clear();

  int scene_id = this->construct_xszc_scp();
  if (scene_id < 0) return -1;

  xszc_scp_info *xsi = new xszc_scp_info(scene_id);
  this->scp_list_.push_back(xsi);

  return 0;
}
int xszc_activity_obj::close()
{
  while(!this->scp_list_.empty())
    delete this->scp_list_.pop_front();
  s_enter_act_char_map.clear();
  return 0;
}
void xszc_activity_obj::do_something(mblock *mb, int *ret, mblock *ret_mb)
{
  if (mb->data_type() == ACT_EV_XSZC_ENTER_SCP)
    *ret = this->enter_xszc_scp(mb);
  else if (mb->data_type() == ACT_EV_XSZC_ACT_INFO)
    this->obtain_xszc_act_info(ret_mb);
  else if (mb->data_type() == ACT_EV_XSZC_GROUP_POS)
    this->obtain_xszc_group_pos(mb, ret_mb);
  else if (mb->data_type() == ACT_EV_XSZC_EXIT_SCP)
    this->exit_xszc_scp(mb);
  else if (mb->data_type() == ACT_EV_XSZC_END)
    this->xszc_end(mb);
}
int xszc_activity_obj::construct_xszc_scp()
{
  scp_obj *so = scp_module::construct_scp(global_param_cfg::battle_scene_cid, 0, 0, 0);
  if (so == NULL || so->open() != 0)
  {
    delete so;
    return -1;
  }

  return so->first_scene_id();
}
int xszc_activity_obj::enter_xszc_scp(mblock *mb)
{
  int char_id = 0;
  int scene_id  = 0;
  *mb >> char_id >> scene_id;

  player_obj *player = player_mgr::instance()->find(char_id);
  if (player == NULL)
    return ERR_CLIENT_NETWORK_UNSTABLE;

  ilist_node<xszc_scp_info *> *itor = this->scp_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_->scene_id_ == scene_id)
      break;
  }
  if (itor == NULL || itor->value_->end_)
    return ERR_XSZC_IS_END;
  if (itor->value_->player_list_.size() >= global_param_cfg::each_battle_number)
    return ERR_OVER_PLAYER_COUNT;

  enter_act_char_map_iter time_iter = s_enter_act_char_map.find(char_id);
  if (time_iter != s_enter_act_char_map.end()
      && time_util::now - time_iter->second < global_param_cfg::mid_leave_battle_time)
    return ERR_5_MIN_CANNOT_ENTER;

  this->assign_group(player, itor->value_);

  int ret = player->do_enter_scp(scene_id, global_param_cfg::battle_scene_cid);
  if (ret < 0) return ret;

  if (player->group() == GROUP_1)
    ret = player->do_transfer_to(global_param_cfg::battle_scene_cid,
                                 scene_id,
                                 s_group_1_enter_pos.x_,
                                 s_group_1_enter_pos.y_);
  else
    ret = player->do_transfer_to(global_param_cfg::battle_scene_cid,
                                 scene_id,
                                 s_group_2_enter_pos.x_,
                                 s_group_2_enter_pos.y_);
  if (ret < 0) return ret;


  itor->value_->player_list_.push_back(char_id);

  if (itor->value_->player_list_.size() >= global_param_cfg::each_battle_number
      && this->scp_list_.size() < global_param_cfg::open_battle_room)
  {
    int scene_id = this->construct_xszc_scp();
    if (scene_id < 0) return 0;

    xszc_scp_info *xsi = new xszc_scp_info(scene_id);
    this->scp_list_.push_back(xsi);
  }
  return 0;
}
void xszc_activity_obj::assign_group(player_obj *player,
                                    xszc_scp_info *xsi)
{
  int group_1_zhan_li = 0;
  int group_1_count   = 0;
  int group_2_zhan_li = 0;
  int group_2_count   = 0;
  char group = 0;

  for (ilist_node<int> *itor = xsi->player_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    player_obj *p = player_mgr::instance()->find(itor->value_);
    if (p == NULL) continue;

    if (p->group() == GROUP_1)
    {
      group_1_count++;
      group_1_zhan_li += p->zhan_li();
    }
    else if (p->group() == GROUP_2)
    {
      group_2_count++;
      group_2_zhan_li += p->zhan_li();
    }
  }
  if (group_1_count < group_2_count)
    group = GROUP_1;
  else if (group_1_count > group_2_count)
    group = GROUP_2;
  else
  {
    if (group_1_zhan_li == group_2_zhan_li)
      group = rand() % 2 + 1;
    else if (group_1_zhan_li < group_2_zhan_li)
      group = GROUP_1;
    else if (group_1_zhan_li > group_2_zhan_li)
      group = GROUP_2;
  }

  player->group(group);
}
int xszc_activity_obj::exit_xszc_scp(mblock *mb)
{
  int char_id = 0;
  int scene_id = 0;
  *mb >> char_id >> scene_id;

  ilist_node<xszc_scp_info *> *itor = NULL;
  for (itor = this->scp_list_.head(); itor != NULL; itor = itor->next_)
  {
    if (itor->value_->scene_id_ == scene_id)
      break;
  }
  if (itor == NULL) return 0;

  itor->value_->player_list_.remove(char_id);

  if (!itor->value_->end_)
  {
    enter_act_char_map_iter time_iter = s_enter_act_char_map.find(char_id);
    if (time_iter != s_enter_act_char_map.end())
      time_iter->second = time_util::now;
    else
      s_enter_act_char_map.insert(std::make_pair(char_id, time_util::now));
  }
  return 0;
}
int xszc_activity_obj::xszc_end(mblock *mb)
{
  int scene_id = 0;
  *mb >> scene_id;

  bool open_new = true;
  ilist_node<xszc_scp_info *> *itor = this->scp_list_.head();
  for (; itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->scene_id_ == scene_id)
      itor->value_->end_ = true;
    if (!itor->value_->end_
        && itor->value_->player_list_.size() < global_param_cfg::each_battle_number)
      open_new = false;
  }
  if (itor != NULL)
    itor->value_->player_list_.clear();

  if (open_new)
  {
    int scene_id = this->construct_xszc_scp();
    if (scene_id > 0)
    {
      xszc_scp_info *xsi = new xszc_scp_info(scene_id);
      this->scp_list_.push_back(xsi);
    }
  }

  return 0;
}
void xszc_activity_obj::obtain_xszc_act_info(mblock *ret_mb)
{
  *ret_mb << (char)this->scp_list_.size();
  for (ilist_node<xszc_scp_info *> *itor = this->scp_list_.head();
       itor != NULL;
       itor = itor->next_)
    *ret_mb << itor->value_->scene_id_
      << (char)itor->value_->player_list_.size()
      << (char)(itor->value_->end_ ? 1 : 0);
}
void xszc_activity_obj::obtain_xszc_group_pos(mblock *mb, mblock *ret_mb)
{
  char group = 0;
  int  scene_id = 0;
  *mb >> group >> scene_id;
  
  ilist_node<xszc_scp_info *> *itor = this->scp_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    if (itor->value_->scene_id_ == scene_id)
      break;
  }
  if (itor == NULL) return ;

  char *n = ret_mb->wr_ptr();
  *ret_mb << (char)0;

  for (ilist_node<int> *id_itor = itor->value_->player_list_.head();
       id_itor != NULL;
       id_itor = id_itor->next_)
  {
    player_obj *p = player_mgr::instance()->find(id_itor->value_);
    if (p == NULL || p->group() != group) continue ;
    *ret_mb << id_itor->value_ << p->coord_x() << p->coord_y();
    (*n)++;
  }
}
