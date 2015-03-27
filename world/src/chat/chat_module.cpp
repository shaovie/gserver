#include "chat_module.h"
#include "channel_mgr.h"
#include "global_param_cfg.h"
#include "sys_log.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "client.h"
#include "message.h"
#include "error.h"
#include "time_value.h"
#include "time_util.h"
#include "vip_module.h"
#include "forbid_opt_module.h"
#include "social_module.h"
#include "all_char_info.h"

// Lib header
#include <map>
#include <string>

static ilog_obj *s_log = sys_log::instance()->get_ilog("chat");
static ilog_obj *e_log = err_log::instance()->get_ilog("chat");

const char *g_channel_name[CHANNEL_CNT] =
{
  "null",
  "world",
  "guild",
  "team",
  "private"
};
class chat_info
{
public:
  chat_info()
  {
    ::memset(last_chat_time_, 0, sizeof(last_chat_time_));
  }

  int last_chat_time_[CHANNEL_CNT];
};

typedef std::map<int, chat_info *> chat_info_map;
typedef std::map<int, chat_info *>::iterator chat_info_map_itor;

static chat_info_map s_chat_info;

class chat_log
{
public:
  chat_log() :
    char_id_(0)
  {
    ::memset(this->content_, 0, sizeof(this->content_));
  }
  int char_id_;
  char content_[MAX_CHAT_CONTENT_LEN];
};
static ilist<chat_log *> s_chat_log_list;

class flaunt_item_info
{
public:
  flaunt_item_info() :
    color_(0),
    item_cid_(0),
    time_(0)
  { }
  int color_;
  int item_cid_;
  int time_;
  char name_[64];
  std::string detail_;
};

class flaunt_item_mgr : public singleton<flaunt_item_mgr>
{
  friend class singleton<flaunt_item_mgr>;
public:
  int do_timeout(const time_value &now)
  {
    for (flaunt_item_info_map_itor itor = this->flaunt_item_info_map_.begin();
         itor != this->flaunt_item_info_map_.end();
        )
    {
      if (now.sec() - itor->second->time_ > 3600)
      {
        delete itor->second;
        this->flaunt_item_info_map_.erase(itor++);
      }else
        ++itor;
    }
    return 0;
  }
  void insert(const int item_id, flaunt_item_info *info)
  { this->flaunt_item_info_map_.insert(std::make_pair(item_id, info)); }
  flaunt_item_info* get_flaunt_item_info(const int flaunt_item_id)
  {
    flaunt_item_info_map_itor itor = this->flaunt_item_info_map_.find(flaunt_item_id);
    if (itor == this->flaunt_item_info_map_.end())
      return NULL;
    return itor->second;
  }
  int alloc_flaunt_item_id()
  { return this->flaunt_item_id_++; }
private:
  flaunt_item_mgr() :
    flaunt_item_id_(1)
  { }
private:
  typedef std::map<int, flaunt_item_info *> flaunt_item_info_map_t;
  typedef std::map<int, flaunt_item_info *>::iterator  flaunt_item_info_map_itor;

  int flaunt_item_id_;
  flaunt_item_info_map_t flaunt_item_info_map_;
};

int chat_module::do_timeout(const time_value &now)
{ return flaunt_item_mgr::instance()->do_timeout(now); }
int chat_module::dispatch_msg(player_obj *player,
                              const int msg_id,
                              const char *msg,
                              const int len)
{
#define SHORT_CODE(ID, FUNC) case ID: \
  ret = chat_module::FUNC(player, msg, len); \
  break

#define SHORT_DEFAULT default: \
  e_log->wning("unkown msg %d", msg_id); \
  break

  int ret = 0;

  switch(msg_id)
  {
    SHORT_CODE(REQ_CHAT_WORLD, clt_world_chat);
    SHORT_CODE(REQ_CHAT_GUILD, clt_guild_chat);
    SHORT_CODE(REQ_CHAT_TEAM, clt_team_chat);
    SHORT_CODE(REQ_CHAT_PRIVATE, clt_private_chat);
    SHORT_CODE(REQ_FLAUNT_ITEM, clt_flaunt_item);
    SHORT_CODE(REQ_OBTAIN_FLAUNT_INFO, clt_get_flaunt_item_info);

    SHORT_DEFAULT;
  }
  return ret;
}
void chat_module::on_enter_game(player_obj *player)
{
  channel_mgr::instance()->join(WORLD_CHANNEL, 
                                0,
                                player->id());
  if (player->guild_id() != 0)
    channel_mgr::instance()->join(GUILD_CHANNEL, 
                                  player->guild_id(), 
                                  player->id());
  if (player->team_id() != 0)
    channel_mgr::instance()->join(TEAM_CHANNEL, 
                                  player->team_id(), 
                                  player->id());

  s_chat_info.insert(std::make_pair(player->id(), new chat_info()));

  if (!player->is_reenter_game())
  {
    for (ilist_node<chat_log *> *itor = s_chat_log_list.head();
         itor != NULL;
         itor = itor->next_)
    {
      chat_log *cl = itor->value_;
      char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(cl->char_id_);
      if (cbi == NULL) continue ;
      out_stream os(client::send_buf, client::send_buf_len);
      os << cl->char_id_
        << stream_ostr(cbi->name_, ::strlen(cbi->name_))
        << cbi->vip_
        << stream_ostr(cl->content_, ::strlen(cl->content_));

      player->send_request(NTF_CHAT_WORLD, &os);
    }
  }
  return ;
}
void chat_module::on_char_logout(player_obj *player)
{
  channel_mgr::instance()->quit(WORLD_CHANNEL, 
                                0,
                                player->id());
  if (player->guild_id() != 0)
    channel_mgr::instance()->quit(GUILD_CHANNEL, 
                                  player->guild_id(),
                                  player->id());
  if (player->team_id() != 0)
    channel_mgr::instance()->quit(TEAM_CHANNEL, 
                                  player->team_id(), 
                                  player->id());

  chat_info_map_itor itor = s_chat_info.find(player->id());
  if (itor != s_chat_info.end())
  {
    delete itor->second;
    s_chat_info.erase(player->id());
  }
}
void chat_module::on_join_team(const int char_id, const int team_id)
{
  channel_mgr::instance()->join(TEAM_CHANNEL, team_id, char_id);
}
void chat_module::on_exit_team(const int char_id, const int team_id)
{
  channel_mgr::instance()->quit(TEAM_CHANNEL, team_id, char_id);
}
void chat_module::on_join_guild(const int char_id, const int guild_id)
{
  channel_mgr::instance()->join(GUILD_CHANNEL, guild_id, char_id);
}
void chat_module::on_exit_guild(const int char_id, const int guild_id)
{
  channel_mgr::instance()->quit(GUILD_CHANNEL, guild_id, char_id);
}
int chat_module::clt_world_chat(player_obj *player, const char *msg, const int len)
{
  int ret = chat_module::channel_chat(player,
                                      msg,
                                      len,
                                      WORLD_CHANNEL,
                                      0,
                                      NTF_CHAT_WORLD);
  if (ret < 0)
    return player->send_respond_err(RES_CHAT_WORLD, ret);

  return player->send_respond_ok(RES_CHAT_WORLD);
}
int chat_module::clt_guild_chat(player_obj *player, const char *msg, const int len)
{
  if (player->guild_id() == 0)
    return player->send_respond_err(RES_CHAT_GUILD, ERR_CHAT_NO_GUILD);

  int ret = chat_module::channel_chat(player,
                                      msg,
                                      len,
                                      GUILD_CHANNEL,
                                      player->guild_id(),
                                      NTF_CHAT_GUILD);
  if (ret < 0)
    return player->send_respond_err(RES_CHAT_GUILD, ret);

  return player->send_respond_ok(RES_CHAT_GUILD);
}
int chat_module::clt_team_chat(player_obj *player, const char *msg, const int len)
{
  if (player->team_id() == 0)
    return player->send_respond_err(RES_CHAT_TEAM, ERR_CHAT_NO_TEAM);

  int ret = chat_module::channel_chat(player,
                                      msg,
                                      len,
                                      TEAM_CHANNEL,
                                      player->team_id(),
                                      NTF_CHAT_TEAM);
  if (ret < 0)
    return player->send_respond_err(RES_CHAT_TEAM, ret);

  return player->send_respond_ok(RES_CHAT_TEAM);
}
int chat_module::clt_private_chat(player_obj *player, const char *msg, const int len)
{
  int receiver_id = 0;
  in_stream in(msg, len);
  in >> receiver_id;

  if (player->id() == receiver_id)
    return player->send_respond_err(RES_CHAT_PRIVATE, ERR_CLIENT_OPERATE_ILLEGAL);

  player_obj* receicer_obj = player_mgr::instance()->find(receiver_id);
  if (receicer_obj == NULL)
    return player->send_respond_err(RES_CHAT_PRIVATE, ERR_CHAT_TARGET_OFFLINE);
  if (social_module::is_black(receicer_obj, player->id()))
    return player->send_respond_err(RES_CHAT_PRIVATE, ERR_IN_THE_OTHER_BALCK);

  int ret = chat_module::channel_chat(player,
                                      in.rd_ptr(),
                                      in.length(),
                                      PRIVATE_CHANNEL,
                                      receiver_id,
                                      NTF_CHAT_PRIVATE);
  if (ret < 0)
    return player->send_respond_err(RES_CHAT_PRIVATE, ret);

  return player->send_respond_ok(RES_CHAT_PRIVATE);
}
int chat_module::clt_flaunt_item(player_obj *player, const char *msg, const int len)
{
  int recv_id = 0;
  int channel = 0;
  int cnt = 0;
  char in_bf[MAX_CHAT_CONTENT_LEN] = {0};
  stream_istr content_si(in_bf, sizeof(in_bf));
  in_stream is(msg, len);
  is >> channel
    >> recv_id
    >> content_si
    >> cnt;
  if (cnt > 4
      || content_si.str_len() == 0)
    return player->send_respond_err(RES_FLAUNT_ITEM, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = chat_module::can_chat(player, channel);
  if (ret < 0)
    return player->send_respond_err(RES_FLAUNT_ITEM, ret);

  player_obj *receicer_obj = player_mgr::instance()->find(recv_id);
  if (receicer_obj == NULL
      && channel == PRIVATE_CHANNEL)
    return player->send_respond_err(RES_FLAUNT_ITEM, ERR_CHAT_TARGET_OFFLINE);

  stream_ostr name_so(player->name(), ::strlen(player->name()));
  stream_ostr content_so(in_bf, content_si.str_len());
  out_stream os(client::send_buf, client::send_buf_len);
  os << channel
    << player->id()
    << name_so
    << vip_module::vip_lvl(player)
    << recv_id;
  if (receicer_obj == NULL)
  {
    os << stream_ostr("", 0)
      << char(0);
  }else
  {
    os << stream_ostr(receicer_obj->name(), ::strlen(receicer_obj->name()))
      << vip_module::vip_lvl(receicer_obj);
  }
  os << content_so
    << cnt;
  if (cnt > 0)
  {
    for (int i = 0; i < cnt; ++i)
    {
      flaunt_item_info *fi = new flaunt_item_info();
      fi->time_ = time_util::now;

      stream_istr name_bf_si(fi->name_, sizeof(fi->name_));

      char detail_bf[2048] = {0};
      stream_istr detail_bf_si(detail_bf, sizeof(detail_bf));

      is >> fi->item_cid_
        >> fi->color_
        >> name_bf_si
        >> detail_bf_si;
      fi->detail_ = detail_bf;
      int flaunt_info_id = flaunt_item_mgr::instance()->alloc_flaunt_item_id();
      flaunt_item_mgr::instance()->insert(flaunt_info_id, fi);

      stream_ostr name_bf_so(fi->name_, name_bf_si.str_len());
      os << flaunt_info_id
        << fi->color_
        << name_bf_so
        << fi->item_cid_;
    }
  }

  int arg = 0;
  if (channel == TEAM_CHANNEL)
    arg = player->team_id();
  else if (channel == GUILD_CHANNEL)
    arg = player->guild_id();

  if (channel == PRIVATE_CHANNEL)
  {
    chat_module::send_to_char(player->id(), NTF_CHAT_FLAUNT_ITEM, os);
    chat_module::send_to_char(recv_id, NTF_CHAT_FLAUNT_ITEM, os);
  }else
    channel_mgr::instance()->post(channel,
                                  arg,
                                  NTF_CHAT_FLAUNT_ITEM, 
                                  &os);
  os.reset(client::send_buf, client::send_buf_len);
  os << channel;
  return player->send_respond_ok(RES_FLAUNT_ITEM, &os);
}
int chat_module::clt_get_flaunt_item_info(player_obj *player, const char *msg, const int len)
{
  int flaunt_item_id = 0;
  in_stream is(msg, len);
  is >> flaunt_item_id;
  flaunt_item_info *item_info = flaunt_item_mgr::instance()->get_flaunt_item_info(flaunt_item_id);
  if (item_info == NULL)
    return 0;
  stream_ostr name_bf_so(item_info->name_, ::strlen(item_info->name_));
  stream_ostr detail_so(item_info->detail_.c_str(), item_info->detail_.length());
  out_stream os(client::send_buf, client::send_buf_len);
  os << item_info->item_cid_
    << item_info->color_
    << name_bf_so
    << detail_so;
  return player->send_respond_ok(RES_OBTAIN_FLAUNT_INFO, &os);
}
//arg  when channel is private  so arg = char_id,
//when channel is team or guild or scene  so arg is team_id guild_id or scend_id;
int chat_module::channel_chat(player_obj *player,
                              const char *msg,
                              const int len,
                              const int channel,
                              const int arg,
                              const int ntf_msg_id)
{
  int ret = chat_module::can_chat(player, channel);
  if (ret < 0)
    return ret;

  static char in_bf[MAX_CHAT_CONTENT_LEN] = {0};
  stream_istr content_si(in_bf, sizeof(in_bf));
  in_stream is(msg, len);
  is >> content_si;
  if (content_si.str_len() == 0)
    return ERR_CLIENT_OPERATE_ILLEGAL;

  s_log->rinfo("char:%d[%s] <%s> [%s]",
               player->id(),
               player->name(),
               g_channel_name[channel],
               content_si.str());
  stream_ostr name_so(player->name(), ::strlen(player->name()));
  stream_ostr content_so(in_bf, content_si.str_len());
  out_stream os(client::send_buf, client::send_buf_len);

  if (channel == PRIVATE_CHANNEL)
  {
    player_obj* p = player_mgr::instance()->find(arg);
    if (p != NULL)
    {
      os << arg
        << stream_ostr(p->name(), ::strlen(p->name()))
        << vip_module::vip_lvl(p);
    }
  }
  os << player->id()
    << name_so
    << vip_module::vip_lvl(player)
    << content_so;

  if (channel == WORLD_CHANNEL)
  {
    chat_log *cl = NULL;
    if (s_chat_log_list.size() < global_param_cfg::chat_log_cnt)
      cl = new chat_log();
    else
      cl = s_chat_log_list.pop_front();
    cl->char_id_ = player->id();
    ::memcpy(cl->content_, in_bf, sizeof(in_bf));

    s_chat_log_list.push_back(cl);
  }

  if (channel == PRIVATE_CHANNEL)
  {
    chat_module::send_to_char(player->id(), ntf_msg_id, os);
    chat_module::send_to_char(arg, ntf_msg_id, os);
  }else
    channel_mgr::instance()->post(channel,
                                  arg,
                                  ntf_msg_id,
                                  &os);
  chat_module::on_chat_ok(player, channel);
  return 0;
}
int chat_module::send_to_char(const int recv_id, const int msg_id, out_stream &body)
{
  player_obj* p = player_mgr::instance()->find(recv_id);
  if (p != NULL)
    p->send_request(msg_id, &body);
  return 0;
}
int chat_module::can_chat(player_obj *player, const int channel)
{
  chat_info_map_itor itor = s_chat_info.find(player->id());
  if (itor == s_chat_info.end())
    return ERR_CHAT_CAN_NOT_CHAT;

  if (!forbid_opt_module::can_talk(player->id()))
    return ERR_CHAT_FORBID_TALK;

  if ((time_util::now - itor->second->last_chat_time_[channel])
      < global_param_cfg::chat_channel_cd[channel])
    return ERR_CHAT_TOO_FAST;

  if (player->lvl() < global_param_cfg::chat_lvl_limit)
    return ERR_CHAT_LEVEL_NOT_ENOUGH;

  itor->second->last_chat_time_[channel] = time_util::now;
  return 0;
}
void chat_module::on_chat_ok(player_obj *player, const int channel)
{
  chat_info_map_itor itor = s_chat_info.find(player->id());
  if (itor != s_chat_info.end())
    itor->second->last_chat_time_[channel] = time_util::now;
}
