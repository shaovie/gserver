#include "zhan_xing_module.h"
#include "player_obj.h"
#include "zhan_xing_cfg.h"
#include "behavior_id.h"
#include "package_module.h"
#include "item_config.h"
#include "vip_module.h"
#include "cheng_jiu_module.h"
#include "notice_module.h"
#include "global_param_cfg.h"
#include "istream.h"
#include "sys_log.h"
#include "message.h"
#include "error.h"

int player_obj::clt_zhan_xing(const char *msg, const int len)
{
  char type = 0;
  in_stream is(msg, len);
  is >> type;

  zx_obj *zo = zhan_xing_cfg::instance()->get_random_zx_obj(type);
  if (zo == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  int cost = zhan_xing_cfg::instance()->get_zx_cost(type);
  if (this->char_extra_info_->zx_value_ < cost
      && this->is_money_enough(M_BIND_UNBIND_DIAMOND, cost * 100) != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_ZHAN_XING_V_NOT_ENOUGH);

  item_obj *item = NULL;
  if (item_config::item_is_money(zo->item_cid_))
  {
    // 这里就不判断金钱上限了
    this->do_got_item_money(zo->item_cid_,
                            zo->item_cnt_,
                            MONEY_GOT_ZHAN_XING,
                            0);
  }else
  {
    if (package_module::would_be_full(this,
                                      PKG_PACKAGE,
                                      zo->item_cid_,
                                      zo->item_cnt_,
                                      zo->bind_))
      return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);
    item = package_module::do_insert_item(this,
                                          PKG_PACKAGE,
                                          zo->item_cid_,
                                          zo->item_cnt_,
                                          zo->bind_,
                                          ITEM_GOT_ZHAN_XING,
                                          type,
                                          0);
  }

  if (this->char_extra_info_->zx_value_ < cost)
  {
    this->do_lose_money(cost * 100,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_ZHAN_XING,
                        1,
                        0,
                        0);
  }else
  {
    this->char_extra_info_->zx_value_ -= cost;
    this->db_save_char_extra_info();
    this->do_notify_zhan_xing_v_to_clt();
  }

  if (zo->notify_ != 0 && item != NULL)
    notice_module::zhan_xing(this->id(), this->name(), item);

  cheng_jiu_module::on_zhan_xing(this, 1);

  out_stream os(client::send_buf, client::send_buf_len);
  os << zo->item_cid_ << zo->item_cnt_ << zo->bind_;
  return this->send_respond_ok(RES_ZHAN_XING, &os);
}
int player_obj::clt_zhan_xing_ten(const char *, const int )
{
  int miss_zx_v = global_param_cfg::zx_ten_cost - this->char_extra_info_->zx_value_;
  if (this->char_extra_info_->zx_value_ < global_param_cfg::zx_ten_cost
      && this->is_money_enough(M_BIND_UNBIND_DIAMOND, miss_zx_v * 100) != 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_ZHAN_XING_V_NOT_ENOUGH);

  int used_space = package_module::used_space(this, PKG_PACKAGE);
  if ((package_module::package_capacity(this, PKG_PACKAGE) - used_space) < ZHAN_XING_ONCE_COUNT)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  ilist<item_amount_bind_t> item_list;
  for (int i = 1; i <= ZHAN_XING_ONCE_COUNT; ++i)
  {
    zx_obj *zo = zhan_xing_ten_cfg::instance()->get_random_zx_obj(i);
    if (zo == NULL) continue;

    if (item_config::item_is_money(zo->item_cid_))
    {
      // 这里就不判断金钱上限了
      this->do_got_item_money(zo->item_cid_,
                              zo->item_cnt_,
                              MONEY_GOT_ZHAN_XING,
                              0);
    }else
    {
      package_module::do_insert_item(this,
                                     PKG_PACKAGE,
                                     zo->item_cid_,
                                     zo->item_cnt_,
                                     zo->bind_,
                                     ITEM_GOT_ZHAN_XING,
                                     i,
                                     0);
    }
    item_amount_bind_t v;
    v.cid_    = zo->item_cid_;
    v.amount_ = zo->item_cnt_;
    v.bind_   = zo->bind_;
    item_list.push_back(v);
  }

  if (this->char_extra_info_->zx_value_ < global_param_cfg::zx_ten_cost)
  {
    this->do_lose_money(miss_zx_v * 100,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_ZHAN_XING,
                        10,
                        0,
                        0);
  }
  if (this->char_extra_info_->zx_value_ > 0)
  {
    this->char_extra_info_->zx_value_ -= global_param_cfg::zx_ten_cost;
    if (this->char_extra_info_->zx_value_ < 0)
      this->char_extra_info_->zx_value_ = 0;
    this->db_save_char_extra_info();
    this->do_notify_zhan_xing_v_to_clt();
  }

  cheng_jiu_module::on_zhan_xing(this, ZHAN_XING_ONCE_COUNT);

  out_stream os(client::send_buf, client::send_buf_len);
  os << item_list.size();
  while (!item_list.empty())
  {
    item_amount_bind_t v = item_list.pop_front();
    os << v.cid_ << v.amount_ << v.bind_;
  }
  return this->send_respond_ok(RES_ZHAN_XING_TEN, &os);
}
void zhan_xing_module::on_recharge_ok(player_obj *player, const int diamond)
{
  player->do_add_zhan_xing_value(diamond / 100);
}
void zhan_xing_module::on_vip_lvl_up(player_obj *player)
{
  player->do_add_zhan_xing_value(vip_module::to_get_add_zhan_xing_value(player));
}
void player_obj::do_add_zhan_xing_value(const int v)
{
  if (v <= 0) return ;
  this->char_extra_info_->zx_value_ += v;
  if (this->char_extra_info_->zx_value_ > MAX_VALID_INT)
    this->char_extra_info_->zx_value_ = MAX_VALID_INT;
  this->db_save_char_extra_info();
  this->do_notify_zhan_xing_v_to_clt();
}
void player_obj::do_notify_zhan_xing_v_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->char_extra_info_->zx_value_;
  this->do_delivery(NTF_ZHAN_XING_VALUE, &mb);
}
