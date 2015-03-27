#include "cheng_jiu_module.h"
#include "cheng_jiu_info.h"
#include "cheng_jiu_cfg.h"
#include "package_module.h"
#include "lucky_egg_module.h"
#include "client.h"
#include "player_obj.h"
#include "package_module.h"
#include "title_module.h"
#include "db_proxy.h"
#include "message.h"
#include "behavior_id.h"
#include "def.h"
#include "error.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("cheng_jiu");
static ilog_obj *e_log = err_log::instance()->get_ilog("cheng_jiu");

typedef std::map<int/*cheng jiu type*/, cheng_jiu_info *> cheng_jiu_map_t;
typedef std::map<int/*cheng jiu type*/, cheng_jiu_info *>::iterator cheng_jiu_map_itor;
class cheng_jiu_data
{
public:
  cheng_jiu_data() { };
  ~cheng_jiu_data()
  {
    for (cheng_jiu_map_itor itor = this->data_.begin();
         itor != this->data_.end();
         ++itor)
      cheng_jiu_info_pool::instance()->release(itor->second);
  }
  cheng_jiu_map_t data_;
};
void cheng_jiu_module::init(player_obj *player)
{
  player->cheng_jiu_data_ = new cheng_jiu_data();
}
void cheng_jiu_module::destroy(player_obj *player)
{
  delete player->cheng_jiu_data_;
  player->cheng_jiu_data_ = NULL;
}
void cheng_jiu_module::handle_db_get_cheng_jiu_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;

  for (int i = 0; i < res_cnt; ++i)
  {
    char buff_bf[sizeof(cheng_jiu_info) + 4] = {0};
    stream_istr buff_si(buff_bf, sizeof(buff_bf));
    is >> buff_si;

    cheng_jiu_info *cji = cheng_jiu_info_pool::instance()->alloc();
    ::memcpy(cji, buff_bf, sizeof(cheng_jiu_info));
    player->cheng_jiu_data_->data_.insert(std::make_pair(cji->type_, cji));
  }
}
void cheng_jiu_module::on_enter_game(player_obj *player)
{
  cheng_jiu_module::on_continue_login_day(player,
                                          player->daily_clean_info_->con_login_days_);
#if 0
  for (cheng_jiu_map_itor itor = player->cheng_jiu_data_->data_.begin();
       itor != player->cheng_jiu_data_->data_.end();
       ++itor)
  {
    if (cheng_jiu_module::check_cheng_jiu_complete(player, itor->second))
    {
      mblock mb(client::send_buf, client::send_buf_len);
      mb.wr_ptr(sizeof(proto_head));
      player->do_delivery(NTF_CAN_GET_CHENG_JIU_AWARD, &mb);
      break;
    }
  }
#else
  ilist<int> id_list;
  for (cheng_jiu_map_itor itor = player->cheng_jiu_data_->data_.begin();
       itor != player->cheng_jiu_data_->data_.end();
       ++itor)
  {
    if (cheng_jiu_module::check_cheng_jiu_complete(player, itor->second))
      id_list.push_back(itor->second->id_);
  }
  if (!id_list.empty())
  {
    mblock mb(client::send_buf, client::send_buf_len);
    mb.wr_ptr(sizeof(proto_head));
    mb << id_list.size();
    while(!id_list.empty())
      mb << id_list.pop_front();
    player->do_delivery(NTF_CAN_GET_CHENG_JIU_AWARD, &mb);
  }
#endif
}
cheng_jiu_info *cheng_jiu_module::find_cj_info(player_obj *player, const int type)
{
  cheng_jiu_map_itor itor = player->cheng_jiu_data_->data_.find(type);
  if (itor == player->cheng_jiu_data_->data_.end()) return NULL;
  return itor->second;
}
bool cheng_jiu_module::check_cheng_jiu_complete(player_obj *player, cheng_jiu_info *cji)
{
  cheng_jiu_obj *cjo = cheng_jiu_cfg::instance()->get_cheng_jiu_obj(cji->id_);
  if (cjo == NULL
      || cji->value_ < cjo->value_
      || player->lvl() < cjo->show_lvl_)
    return false;

  return true;
}
void cheng_jiu_module::notify_cheng_jiu_complete(player_obj *player, const int id)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << id;
  player->send_request(NTF_COMPLETE_CHENG_JIU, &os);
}
void cheng_jiu_module::give_cheng_jiu_title(player_obj *player, const int id)
{
  cheng_jiu_obj *cjo = cheng_jiu_cfg::instance()->get_cheng_jiu_obj(id);
  if (cjo == NULL || cjo->title_ == 0) return; 

  title_module::add_new_title(player, cjo->title_);
}
void cheng_jiu_module::update_cheng_jiu_2_db(player_obj *player, cheng_jiu_info *cji)
{
  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << stream_ostr((char *)cji, sizeof(cheng_jiu_info));
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_CHENG_JIU_INFO, &db_os);
}
cheng_jiu_info *cheng_jiu_module::new_cheng_jiu(player_obj *player,
                                                const int type)
{
  // 规定每类成就第一个id为001
  int id = type * 1000 + 1;
  if (cheng_jiu_cfg::instance()->get_cheng_jiu_obj(id) == NULL)
    return NULL;

  cheng_jiu_info *cji = cheng_jiu_info_pool::instance()->alloc();
  cji->char_id_ = player->id();
  cji->id_      = id;
  cji->type_    = type;

  player->cheng_jiu_data_->data_.insert(std::make_pair(type, cji));
  return cji;
}
void cheng_jiu_module::on_char_lvl_up(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_LEVEL);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_LEVEL);

  if (cji == NULL || cji->value_ >= player->lvl()) return;

  cji->value_ = player->lvl();
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_zhan_li_update(player_obj *player, const int zhan_li)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_FIGHT_SCORE);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_FIGHT_SCORE);

  if (cji == NULL || cji->value_ >= zhan_li) return;

  cji->value_ = zhan_li;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_equip_qh(player_obj *player, const int part, const int qh)
{
  cheng_jiu_info *cji = NULL;
  switch (part)
  {
  case PART_ZHU_WU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_ZHU_WU_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_ZHU_WU_QH);
    break;
  case PART_FU_WU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_FU_WU_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_FU_WU_QH);
    break;
  case PART_YI_FU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_YI_FU_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_YI_FU_QH);
    break;
  case PART_KU_ZI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_KU_ZI_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_KU_ZI_QH);
    break;
  case PART_XIE_ZI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_XIE_ZI_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_XIE_ZI_QH);
    break;
  case PART_SHOU_TAO:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SHOU_TAO_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SHOU_TAO_QH);
    break;
  case PART_XIANG_LIAN:
    cji = cheng_jiu_module::find_cj_info(player, CJ_XIANG_LIAN_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_XIANG_LIAN_QH);
    break;
  case PART_JIE_ZHI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_JIE_ZHI_QH);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_JIE_ZHI_QH);
    break;
  default:
    return;
  }
  if (cji == NULL || cji->value_ >= qh) return;

  cji->value_ = qh;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_passive_skill_level_up(player_obj *player,
                                                 const int effect_id,
                                                 const int lvl)
{
  cheng_jiu_info *cji = NULL;
  switch (effect_id)
  {
  case ATTR_T_HP:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_HP_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_HP_LVL);
    break;
  case ATTR_T_GONG_JI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_GONG_JI_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_GONG_JI_LVL);
    break;
  case ATTR_T_FANG_YU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_FANG_YU_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_FANG_YU_LVL);
    break;
  case ATTR_T_MING_ZHONG:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_MING_ZHONG_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_MING_ZHONG_LVL);
    break;
  case ATTR_T_SHAN_BI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_SHAN_BI_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_SHAN_BI_LVL);
    break;
  case ATTR_T_BAO_JI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_BAO_JI_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_BAO_JI_LVL);
    break;
  case ATTR_T_KANG_BAO:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_KANG_BAO_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_KANG_BAO_LVL);
    break;
  case ATTR_T_SHANG_MIAN:
    cji = cheng_jiu_module::find_cj_info(player, CJ_P_SK_SHANG_MIAN_LVL);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_P_SK_SHANG_MIAN_LVL);
    break;
  default:
    return;
  }

  if (cji == NULL || cji->value_ >= lvl) return;

  cji->value_ = lvl;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_friend_count(player_obj *player, const int count)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_FRIEND_COUNT);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_FRIEND_COUNT);

  if (cji == NULL || cji->value_ >= count) return;

  cji->value_ = count;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_equip_fen_jie(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_EQUIP_FEN_JIE);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_EQUIP_FEN_JIE);

  if (cji == NULL) return;

  cji->value_ += 1;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_equip_xi_lian(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_EQUIP_XI_LIAN);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_EQUIP_XI_LIAN);

  if (cji == NULL) return;

  cji->value_ += 1;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_complete_tui_tu(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_COMPLETE_TUI_TU);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_COMPLETE_TUI_TU);

  if (cji == NULL) return;

  cji->value_ += 1;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_zhan_xing(player_obj *player, const int count)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_ZHAN_XING);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_ZHAN_XING);

  if (cji == NULL) return;

  cji->value_ += count;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_guild_contrib(player_obj *player, const int contrib)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_GUILD_CONTRIB);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_GUILD_CONTRIB);

  if (cji == NULL || cji->value_ >= contrib) return;

  cji->value_ = contrib;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_kill_char(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_KILL_CHAR);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_KILL_CHAR);

  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_coin_value(player_obj *player, const int coin)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_COIN_VALUE);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_COIN_VALUE);

  if (cji == NULL || cji->value_ >= coin) return;

  cji->value_ = coin;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_vip_level(player_obj *player, const int vip)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_VIP_LEVEL);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_VIP_LEVEL);

  if (cji == NULL || cji->value_ >= vip) return;

  cji->value_ = vip;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_online_time(player_obj *player, const int time)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_ONLINE_TIME);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_ONLINE_TIME);

  // 小时计算
  int hour = time / 3600;
  if (cji == NULL || cji->value_ >= hour) return;

  cji->value_ = hour;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_continue_login_day(player_obj *player, const int day)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_CONTINUE_LOGIN_DAY);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_CONTINUE_LOGIN_DAY);

  if (cji == NULL || cji->value_ >= day) return;

  cji->value_ = day;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_bao_shi_lvl(player_obj *player, const int part, const int lvl)
{
  cheng_jiu_info *cji = NULL;
  switch (part)
  {
  case PART_ZHU_WU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_ZHU_WU_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_ZHU_WU_BAO_SHI);
    break;
  case PART_FU_WU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_FU_WU_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_FU_WU_BAO_SHI);
    break;
  case PART_YI_FU:
    cji = cheng_jiu_module::find_cj_info(player, CJ_YI_FU_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_YI_FU_BAO_SHI);
    break;
  case PART_KU_ZI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_KU_ZI_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_KU_ZI_BAO_SHI);
    break;
  case PART_XIE_ZI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_XIE_ZI_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_XIE_ZI_BAO_SHI);
    break;
  case PART_SHOU_TAO:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SHOU_TAO_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SHOU_TAO_BAO_SHI);
    break;
  case PART_XIANG_LIAN:
    cji = cheng_jiu_module::find_cj_info(player, CJ_XIANG_LIAN_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_XIANG_LIAN_BAO_SHI);
    break;
  case PART_JIE_ZHI:
    cji = cheng_jiu_module::find_cj_info(player, CJ_JIE_ZHI_BAO_SHI);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_JIE_ZHI_BAO_SHI);
    break;
  default:
    return;
  }
  if (cji == NULL || cji->value_ >= lvl) return;

  cji->value_ = lvl;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_enter_scp(player_obj *player, const int scene_cid)
{
  cheng_jiu_info *cji = NULL;
  switch (scene_cid)
  {
  case 3101:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SCP_3101);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SCP_3101);
    break;
  case 3102:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SCP_3102);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SCP_3102);
    break;
  case 3103:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SCP_3103);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SCP_3103);
    break;
  case 3104:
    cji = cheng_jiu_module::find_cj_info(player, CJ_SCP_3104);
    if (cji == NULL)
      cji = cheng_jiu_module::new_cheng_jiu(player, CJ_SCP_3104);
    break;
  default:
    return;
  }
  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_jing_ji(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_JING_JI);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_JING_JI);

  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_guild_ju_bao(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_GUILD_JU_BAO);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_GUILD_JU_BAO);

  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_mo_bai(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_MO_BAI);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_MO_BAI);

  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}
void cheng_jiu_module::on_water_tree(player_obj *player)
{
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, CJ_WATER_TREE);
  if (cji == NULL)
    cji = cheng_jiu_module::new_cheng_jiu(player, CJ_WATER_TREE);

  if (cji == NULL) return;

  cji->value_++;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);
}

//===================================================================================
int cheng_jiu_module::dispatch_msg(player_obj *player,
                                   const int msg_id,
                                   const char *msg,
                                   const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = cheng_jiu_module::FUNC(player, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  if (player->social_relation_ == NULL) return 0;

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GET_CHENG_JIU_LIST,    clt_get_cheng_jiu_list);
    SHORT_CODE(REQ_GET_CHENG_JIU_AWARD,   clt_get_cheng_jiu_award);

    SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle cheng jiu msg %d return %d", msg_id, ret);
  return ret;
}
int cheng_jiu_module::clt_get_cheng_jiu_list(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << (int)player->cheng_jiu_data_->data_.size();
  for (cheng_jiu_map_itor itor = player->cheng_jiu_data_->data_.begin();
       itor != player->cheng_jiu_data_->data_.end();
       ++itor)
    os << itor->second->id_ << itor->second->value_;

  return player->send_request(RES_GET_CHENG_JIU_LIST, &os);
}
int cheng_jiu_module::clt_get_cheng_jiu_award(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int cj_id = 0;
  is >> cj_id;

  int type = cj_id / 1000;
  cheng_jiu_info *cji = cheng_jiu_module::find_cj_info(player, type);
  if (cji == NULL || cji->id_ != cj_id)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_OPERATION_TOO_FAST);

  cheng_jiu_obj *cjo = cheng_jiu_cfg::instance()->get_cheng_jiu_obj(cj_id);
  if (cjo == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  if (cji->value_ < cjo->value_)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CHENG_JIU_NOT_ENOUGH);

  int ret = package_module::do_insert_award_item_list(player,
                                                      &cjo->award_list_, 
                                                      blog_t(ITEM_GOT_CHENG_JIU, cj_id, 0));
  if (ret < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  cheng_jiu_module::give_cheng_jiu_title(player, cji->id_);

  cji->id_ = cjo->next_cj_id_;
  cheng_jiu_module::update_cheng_jiu_2_db(player, cji);

  int r = rand() % 10000 + 1;
  if (r <= cjo->lucky_rate_)
    lucky_egg_module::on_cheng_jiu_egg(player, cjo->lucky_award_);

  if (cheng_jiu_module::check_cheng_jiu_complete(player, cji))
    cheng_jiu_module::notify_cheng_jiu_complete(player, cji->id_);

  out_stream os(client::send_buf, client::send_buf_len);
  os << cj_id;
  return player->send_respond_ok(RES_GET_CHENG_JIU_AWARD, &os);
}
