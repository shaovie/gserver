#include "battle_notice_module.h"
#include "player_mgr.h"
#include "istream.h"
#include "client.h"
#include "message.h"
#include "sys_log.h"
#include "clsid.h"

// Defines
#define MAX_MSG_TMP_LEM             512

enum
{
  NOTICE_CHAR_ENTER_BATTLE          = 1,
  NOTICE_CHAR_EXIT_BATTLE           = 2,
  NOTICE_CHAR_KILLED_BY_CHAR        = 3,
  NOTICE_CHAR_KILL_TZ_BUILD         = 4,
  NOTICE_CHAR_KILL_JW_BUILD         = 5,
  NOTICE_CHAR_KILLED_BY_TZ          = 6,
  NOTICE_CHAR_KILLED_BY_JW          = 7,
};

void battle_notice_module::build_char(const int char_id,
                                      const char *name,
                                      out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d,%s)", STRING_CHAR, char_id, name);
  os << stream_ostr(msg_tmp, len);
}

void battle_notice_module::char_enter_battle(const int scene_id,
                                             const int char_id,
                                             const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_ENTER_BATTLE << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_exit_battle(const int scene_id,
                                            const int char_id,
                                            const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_EXIT_BATTLE << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_killed_by_char(const int scene_id,
                                               const int char_id_1,
                                               const char *name_1,
                                               const int char_id_2,
                                               const char *name_2)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_KILLED_BY_CHAR << (char)2;

  battle_notice_module::build_char(char_id_1, name_1, os);
  battle_notice_module::build_char(char_id_2, name_2, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_kill_tz_build(const int scene_id,
                                              const int char_id,
                                              const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_KILL_TZ_BUILD << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_kill_jw_build(const int scene_id,
                                              const int char_id,
                                              const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_KILL_JW_BUILD << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_killed_by_tz(const int scene_id,
                                             const int char_id,
                                             const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_KILLED_BY_TZ << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
void battle_notice_module::char_killed_by_jw(const int scene_id,
                                             const int char_id,
                                             const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHAR_KILLED_BY_JW << (char)1;

  battle_notice_module::build_char(char_id, name, os);
  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_scene(scene_id, NTF_BROADCAST_BATTLE_NOTICE, &mb);
}
