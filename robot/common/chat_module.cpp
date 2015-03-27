#include "chat_module.h"
#include "istream.h"
#include "player.h"
#include "message.h"
#include "sys_log.h"
#include "scene_unit_obj.h"
#ifdef FOR_ONEROBOT
#include "director.h"
#endif

// Lib header
#include<string>

static ilog_obj *s_log = sys_log::instance()->get_ilog("chat");
static ilog_obj *e_log = err_log::instance()->get_ilog("chat");

int chat_module::dispatch_msg(player *p, const int id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = chat_module::FUNC(p, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", id);       \
  break

  int ret = 0;
  switch(id)
  {
    SHORT_CODE(NTF_CHAT_WORLD,              recv_world_chat);
    SHORT_CODE(NTF_CHAT_TEAM,               recv_team_chat);
    SHORT_CODE(NTF_CHAT_PRIVATE,            recv_private_chat);
    SHORT_CODE(NTF_CHAT_SCENE,              recv_scene_chat);
    SHORT_CODE(NTF_CHAT_FLAUNT_ITEM,        recv_scene_chat); 
    //SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle social msg %d return %d", id, ret);
  return ret;
}

int chat_module::world_chat(player *p)
{
  std::string send_msg("word");
  send_msg.append(512, 'w');
  int num = rand() % 512;
  return chat_module::channel_chat(p, REQ_CHAT_WORLD, send_msg.c_str(), num);
}

int chat_module::team_chat(player *p)
{
  std::string send_msg("team");
  send_msg.append(512, 't');
  int num = rand() % 512;
  return chat_module::channel_chat(p, REQ_CHAT_TEAM, send_msg.c_str(), num);
}

int chat_module::scene_chat(player *p)
{
  std::string send_msg("scene");
  send_msg.append(512, 's');
  int num = rand() % 512;
  return chat_module::channel_chat(p, REQ_CHAT_SCENE, send_msg.c_str(), num);
}

int chat_module::private_chat(player *p)
{
  scene_unit_obj *su = p->get_random_scene_unit(scene_unit_obj::PLAYER, 0, 100);
  if (su == NULL) return 0;

  if (su->id_ == 0)
  {
    s_log->wning("error get_random char id == 0");
    return 0;
  }

  std::string send_msg("private");
  send_msg.append(512, 'p');
  int num = rand() % 512;

  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr is(send_msg.c_str(), num);
  os << su->id_ << is;
  return p->send_request(REQ_CHAT_PRIVATE, &os);
}
int chat_module::flaunt_item(player*)
{
  return 0;
}
//-----------------------------------private------------------------------------------
int chat_module::channel_chat(player *p, const int msg_id, const char *msg, const int len)
{
  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr is(msg, len);
  os << is;
  return p->send_request(msg_id, &os);
}
int chat_module::recv_world_chat(player *, const char *msg, const int len)
{
  in_stream in(msg, len);

  int char_id = 0;
  char name[MAX_NAME_LEN] = {0};
  char chat_content[MAX_CHAT_CONTENT_LEN] = {0};

  stream_istr name_s(name, sizeof(name));
  stream_istr content(chat_content, sizeof(chat_content));
  in >> char_id >> name_s >> content;
  //e_log->wning("char_id: %d, name: %s, content: %s", char_id, name, chat_content);
#ifdef FOR_ONEROBOT
  director::instance()->chat(name, chat_content);
#endif
  return 0;
}
int chat_module::recv_private_chat(player *, const char *, const int)
{
  return 0;
}
int chat_module::recv_team_chat(player *, const char *, const int)
{
  //e_log->wning("receive team msg");
  return 0;
}
int chat_module::recv_scene_chat(player *, const char *, const int)
{
  return 0;
}
int chat_module::recv_flaunt_item(player *, const char *, const int)
{
  //e_log->wning("receive scene msg");
  return 0;
}
