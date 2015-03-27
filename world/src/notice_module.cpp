#include "notice_module.h"
#include "guild_module.h"
#include "player_mgr.h"
#include "istream.h"
#include "client.h"
#include "message.h"
#include "sys_log.h"
#include "clsid.h"
#include "item_obj.h"

// Lib header

// Defines
#define MAX_MSG_TMP_LEM             512

enum
{
  NOTICE_EQUIP_QH                   = 1,
  NOTICE_EQUIP_RONG_HE              = 2,
  NOTICE_MST_DROP_ITEM              = 3,
  NOTICE_EQUIP_XI_LIAN              = 4,
  NOTICE_BAO_SHI_UP_TO_10TH         = 5,
  NOTICE_GUILD_SUMMON_BOSS          = 6,
  NOTICE_ZHAN_XING                  = 7,
  NOTICE_PASSIVE_SKILL_UPGRADE      = 8,
  NOTICE_GUILD_LVL_UP               = 9,
  NOTICE_GHZ_BEFORE_OPEN            = 10,
  NOTICE_GHZ_SHOU_WEI_DEAD          = 11,
  NOTICE_GHZ_SHOU_WEI_LIVE          = 12,
  NOTICE_GHZ_OVER_WIN               = 13,
  NOTICE_GHZ_OVER_WEI_MIAN          = 14,
  NOTICE_GHZ_OPEN                   = 15,
  NOTICE_CHENG_JIU_LUCKY_EGG        = 16,
  NOTICE_EQUIP_QH_LUCKY_EGG         = 17,
  NOTICE_PASSIVE_SKILL_UPGRADE_LUCKY_EGG = 18,
  NOTICE_BAO_SHI_UPGRADE_LUCKY_EGG  = 19,
  NOTICE_LUCKY_TURN_AWARD           = 20,
  NOTICE_LUCKY_TURN_BIG_AWARD       = 21,
  NOTICE_GUILD_MEM_EXIT             = 22,
  NOTICE_PRODUCE_ITEM               = 23,
  NOTICE_JING_JI_RANK_TOP           = 24,
  NOTICE_TIANFU_SKILL_UPGRADE       = 25,
  NOTICE_JING_JI_WIN                = 26,
  NOTICE_GOT_SEVEN_DAY_LOGIN_AWARD  = 27,
  NOTICE_GOT_VIP_EQUIP_AWARD        = 28,
  NOTICE_GOT_FIRST_RECHARGE_AWARD   = 29,
  NOTICE_WATER_TREE_GOODS_AWARD     = 30,
  NOTICE_LUCKY_GOODS_TURN           = 31,
  NOTICE_MARKET_SALE_ITEM           = 32,
  NOTICE_MARKET_SALE_MONEY          = 33,
};

static ilog_obj *e_log = err_log::instance()->get_ilog("notice");

void notice_module::build_char(const int char_id,
                               const char *name,
                               out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d,%s)", STRING_CHAR, char_id, name);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_guild(const char *name, out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%s)", STRING_GUILD, name);
  os << stream_ostr(msg_tmp, len);
}
int notice_module::build_item_str(const item_obj *item,
                                  char *bf,
                                  const int bf_len)
{
  if (clsid::is_equip(item->cid_))
    return ::snprintf(bf, bf_len, "%s(%d,%d,%s)",
                      STRING_EQUIP,
                      item->cid_,
                      item->bind_,
                      item->extra_info_ == NULL ? "{}" : item->extra_info_->rd_ptr());

  return ::snprintf(bf, bf_len, "%s(%d,%d,%d)", STRING_ITEM, item->cid_, item->bind_, item->amount_);
}
void notice_module::build_item(const item_obj *item, out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = notice_module::build_item_str(item, msg_tmp, sizeof(msg_tmp));
  if (len < 0) return ;
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_number(const int number, out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_NUMBER, number);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_attr(const int attr, out_stream &os)
{
  static char msg_tmp[MAX_MSG_TMP_LEM] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_ATTR, attr);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_scene(const int scene_cid, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_SCENE, scene_cid);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_monster(const int mst_cid, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_MONSTER, mst_cid);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_npc(const int npc_cid, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_NPC, npc_cid);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_pskill(const int skill, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_PSKILL, skill);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_tskill(const int skill, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_TSKILL, skill);
  os << stream_ostr(msg_tmp, len);
}
void notice_module::build_part(const int part, out_stream &os)
{
  static char msg_tmp[256] = {0};
  int len = ::snprintf(msg_tmp, sizeof(msg_tmp), "%s(%d)", STRING_PART, part);
  os << stream_ostr(msg_tmp, len);
}
// ----------------------------------------------------------------------
void notice_module::pick_up_dropped_item(const int char_id,
                                         const char *name,
                                         const int scene_cid,
                                         const int mst_cid,
                                         const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_MST_DROP_ITEM << (char)4;

  notice_module::build_char(char_id, name, os);
  notice_module::build_scene(scene_cid, os);
  notice_module::build_monster(mst_cid, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::equip_strengthen(const int char_id,
                                     const char *name,
                                     const item_obj *item,
                                     const int qh_lvl)
{
  if (qh_lvl % 10 != 0) return ;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_EQUIP_QH << (char)3;

  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  notice_module::build_number(qh_lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::equip_rong_he(const int char_id,
                                  const char *name,
                                  const item_obj *item1,
                                  const item_obj *item2,
                                  const item_obj *new_item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_EQUIP_RONG_HE << (char)4;

  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item1, os);
  notice_module::build_item(item2, os);
  notice_module::build_item(new_item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::equip_xi_lian(const int char_id,
                                  const char *name,
                                  const item_obj *item,
                                  const int star,
                                  const int attr)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_EQUIP_XI_LIAN << (char)4;

  notice_module::build_char(char_id, name, os);

  static char item_bf[MAX_MSG_TMP_LEM] = {0};
  int len = notice_module::build_item_str(item, item_bf, sizeof(item_bf));
  os << stream_ostr(item_bf, len);

  notice_module::build_number(star, os);
  notice_module::build_attr(attr, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::boss_appear(const int , const int )
{ }
void notice_module::ghz_before_opened(const int min)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_BEFORE_OPEN << (char)1;

  notice_module::build_number(min, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::ghz_opened()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_OPEN << (char)0;

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::ghz_shou_wei_dead(const char *guild_name,
                                      const int char_id,
                                      const char *name,
                                      const int idx)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_SHOU_WEI_DEAD << (char)3;

  notice_module::build_number(idx, os);
  notice_module::build_guild(guild_name, os);
  notice_module::build_char(char_id, name, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::ghz_shou_wei_live(const char *guild_name,
                                      const int char_id,
                                      const char *name,
                                      const int idx)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_SHOU_WEI_LIVE << (char)3;

  notice_module::build_guild(guild_name, os);
  notice_module::build_char(char_id, name, os);
  notice_module::build_number(idx, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::ghz_over_win(const char *guild_name,
                                 const int char_id,
                                 const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_OVER_WIN << (char)3;

  notice_module::build_guild(guild_name, os);
  notice_module::build_char(char_id, name, os);
  notice_module::build_guild(guild_name, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::ghz_over_wei_mian(const char *guild_name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GHZ_OVER_WEI_MIAN << (char)1;

  notice_module::build_guild(guild_name, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::guild_summon_boss(const int guild_id,
                                      const int char_id,
                                      const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GUILD_SUMMON_BOSS << (char)1;
  notice_module::build_char(char_id, name, os);

  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(guild_id, NTF_BROADCAST_NOTICE, 0, &mb);
}
void notice_module::zhan_xing(const int char_id, const char *name, const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_ZHAN_XING << (char)2;

  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::passive_skill_upgrade(const int char_id,
                                          const char *name,
                                          const int skill_cid,
                                          const int lvl)
{
  if (lvl % 10 != 0) return ;
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_PASSIVE_SKILL_UPGRADE << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_pskill(skill_cid, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::equip_strengthen_lucky_egg(const int char_id,
                                               const char *name,
                                               const item_obj *item,
                                               const int lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_EQUIP_QH_LUCKY_EGG << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::passive_skill_upgrade_lucky_egg(const int char_id,
                                                    const char *name,
                                                    const int skill_cid,
                                                    const int lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_PASSIVE_SKILL_UPGRADE_LUCKY_EGG << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_pskill(skill_cid, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::cheng_jiu_lucky_egg(const int char_id, const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_CHENG_JIU_LUCKY_EGG << (char)1;
  notice_module::build_char(char_id, name, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::bao_shi_up_to_10th(const int char_id,
                                       const char *name,
                                       const char part,
                                       const short lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_BAO_SHI_UP_TO_10TH << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_part(part, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::bao_shi_upgrade_lucky_egg(const int char_id,
                                              const char *name,
                                              const char part,
                                              const short lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_BAO_SHI_UPGRADE_LUCKY_EGG << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_part(part, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::guild_lvl_up(const int guild_id,
                                 const int char_id,
                                 const char *name,
                                 const short lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GUILD_LVL_UP << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  guild_module::broadcast_to_guild(guild_id, NTF_BROADCAST_NOTICE, 0, &mb);
}
void notice_module::lucky_turn_award(const int char_id,
                                     const char *name,
                                     const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_LUCKY_TURN_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::lucky_turn_big_award(const int char_id,
                                         const char *name,
                                         const int diamond)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_LUCKY_TURN_BIG_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_number(diamond, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::produce_special_item(const int char_id,
                                         const char *name,
                                         const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_PRODUCE_ITEM << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::jing_ji_rank_top(const int char_id, const char *name)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_JING_JI_RANK_TOP << (char)1;
  notice_module::build_char(char_id, name, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::jing_ji_win(const int char_id_1, const char *name_1,
                                const int char_id_2, const char *name_2,
                                const int rank)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_JING_JI_WIN << (char)3;
  notice_module::build_char(char_id_1, name_1, os);
  notice_module::build_number(rank, os);
  notice_module::build_char(char_id_2, name_2, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::got_seven_day_login_award(const int char_id,
                                              const char *name,
                                              const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GOT_SEVEN_DAY_LOGIN_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::got_vip_equip_award(const int char_id,
                                        const char *name,
                                        const char vip_lvl,
                                        const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GOT_VIP_EQUIP_AWARD << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_number(vip_lvl, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::tianfu_skill_upgrade(const int char_id,
                                         const char *name,
                                         const int skill_cid,
                                         const short lvl)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_TIANFU_SKILL_UPGRADE << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_tskill(skill_cid, os);
  notice_module::build_number(lvl, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::got_first_recharge_equip_award(const int char_id,
                                                   const char *name,
                                                   const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_GOT_FIRST_RECHARGE_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::water_tree_goods_award(const int char_id,
                                           const char *name,
                                           const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_WATER_TREE_GOODS_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::lucky_goods_turn(const int char_id,
                                     const char *name,
                                     const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_LUCKY_TURN_AWARD << (char)2;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::market_sale_item(const int char_id,
                                     const char *name,
                                     const int price,
                                     const item_obj *item)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_MARKET_SALE_ITEM << (char)4;
  notice_module::build_char(char_id, name, os);
  notice_module::build_item(item, os);
  notice_module::build_number(item->amount_, os);
  notice_module::build_number(price, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
void notice_module::market_sale_money(const int char_id,
                                      const char *name,
                                      const int value,
                                      const int price)
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  out_stream os(mb.wr_ptr(), mb.space());
  os << (int)NOTICE_MARKET_SALE_MONEY << (char)3;
  notice_module::build_char(char_id, name, os);
  notice_module::build_number(value, os);
  notice_module::build_number(price, os);

  mb.wr_ptr(os.length());
  player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_NOTICE, &mb);
}
