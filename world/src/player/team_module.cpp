#include "team_module.h"
#include "team_mgr.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "all_char_info.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "error.h"
#include "package_module.h"
#include "chat_module.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("team");
static ilog_obj *e_log = err_log::instance()->get_ilog("team");

// static method
int team_module::dispatch_msg(player_obj *player,
                              const int msg_id,
                              const char *msg,
                              const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = team_module::FUNC(player, msg, len);      \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch(msg_id)
  {
    SHORT_CODE(REQ_CREATE_TEAM,      clt_create_team);
    SHORT_CODE(REQ_QUIT_TEAM,        clt_quit_team);
    SHORT_CODE(REQ_KICK_MEMBER,      clt_kick_member);
    SHORT_CODE(REQ_INVITE_BE_A_TEAM, clt_invite_be_a_team);
    SHORT_CODE(REQ_AGREE_INVITE,     clt_agree_invite);
    SHORT_CODE(REQ_REFUSE_INVITE,    clt_refuse_invite);

    SHORT_DEFAULT;
  }
  return ret;
}
int team_module::clt_create_team(player_obj *player, const char *, const int)
{
  if (player->team_id() != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_SELF_HAVE_TEAM);

  team_module::do_create_team(player, NULL);
  return 0;
}
int team_module::clt_kick_member(player_obj *player, const char *msg, const int len)
{
  if (player->team_id() == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_SELF_NOT_HAVE_TEAM);

  int team_id = player->team_id();
  team_info *team = team_mgr::instance()->find(player->team_id());
  if (!team || team->leader() != player->id())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_LEADER);

  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;
  if (char_id == player->id())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (!team->have_player(char_id))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  team->del_member(char_id);
  team_module::broadcast_to_members(team, team_info::DEL_MEMBER, char_id);

  player_obj *del_p = player_mgr::instance()->find(char_id);
  if (del_p != NULL)
  {
    del_p->team_id(0);
    del_p->del_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);
    team_module::on_exit_team(del_p, team_id);
  }
  return 0;
}
int team_module::clt_quit_team(player_obj *player, const char *, const int)
{
  if (player->team_id() == 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_SELF_NOT_HAVE_TEAM);

  int team_id = player->team_id();
  team_info *team = team_mgr::instance()->find(player->team_id());
  if (!team)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_SELF_NOT_HAVE_TEAM);

  team->del_member(player->id());
  player->team_id(0);
  player->del_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);

  if (team->member_count() == 0)
    team_module::do_disband_team(team);
  else if (team->leader() == player->id())
    team_module::do_change_leader(team);

  team_module::broadcast_to_members(team, team_info::LEAVE_TEAM, player->id());
  team_module::on_exit_team(player, team_id);
  return 0;
}
int team_module::clt_invite_be_a_team(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;
  if (char_id == player->id())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  player_obj *aim_p = player_mgr::instance()->find(char_id);
  if (!aim_p)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);

  stream_ostr aim_name(aim_p->name(), ::strlen(aim_p->name()));
  if (player->team_id() != 0 && aim_p->team_id() != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_OPP_HAVE_TEAM);
  else if (player->team_id() != 0)
  {
    // check if i'm the leader
    team_info *team = team_mgr::instance()->find(player->team_id());
    if (team == NULL || team->member_count() == MAX_TEAM_MEMBER)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TEAM_FULL);

    if (team->leader() != player->id())
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_LEADER);
  }
  else if (aim_p->team_id() != 0)
  {
    // get the team's leader
    team_info *team = team_mgr::instance()->find(aim_p->team_id());
    if (team == NULL || team->member_count() == MAX_TEAM_MEMBER)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TEAM_FULL);

    aim_p = player_mgr::instance()->find(team->leader());
    if (aim_p == NULL)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->id()
    << stream_ostr(player->name(), ::strlen(player->name()))
    << player->lvl();
  aim_p->send_request(NTF_INVITE_BE_A_TEAM, &os);

  out_stream my_os(client::send_buf, client::send_buf_len);
  my_os << aim_name;
  return player->send_respond_ok(RES_INVITE_BE_A_TEAM, &my_os);
}
int team_module::clt_agree_invite(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  player_obj *aim_p = player_mgr::instance()->find(char_id);
  if (!aim_p)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PLAYER_IS_OFFLINE);

  if (player->team_id() != 0 && aim_p->team_id() != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_OPP_HAVE_TEAM);
  else if (player->team_id() != 0)
  {
    team_info *team = team_mgr::instance()->find(player->team_id());
    if (team == NULL || team->member_count() == MAX_TEAM_MEMBER)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TEAM_FULL);

    if (team->leader() != player->id())
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_LEADER);

    team_module::do_join_team(team, aim_p);
  }
  else if (aim_p->team_id() != 0)
  {
    team_info *team = team_mgr::instance()->find(aim_p->team_id());
    if (team == NULL || team->member_count() == MAX_TEAM_MEMBER)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TEAM_FULL);

    team_module::do_join_team(team, player);
  }
  else
  {
    team_module::do_create_team(aim_p, player);
  }
  return 0;
}
int team_module::clt_refuse_invite(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  player_obj *inviter = player_mgr::instance()->find(char_id);
  if (inviter != NULL)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << char_id << stream_ostr(player->name(), ::strlen(player->name()));
    inviter->send_request(NTF_REFUSE_INVITE, &os);
  }
  return 0;
}
// ------------------------------------------------------------------------------
void team_module::on_char_login(player_obj *player)
{
  team_info *team = team_mgr::instance()->find_by_char(player->id());
  if (!team) return ;

  player->team_id(team->team_id());

  if (team->leader() == player->id())
    player->add_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);
  else if (player_mgr::instance()->find(team->leader()) != NULL)
    player->add_status(OBJ_IN_TEAM);
  else
  {
    team->set_leader(player->id());
    player->add_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);
  }
}
void team_module::on_char_logout(player_obj *player)
{
  player->del_status(OBJ_TEAM_LEADER|OBJ_IN_TEAM);
  if (player->team_id() != 0)
  {
    team_info *team = team_mgr::instance()->find(player->team_id());
    if (team != NULL && team->leader() == player->id())
      team_module::do_change_leader(team);

    team_module::broadcast_to_members(team, team_info::MEMBER_OFFLINE, player->id());
  }
}
void team_module::on_enter_game(player_obj *player)
{
  if (player->team_id() == 0) return ;
  team_info *team = team_mgr::instance()->find(player->team_id());
  if (team == NULL) return ;

  out_stream os(client::send_buf, client::send_buf_len);
  team_module::build_all_member_info(team, os);
  player->send_request(NTF_ALL_MEMBER_INFO, &os);

  team_module::broadcast_to_members(team, team_info::MEMBER_ONLINE, player->id());
}
void team_module::on_join_team(player_obj *player,
                               const int team_id)
{
  chat_module::on_join_team(player->id(), team_id);
}
void team_module::on_exit_team(player_obj *player,
                               const int team_id)
{
  chat_module::on_exit_team(player->id(), team_id);
}
bool team_module::build_one_member_info(const int member_id,
                                        const char is_leader,
                                        out_stream &os)
{
  player_obj *player = player_mgr::instance()->find(member_id);
  if (player != NULL)
  {
    int zhu_wu_cid = 0, fu_wu_cid = 0;
    package_module::get_equip_for_view(player, zhu_wu_cid, fu_wu_cid);
    os << player->id()
      << stream_ostr(player->name(), ::strlen(player->name()))
      << player->career()
      << player->lvl()
      << player->zhan_li()
      << zhu_wu_cid
      << fu_wu_cid
      << is_leader
      << char(1);
  }
  else
  {
    char_brief_info *cinfo = all_char_info::instance()->get_char_brief_info(member_id);
    if (cinfo == NULL) return false;

    os << cinfo->char_id_
      << stream_ostr(cinfo->name_, ::strlen(cinfo->name_))
      << cinfo->career_
      << cinfo->lvl_
      << cinfo->zhan_li_
      << cinfo->zhu_wu_cid_
      << cinfo->fu_wu_cid_
      << is_leader
      << char(0);
  }
  return true;
}
void team_module::build_all_member_info(team_info *team, out_stream &os)
{
  char *ptr = os.wr_ptr();
  os << char(0);
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (team->mem_id_[i] == 0) continue;

    char is_leader = (team->mem_id_[i] == team->leader() ? char(1) : char(0));
    if (team_module::build_one_member_info(team->mem_id_[i], is_leader, os))
      ++(*ptr);
  }
}
team_info* team_module::do_create_team(player_obj *leader, player_obj *member)
{
  int team_id = team_mgr::instance()->assign_id();
  team_info *team = new team_info(team_id, leader->id());
  team_mgr::instance()->insert(team_id, team);

  leader->team_id(team_id);
  leader->add_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);
  if (member != NULL)
  {
    team->add_member(member->id());
    member->team_id(team->team_id());
    member->add_status(OBJ_IN_TEAM);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  team_module::build_all_member_info(team, os);
  leader->send_request(NTF_ALL_MEMBER_INFO, &os);
  team_module::on_join_team(leader, team_id);
  if (member != NULL)
  {
    member->send_request(NTF_ALL_MEMBER_INFO, &os);
    team_module::on_join_team(member, team_id);
  }

  return team;
}
void team_module::do_join_team(team_info *team, player_obj *player)
{
  team->add_member(player->id());
  player->team_id(team->team_id());
  player->add_status(OBJ_IN_TEAM);

  out_stream os(client::send_buf, client::send_buf_len);
  team_module::build_all_member_info(team, os);
  player->send_request(NTF_ALL_MEMBER_INFO, &os);

  team_module::broadcast_to_members(team, team_info::ADD_MEMBER, player->id());
  team_module::on_join_team(player, player->team_id());
}
void team_module::do_disband_team(team_info *team)
{
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (team->mem_id_[i] == 0) continue;
    player_obj *mem_p = player_mgr::instance()->find(team->mem_id_[i]);
    if (mem_p != NULL)
    {
      mem_p->team_id(0);
      mem_p->del_status(OBJ_IN_TEAM|OBJ_TEAM_LEADER);
    }
  }
  team_mgr::instance()->remove(team->team_id());
  delete team;
}
int team_module::do_change_leader(team_info *team)
{
  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (team->mem_id_[i] == 0
        || team->mem_id_[i] == team->leader())
      continue;
    player_obj *leader = player_mgr::instance()->find(team->mem_id_[i]);
    if (leader != NULL)
    {
      team->set_leader(leader->id());
      leader->add_status(OBJ_TEAM_LEADER);
      team_module::broadcast_to_members(team, team_info::CHANGE_LEADER, 0);
      return 0;
    }
  }
  team->set_leader(0);
  return -1;
}
void team_module::broadcast_to_members(team_info *team,
                                       const int type,
                                       const int char_id)
{
  int message_id = 0, ex_mem = 0;
  out_stream os(client::send_buf, client::send_buf_len);
  switch (type)
  {
  case team_info::MEMBER_ONLINE:
    message_id = NTF_MEMBER_ONLINE;
  case team_info::ADD_MEMBER:
    if (message_id == 0) message_id = NTF_ADD_MEMBER;
    {
      ex_mem = char_id;

      if (! team_module::build_one_member_info(char_id, char(0), os))
        return;
      break;
    }
  case team_info::DEL_MEMBER:
    message_id = NTF_DEL_MEMBER;
  case team_info::LEAVE_TEAM:
    if(message_id == 0) message_id = NTF_LEAVE_TEAM;
    {
      os << char_id;
      player_obj *leave_mem = player_mgr::instance()->find(char_id);
      if (leave_mem != NULL)
        leave_mem->send_request(message_id, &os);
      break;
    }
  case team_info::CHANGE_LEADER:
    message_id = NTF_LEADER_CHANGE;
    os << team->leader();
    break;
  case team_info::MEMBER_OFFLINE:
    message_id = NTF_MEMBER_OFFLINE;
    ex_mem = char_id;
    os << char_id;
    break;
  default:
    return ;
  }

  for (int i = 0; i < MAX_TEAM_MEMBER; ++i)
  {
    if (team->mem_id_[i] == 0 || team->mem_id_[i] == ex_mem) continue;
    player_obj *member = player_mgr::instance()->find(team->mem_id_[i]);
    if (member != NULL)
      member->send_request(message_id, &os);
  }
}
