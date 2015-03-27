#include "all_char_info.h"
#include "time_util.h"
#include "recharge_log.h"
#include "consume_log.h"
#include "package_module.h"
#include "account_info.h"
#include "player_obj.h"
#include "char_attr.h"
#include "db_proxy.h"
#include "rank_module.h"
#include "tianfu_skill_module.h"

// Lib header
#include <map>
#include <string>
#include <strings.h>

account_brief_info::account_brief_info()
{
  ac_info_ = new account_info();
}
bool account_brief_info::is_char_ok(const int char_id)
{
  for (ilist_node<char_brief_info *> *itor = this->char_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->char_id_ == char_id)
      return true;
  }
  return false;
}
void char_brief_info::on_char_lvl_up(const int char_id, const short lvl)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL) cbi->lvl_ = lvl;
}
void char_brief_info::on_char_logout(player_obj *player)
{
  // player attribute cache
  obj_attr o_attr = player->get_obj_attr();
  char_attr c_attr;
  c_attr.char_id_ = player->id();
  c_attr.attr_[ATTR_T_HP]         = o_attr.total_hp();
  c_attr.attr_[ATTR_T_MP]         = o_attr.total_mp();
  c_attr.attr_[ATTR_T_GONG_JI]    = o_attr.gong_ji();
  c_attr.attr_[ATTR_T_FANG_YU]    = o_attr.fang_yu();
  c_attr.attr_[ATTR_T_MING_ZHONG] = o_attr.ming_zhong();
  c_attr.attr_[ATTR_T_SHAN_BI]    = o_attr.shan_bi();
  c_attr.attr_[ATTR_T_BAO_JI]     = o_attr.bao_ji();
  c_attr.attr_[ATTR_T_KANG_BAO]   = o_attr.kang_bao();
  c_attr.attr_[ATTR_T_SHANG_MIAN] = o_attr.shang_mian();
  item_obj *equip_obj = package_module::find_item_by_part(player, PART_ZHU_WU);
  if (equip_obj != NULL)
    c_attr.zhu_wu_ = equip_obj->cid_;
  equip_obj = package_module::find_item_by_part(player, PART_FU_WU);
  if (equip_obj != NULL)
    c_attr.fu_wu_ = equip_obj->cid_;

  stream_ostr attr_so((const char *)&c_attr, sizeof(char_attr));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << attr_so;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_CHAR_ATTR, &os);

  // other
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(player->id());
  if (cbi != NULL)
  {
    cbi->out_time_ = time_util::now;
    ::memcpy(cbi->attr_, c_attr.attr_, sizeof(cbi->attr_));
    cbi->zhu_wu_cid_ = c_attr.zhu_wu_;
    cbi->fu_wu_cid_ = c_attr.fu_wu_;
    cbi->scene_cid_ = player->scene_cid();
    cbi->total_mstar_ = player->total_mstar();

    tianfu_skill_module::fill_char_brief_info(player, cbi);
    cbi->all_qh_lvl_ = player->all_qh_lvl();
    cbi->sum_bao_shi_lvl_ = player->sum_bao_shi_lvl();
  }
}
void char_brief_info::on_char_zhan_li_update(const int char_id, const int val)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL && cbi->zhan_li_ != val)
  {
    rank_module::on_char_zhanli_change(char_id, cbi->zhan_li_, val);
    cbi->zhan_li_ = val;
  }
}
void char_brief_info::on_char_sin_val_update(const int char_id, const int val)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL) cbi->sin_val_ = val;
}
void char_brief_info::on_char_equip_update(const int char_id, const int part, const int equip_cid)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL)
  {
    if (part == PART_ZHU_WU)
      cbi->zhu_wu_cid_ = equip_cid;
    else if (part == PART_FU_WU)
      cbi->fu_wu_cid_ = equip_cid;
  }
}
void char_brief_info::on_char_title_update(const int char_id, const short val)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL) cbi->title_cid_ = val;
}
void char_brief_info::on_char_fa_bao_update(const int char_id, const char dj, const char *fa_bao)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL)
  {
    cbi->fa_bao_dj_ = dj;
    if (fa_bao != NULL)
      ::strncpy(cbi->fa_bao_, fa_bao, sizeof(cbi->fa_bao_) - 1);
  }
}
void char_brief_info::on_recharge_ok(const int char_id,
                                     const int t,
                                     const int value)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return ;
  recharge_log_info *rli = new recharge_log_info(t, value);
  cbi->recharge_log_.push_back(rli);
}
void char_brief_info::on_consume_diamond(const int char_id,
                                         const int behavior_sub_type,
                                         const int t,
                                         const int value)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return ;
  consume_log_info *cli = new consume_log_info(behavior_sub_type,
                                               t,
                                               value);
  cbi->consume_log_.push_back(cli);
}
int char_brief_info::acc_recharge(const int char_id, const int begin_time, const int end_time)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return 0;
  int acc_recharge = 0;
  for (ilist_node<recharge_log_info *> *itor = cbi->recharge_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->rc_time_ >= begin_time
        && itor->value_->rc_time_ <= end_time)
      acc_recharge += itor->value_->value_;
  }
  return acc_recharge;
}
int char_brief_info::daily_recharge(const int char_id, const int begin_time, const int end_time)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return 0;
  int daily_recharge = 0;
  for (ilist_node<recharge_log_info *> *itor = cbi->recharge_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->rc_time_ >= begin_time
        && itor->value_->rc_time_ <= end_time
        && time_util::diff_days(itor->value_->rc_time_) == 0)
      daily_recharge += itor->value_->value_;
  }
  return daily_recharge;
}
int char_brief_info::acc_consume(const int char_id, const int begin_time, const int end_time)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return 0;
  int acc_consume = 0;
  for (ilist_node<consume_log_info *> *itor = cbi->consume_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cs_time_ >= begin_time
        && itor->value_->cs_time_ <= end_time)
      acc_consume += itor->value_->value_;
  }
  return acc_consume;
}
int char_brief_info::daily_consume(const int char_id, const int begin_time, const int end_time)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi == NULL) return 0;
  int daily_consume = 0;
  for (ilist_node<consume_log_info *> *itor = cbi->consume_log_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cs_time_ >= begin_time
        && itor->value_->cs_time_ <= end_time
        && time_util::diff_days(itor->value_->cs_time_) == 0)
      daily_consume += itor->value_->value_;
  }
  return daily_consume;
}
void char_brief_info::on_char_vip_up(const int char_id, const char val)
{
  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
  if (cbi != NULL) cbi->vip_ = val;
}
/**
 * @class
 * 
 * @brief implement of all_char_info
 */
class all_char_info_impl
{
public:
  class my_strcasecmp : public std::binary_function<std::string, std::string, bool>
  {
  public:
    bool operator() (const std::string& left,const std::string& right) const
    { return ::strcasecmp(left.c_str(), right.c_str()) < 0; }
  };
  typedef std::map<int/*char id*/, char_brief_info *> char_id_key_map_t;
  typedef std::map<int/*char id*/, char_brief_info *>::iterator char_id_key_map_itor;
  typedef std::map<std::string, char_brief_info *, my_strcasecmp> char_name_key_map_t;
  typedef std::map<std::string, char_brief_info *, my_strcasecmp>::iterator char_name_key_map_itor;
  typedef std::map<std::string, account_brief_info *> account_info_map_t;
  typedef std::map<std::string, account_brief_info *>::iterator account_info_map_itor;

  //= account
  void insert_account_info(const char *account, account_brief_info *abi)
  { this->account_info_map_.insert(std::make_pair(account, abi)); }
  account_brief_info *get_account_brief_info(const char *account)
  {
    account_info_map_itor itor = this->account_info_map_.find(account);
    return itor == this->account_info_map_.end() ? NULL : itor->second;
  }
  //= char
  void insert_char_info(char_brief_info *cbi)
  {
    this->char_id_key_map_.insert(std::make_pair(cbi->char_id_, cbi));
    this->char_name_key_map_.insert(std::make_pair(cbi->name_, cbi));
  }
  char_brief_info *get_char_brief_info(const char *name)
  {
    char_name_key_map_itor itor = this->char_name_key_map_.find(name);
    return itor == this->char_name_key_map_.end() ? NULL : itor->second;
  }
  char_brief_info *get_char_brief_info(const int char_id)
  {
    char_id_key_map_itor itor = this->char_id_key_map_.find(char_id);
    return itor == this->char_id_key_map_.end() ? NULL : itor->second;
  }
  ilist<int> *get_socialer_list(const int char_id)
  {
    char_brief_info *cbi = this->get_char_brief_info(char_id);
    if (cbi == NULL) return NULL;
    return &(cbi->socialer_list_);
  }
  void on_i_add_socialer(const int my_id, const int socialer_id)
  {
    char_brief_info *cbi = this->get_char_brief_info(socialer_id);
    if (cbi != NULL)
      cbi->socialer_list_.push_back(my_id);
  }
  void on_i_del_socialer(const int my_id, const int socialer_id)
  {
    char_brief_info *cbi = this->get_char_brief_info(socialer_id);
    if (cbi != NULL)
      cbi->socialer_list_.remove(my_id);
  }
public:
  all_char_info_impl() { }

  char_id_key_map_t char_id_key_map_;
  char_name_key_map_t char_name_key_map_;
  account_info_map_t account_info_map_;
};
//=
all_char_info::all_char_info() : impl_(new all_char_info_impl) { }
void all_char_info::insert_char_info(char_brief_info *cbi)
{ this->impl_->insert_char_info(cbi); }
char_brief_info *all_char_info::get_char_brief_info(const char *name)
{ return this->impl_->get_char_brief_info(name); }
char_brief_info *all_char_info::get_char_brief_info(const int char_id)
{ return this->impl_->get_char_brief_info(char_id); }
void all_char_info::insert_account_info(const char *account, account_brief_info *abi)
{ this->impl_->insert_account_info(account, abi); }
account_brief_info *all_char_info::get_account_brief_info(const char *name)
{ return this->impl_->get_account_brief_info(name); }
ilist<int> *all_char_info::get_socialer_list(const int char_id)
{ return this->impl_->get_socialer_list(char_id); }
void all_char_info::on_i_add_socialer(const int my_id, const int socialer_id)
{ this->impl_->on_i_add_socialer(my_id, socialer_id); }
void all_char_info::on_i_del_socialer(const int my_id, const int socialer_id)
{ this->impl_->on_i_del_socialer(my_id, socialer_id); }
