#include "huo_yue_du_module.h"
#include "huo_yue_du_cfg.h"
#include "huo_yue_du_info.h"
#include "player_obj.h"
#include "db_proxy.h"
#include "package_module.h"
#include "behavior_id.h"

// Lib header
#include <map>

class huo_yue_du_data
{
public:
  typedef std::map<int/*cid*/, huo_yue_du_info *> huo_yue_du_info_map_t;
  typedef std::map<int/*cid*/, huo_yue_du_info *>::iterator huo_yue_du_info_map_iter;

  huo_yue_du_data() { }
  ~huo_yue_du_data()
  {
    for (huo_yue_du_info_map_iter iter = this->huo_yue_du_info_map_.begin();
         iter != this->huo_yue_du_info_map_.end();
         ++iter)
      huo_yue_du_info_pool::instance()->release(iter->second);
  }
  
  void insert(const int cid, huo_yue_du_info *hydi)
  { this->huo_yue_du_info_map_.insert(std::make_pair(cid, hydi)); }

  void erase(const int cid)
  { this->huo_yue_du_info_map_.erase(cid); }

  void clear()
  {
    for (huo_yue_du_info_map_iter iter = this->huo_yue_du_info_map_.begin();
         iter != this->huo_yue_du_info_map_.end();
         ++iter)
    {
      iter->second->get_award_ = 0;
      iter->second->score_ = 0;
    }
  }

  huo_yue_du_info *find(const int cid)
  {
    huo_yue_du_info_map_iter iter = this->huo_yue_du_info_map_.find(cid);
    if (iter == this->huo_yue_du_info_map_.end())
      return NULL;
    return iter->second;
  }
  void build_info(out_stream &os)
  {
    os << (short)this->huo_yue_du_info_map_.size();
    for (huo_yue_du_info_map_iter iter = this->huo_yue_du_info_map_.begin();
         iter != this->huo_yue_du_info_map_.end();
         ++iter)
      os << iter->second->cid_ << iter->second->score_ << iter->second->get_award_;
  }

private:
  huo_yue_du_info_map_t huo_yue_du_info_map_;
};

void huo_yue_du_module::init(player_obj *player)
{
  player->huo_yue_du_data_ = new huo_yue_du_data();
}
void huo_yue_du_module::destroy(player_obj *player)
{
  delete player->huo_yue_du_data_;
  player->huo_yue_du_data_ = NULL;
}
void huo_yue_du_module::on_enter_game(player_obj *player)
{
  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_GET_HUO_YUE_DU_INFO, &db_os);
}
int huo_yue_du_module::handle_db_get_huo_yue_du_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;

  bool notify = false;
  for (int i = 0; i < res_cnt; ++i)
  {
    char bf[sizeof(huo_yue_du_info) + 4] = {0};
    stream_istr buff_si(bf, sizeof(bf));
    is >> buff_si;

    huo_yue_du_info *p = (huo_yue_du_info *)bf;
    if (huo_yue_du_cfg::instance()->get_award(p->cid_) == NULL)
      continue;
    huo_yue_du_info *hydi = huo_yue_du_info_pool::instance()->alloc();
    ::memcpy(hydi, bf, sizeof(huo_yue_du_info));
    player->huo_yue_du_data_->insert(hydi->cid_, hydi);

    int total_score = huo_yue_du_cfg::instance()->get_total_score(hydi->cid_);
    if (hydi->score_ >= total_score && hydi->get_award_ == 0)
      notify = true;
  }

  if (notify)
    player->send_request(NTF_HUO_YUE_DU_COMPLETE, NULL);

  return 0;
}
void huo_yue_du_module::on_new_day(player_obj *player)
{
  player->huo_yue_du_data_->clear();

  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_CLEAR_HUO_YUE_DU_INFO, &db_os);
}
void huo_yue_du_module::add_huo_yue_du_score(player_obj *player,
                                             huo_yue_du_info *hydi,
                                             const int cid)
{
  int total_score = huo_yue_du_cfg::instance()->get_total_score(cid);
  if (hydi->score_ >= total_score) return;
  int per_score = huo_yue_du_cfg::instance()->get_per_score(cid);

  hydi->score_ += per_score;

  huo_yue_du_module::update_huo_yue_du_2_db(player, hydi);

  player->char_extra_info_->huo_yue_score_ += per_score;

  if (player->char_extra_info_->huo_yue_score_ >= HUO_YUE_SCORE_CHANGE_ZX_VALUE)
  {
    player->char_extra_info_->huo_yue_score_ -= HUO_YUE_SCORE_CHANGE_ZX_VALUE;
    player->char_extra_info_->zx_value_++;
    if (player->char_extra_info_->zx_value_ > MAX_VALID_INT)
      player->char_extra_info_->zx_value_ = MAX_VALID_INT;
    player->do_notify_zhan_xing_v_to_clt();
  }
  player->db_save_char_extra_info();

  if (hydi->score_ >= total_score && hydi->get_award_ == 0)
    player->send_request(NTF_HUO_YUE_DU_COMPLETE, NULL);
}
void huo_yue_du_module::update_huo_yue_du_2_db(player_obj *player, huo_yue_du_info *hydi)
{
  out_stream db_os(client::send_buf, client::send_buf_len);
  db_os << player->db_sid() << stream_ostr((char *)hydi, sizeof(huo_yue_du_info));
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_HUO_YUE_DU_INFO, &db_os);
}

void huo_yue_du_module::on_login_award(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_LOGIN_AWARD);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_LOGIN_AWARD;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_LOGIN_AWARD);
}
void huo_yue_du_module::on_equip_qh(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_EQUIP_QH);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_EQUIP_QH;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_EQUIP_QH);
}
void huo_yue_du_module::on_equip_fen_jie(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_EQUIP_FEN_JIE);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_EQUIP_FEN_JIE;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_EQUIP_FEN_JIE);
}
void huo_yue_du_module::on_equip_xi_lian(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_EQUIP_XI_LIAN);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_EQUIP_XI_LIAN;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_EQUIP_XI_LIAN);
}
void huo_yue_du_module::on_upgrade_bao_shi(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_UPGRADE_BAO_SHI);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_UPGRADE_BAO_SHI;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_UPGRADE_BAO_SHI);
}
void huo_yue_du_module::on_upgrade_passive_skill(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_UPGRADE_PASSIVE_SKILL);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_UPGRADE_PASSIVE_SKILL;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_UPGRADE_PASSIVE_SKILL);
}
void huo_yue_du_module::on_enter_scp(player_obj *player, const int scene_cid)
{
  int cid = 0;
  if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_QH_SHI_SCP))
    cid = HYD_QH_SHI_SCP;
  else if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_SKILL_FU_SCP))
    cid = HYD_SKILL_FU_SCP;
  else if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_XI_LIAN_SHI_SCP))
    cid = HYD_XI_LIAN_SHI_SCP;
  else if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_BAO_SHI_JING_HUA_SCP))
    cid = HYD_BAO_SHI_JING_HUA_SCP;
  else if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_COIN_SCP))
    cid = HYD_COIN_SCP;
  else if (scene_cid == huo_yue_du_cfg::instance()->get_param(HYD_EXP_SCP))
    cid = HYD_EXP_SCP;
  else
    return;

  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(cid);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = cid;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, cid);
}
void huo_yue_du_module::on_jing_ji(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_JING_JI);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_JING_JI;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_JING_JI);
}
void huo_yue_du_module::on_guild_ju_bao(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_GUILD_JU_BAO);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_GUILD_JU_BAO;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_GUILD_JU_BAO);
}
void huo_yue_du_module::on_mo_bai(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_MO_BAI);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_MO_BAI;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_MO_BAI);
}
void huo_yue_du_module::on_water_tree(player_obj *player)
{
  huo_yue_du_info *hydi = player->huo_yue_du_data_->find(HYD_WATER_TREE);
  if (hydi == NULL)
  {
    hydi = huo_yue_du_info_pool::instance()->alloc();

    hydi->cid_ = HYD_WATER_TREE;
    hydi->char_id_ = player->id();

    player->huo_yue_du_data_->insert(hydi->cid_, hydi);
  }

  huo_yue_du_module::add_huo_yue_du_score(player, hydi, HYD_WATER_TREE);
}

//==========================================================================
int player_obj::clt_get_huo_yue_du_info(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  this->huo_yue_du_data_->build_info(os);

  os << this->char_extra_info_->huo_yue_score_;

  return this->send_respond_ok(RES_GET_HUO_YUE_DU_LIST, &os);
}
int player_obj::clt_get_huo_yue_du_award(const char *msg, const int len)
{
  in_stream is(msg, len);
  short cid = 0;
  is >> cid;

  int total_score = huo_yue_du_cfg::instance()->get_total_score(cid);
  huo_yue_du_info *hydi = this->huo_yue_du_data_->find(cid);
  if (hydi == NULL || hydi->score_ < total_score)
    return this->send_respond_err(RES_GET_HUO_YUE_DU_AWARD, ERR_HUO_YUE_DU_NOT_ENOUGH);
  if (hydi->get_award_ != 0)
    return this->send_respond_err(RES_GET_HUO_YUE_DU_AWARD, ERR_AWARD_OVER_GET);

  ilist<item_amount_bind_t> *v = huo_yue_du_cfg::instance()->get_award(cid);
  if (v == NULL)
    return this->send_respond_err(RES_GET_HUO_YUE_DU_AWARD, ERR_CONFIG_NOT_EXIST);

  int ret = package_module::do_insert_award_item_list(this,
                                                      v,
                                                      blog_t(ITEM_GOT_HUO_YUE_DU, cid, 0));
  if (ret != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ret);

  hydi->get_award_ = 1;
  huo_yue_du_module::update_huo_yue_du_2_db(this, hydi);

  out_stream os(client::send_buf, client::send_buf_len);
  os << cid;
  return this->send_respond_ok(RES_GET_HUO_YUE_DU_AWARD, &os);
}

