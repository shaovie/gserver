#include "team_module.h"
#include "istream.h"
#include "player.h"
#include "message.h"
#include "scene_unit_obj.h"
#include "sys_log.h"
#ifdef FOR_ONEROBOT
#include "director.h"
#endif

static ilog_obj *s_log = sys_log::instance()->get_ilog("team");
static ilog_obj *e_log = err_log::instance()->get_ilog("team");

/**
 * @class team_info
 *
 * @brief
 */
class team_info
{
public:
  ~team_info()
  {
    while (!this->member_list_.empty())
    {
      team_info::_member *tim = this->member_list_.pop_front();
      if (tim != NULL) delete tim;
    }
  }

  class _member
  {
  public:
    _member()
      : career_(0),
      online_(1),
      isleader_(0),
      lvl_(0),
      char_id_(0),
      zhan_li_(0),
      zhu_wu_(0),
      fu_wu_(0)
    {
      ::memset(this->name_, 0, sizeof(this->name_));
    }

    char career_;
    char online_;
    char isleader_;
    short lvl_;
    int char_id_;
    int zhan_li_;
    int zhu_wu_;
    int fu_wu_;
    char name_[MAX_NAME_LEN + 1];
  };

  ilist<_member *> member_list_;
};

void clear_team_member(team_info *ti)
{
  while (!ti->member_list_.empty())
  {
    team_info::_member *tim = ti->member_list_.pop_front();
    if (tim != NULL)
      delete tim;
  }
}

void team_module::init(player *p)
{
  p->team_info_ = new team_info();
}
void team_module::destroy(player *p)
{
  delete p->team_info_;
}
int team_module::dispatch_msg(player *p, const int id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = team_module::FUNC(p, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", id);       \
  break

  int ret = 0;
  switch(id)
  {
    SHORT_CODE(NTF_ALL_MEMBER_INFO,         all_member_info);
    SHORT_CODE(NTF_ADD_MEMBER,              add_member);
    SHORT_CODE(NTF_DEL_MEMBER,              del_member);
    SHORT_CODE(NTF_LEAVE_TEAM,              leave_team);
    SHORT_CODE(NTF_LEADER_CHANGE,           leader_change);
    SHORT_CODE(NTF_MEMBER_ONLINE,           member_online);
    SHORT_CODE(NTF_MEMBER_OFFLINE,          member_offline);
    SHORT_CODE(NTF_INVITE_BE_A_TEAM,        invite_be_a_team);

    //SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle social msg %d return %d", id, ret);
  return ret;
}
int team_module::invite_be_a_team(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  out_stream os(client::send_buf, client::send_buf_len);
  os << char_id;
  // 0:refuse 1:agree
  if (rand() % 2 == 0)
    p->send_request(REQ_REFUSE_INVITE, &os);
  else
    p->send_request(REQ_AGREE_INVITE, &os);

  return 0;
}
int team_module::all_member_info(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  char cnt = 0;
  is >> cnt;

  clear_team_member(p->team_info_);
  for (int i = 0; i < cnt; ++i)
  {
    team_info::_member *tim = new team_info::_member();
    stream_istr name_si(tim->name_, sizeof(tim->name_));
    is >> tim->char_id_
      >> name_si
      >> tim->career_
      >> tim->lvl_
      >> tim->zhan_li_
      >> tim->zhu_wu_
      >> tim->fu_wu_
      >> tim->isleader_
      >> tim->online_; 

    p->team_info_->member_list_.push_back(tim);
#ifdef FOR_ONEROBOT
    director::instance()->add_teamer(tim->char_id_, tim->name_);
#endif
  }
  return 0;
}
int team_module::add_member(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  team_info::_member *tim = new team_info::_member();
  stream_istr name_si(tim->name_, sizeof(tim->name_));
  is >> tim->char_id_
    >> name_si
    >> tim->career_
    >> tim->lvl_
    >> tim->zhan_li_
    >> tim->zhu_wu_
    >> tim->fu_wu_
    >> tim->isleader_
    >> tim->online_; 

  p->team_info_->member_list_.push_back(tim);
#ifdef FOR_ONEROBOT
  director::instance()->add_teamer(tim->char_id_, tim->name_);
#endif
  return 0;
}
int team_module::del_member(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  int size = p->team_info_->member_list_.size();
  for (int i = 0; i < size; ++i)
  {
    team_info::_member *tim = p->team_info_->member_list_.pop_front();
    if (tim->char_id_ == char_id)
    {
#ifdef FOR_ONEROBOT
      director::instance()->delet_teamer(char_id);
#endif
      delete tim;
      break;
    }
    p->team_info_->member_list_.push_back(tim);
  }

  return 0;
}
int team_module::leave_team(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  int size = p->team_info_->member_list_.size();
  for (int i = 0; i < size; ++i)
  {
    team_info::_member *tim = p->team_info_->member_list_.pop_front();
    if (tim->char_id_ == char_id)
    {
#ifdef FOR_ONEROBOT
      director::instance()->delet_teamer(char_id);
#endif
      delete tim;
      break;
    }
    p->team_info_->member_list_.push_back(tim);
  }

  return 0;
}
int team_module::leader_change(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int leader_id = 0;
  is >> leader_id;

  for (ilist_node<team_info::_member *> *itor = p->team_info_->member_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    team_info::_member *tim = itor->value_;
    if (tim->char_id_ == leader_id)
      tim->isleader_ = 1;
    else
      tim->isleader_ = 0;
  }

  return 0;
}
int team_module::member_online(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  for (ilist_node<team_info::_member *> *itor = p->team_info_->member_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    team_info::_member *tim = itor->value_;
    if (tim->char_id_ != char_id) continue;

    stream_istr name_si(tim->name_, sizeof(tim->name_));
    is >> name_si
      >> tim->career_
      >> tim->lvl_
      >> tim->zhan_li_
      >> tim->zhu_wu_
      >> tim->fu_wu_
      >> tim->isleader_
      >> tim->online_; 
  }

  return 0;
}
int team_module::member_offline(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  for (ilist_node<team_info::_member *> *itor = p->team_info_->member_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    team_info::_member *tim = itor->value_;
    if (tim->char_id_ != char_id) continue;

    tim->online_ = 0;
  }

  return 0;
}
void team_module::create_team(player *p)
{ p->send_request(REQ_CREATE_TEAM, NULL); }
void team_module::kick_member(player *p)
{
  if (p->team_info_->member_list_.empty()) return;

  int kick_id = 0;
  for (ilist_node<team_info::_member *> *itor = p->team_info_->member_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    team_info::_member *tim = itor->value_;
    if (tim->char_id_ != p->id_)
    {
      kick_id = tim->char_id_;
      break;
    }
  }

  if (kick_id == 0) return;

  out_stream os(client::send_buf, client::send_buf_len);
  os << kick_id;
  p->send_request(REQ_KICK_MEMBER, &os);
}
void team_module::quit_team(player *p)
{ p->send_request(REQ_QUIT_TEAM, NULL); }
void team_module::invite_member(player *p)
{
  scene_unit_obj *su = p->get_random_scene_unit(scene_unit_obj::PLAYER, 0, 100);
  if (su == NULL) return ;

  out_stream os(client::send_buf, client::send_buf_len);
  os << su->id_;
  p->send_request(REQ_INVITE_BE_A_TEAM, &os);
}
bool team_module::team_empty(player *p)
{ return p->team_info_->member_list_.empty(); }
