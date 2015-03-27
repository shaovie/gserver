#include "mall_module.h"
#include "player_obj.h"
#include "shop_config.h"
#include "mall_buy_log.h"
#include "baowu_mall_info.h"
#include "baowu_mall_config.h"
#include "db_proxy.h"
#include "time_util.h"
#include "error.h"
#include "message.h"
#include "global_param_cfg.h"
#include "behavior_id.h"
#include "package_module.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("mall");
static ilog_obj *e_log = err_log::instance()->get_ilog("mall");

void mall_module::destroy(player_obj *player)
{
  while (!player->mall_buy_list_.empty())
    delete player->mall_buy_list_.pop_front();
}
void mall_module::on_enter_game(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << time_util::now - 86400;
  db_proxy::instance()->send_request(player->id(), REQ_GET_MALL_BUY_LOG, &os);
}
void player_obj::handle_db_get_mall_buy_log_result(in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  if (cnt == 0) return ;

  for (int i = 0; i < cnt; ++i)
  {
    char info_bf[sizeof(mall_buy_log) + 4] = {0};
    stream_istr info_si(info_bf, sizeof(info_bf));
    is >> info_si;
    if (info_si.str_len() == sizeof(mall_buy_log))
    {
      mall_buy_log *p = (mall_buy_log *)info_bf;
      mall_module::do_insert_mall_buy_log(this,
                                          p->npc_cid_,
                                          p->item_cid_,
                                          p->amount_,
                                          p->buy_time_);
    }
  }
}
void player_obj::handle_db_get_baowu_mall_info_result(baowu_mall_info *info)
{
  if (info == NULL)
    return ;

  this->baowu_mall_info_ = new baowu_mall_info();
  ::memcpy(this->baowu_mall_info_, info, sizeof(baowu_mall_info));
}
int player_obj::clt_obtain_mall_buy_log(const char *msg, const int len)
{
  int npc_cid = 0;
  in_stream is(msg, len);
  is >> npc_cid;
  out_stream os(client::send_buf, client::send_buf_len);
  os << npc_cid;
  short *amt = (short *)os.wr_ptr();
  os << (short)0;
  for (ilist_node<mall_buy_log *> *itor = this->mall_buy_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (npc_cid == itor->value_->npc_cid_
        && time_util::diff_days(itor->value_->buy_time_) == 0)
    {
      os << itor->value_->item_cid_
        << itor->value_->amount_;
      ++(*amt);
    }
  }
  if (*amt > 0)
    return this->send_request(RES_OBTAIN_MALL_BUY_LOG, &os);
  return 0;
}
int player_obj::clt_obtain_baowu_mall_info(const char *, const int )
{
  if (this->baowu_mall_info_ == NULL)
  {
    this->baowu_mall_info_ = new baowu_mall_info();
    this->baowu_mall_info_->char_id_ = this->id_;
  }

  date_time now_dt((time_t)time_util::now);
  itime now_it(now_dt.hour(), now_dt.min(), now_dt.sec());

  bool refresh = false;
  for (ilist_node<itime> *iter = global_param_cfg::baowu_mall_refresh_time.head();
       iter != NULL;
       iter = iter->next_)
  {
    int sec = now_it - iter->value_;
    if (this->baowu_mall_info_->refresh_time_ == 0
        || (sec >= 0 && sec < time_util::now - this->baowu_mall_info_->refresh_time_))
    {
      refresh = true;
      break ;
    }
  }
  if (refresh)
    mall_module::baowu_mall_refresh(this);


  out_stream os(client::send_buf, client::send_buf_len);
  os << (char)BAOWU_MALL_ITEM_COUNT;
  for (int i = 0; i < BAOWU_MALL_ITEM_COUNT; ++i)
  {
    os << (char)(i + 1)
      << this->baowu_mall_info_->item_info_[i][0]
      << (char)this->baowu_mall_info_->item_info_[i][1];
  }
  return this->send_respond_ok(RES_OBTAIN_BAOWU_MALL_INFO, &os);
}
int player_obj::clt_refresh_baowu_mall(const char *, const int )
{
  if (package_module::calc_item_amount(this, global_param_cfg::mall_refresh_item) == 0)
  {
    int ret = this->is_money_enough(M_BIND_UNBIND_DIAMOND, global_param_cfg::baowu_mall_refresh_diamond);
    if (ret != 0)
      return this->send_respond_err(RES_REFRESH_BAOWU_MALL, ret);
  }

  if (this->baowu_mall_info_ == NULL)
  {
    this->baowu_mall_info_ = new baowu_mall_info();
    this->baowu_mall_info_->char_id_ = this->id_;
  }

  mall_module::baowu_mall_refresh(this);

  if (package_module::calc_item_amount(this, global_param_cfg::mall_refresh_item) == 0)
    this->do_lose_money(global_param_cfg::baowu_mall_refresh_diamond,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_REFRESH_BAOWU_MALL,
                        global_param_cfg::baowu_mall_npc,
                        0,
                        0);
  else
    package_module::do_remove_item(this,
                                   global_param_cfg::mall_refresh_item,
                                   1,
                                   ITEM_LOSE_USE,
                                   0);

  out_stream os(client::send_buf, client::send_buf_len);
  os << (char)BAOWU_MALL_ITEM_COUNT;
  for (int i = 0; i < BAOWU_MALL_ITEM_COUNT; ++i)
    os << (char)(i + 1) << this->baowu_mall_info_->item_info_[i][0];

  return this->send_respond_ok(RES_REFRESH_BAOWU_MALL, &os);
}
int mall_module::can_buy_item(player_obj *player,
                              const int npc_cid,
                              const int item_cid,
                              const int amount,
                              const int param)
{
  if (npc_cid == global_param_cfg::baowu_mall_npc)
  {
    if (param < 1
        || param > BAOWU_MALL_ITEM_COUNT)
      return ERR_CLIENT_OPERATE_ILLEGAL;
    if (player->baowu_mall_info_->item_info_[param - 1][0] != item_cid
        || player->baowu_mall_info_->item_info_[param - 1][1] != 0)
      return ERR_CLIENT_OPERATE_ILLEGAL;
  }else // 每日限购的商城
  {
    const goods_obj *go = shop_config::instance()->get_goods(npc_cid, item_cid);
    if (go == NULL) return ERR_CONFIG_NOT_EXIST;
    if (go->limit_cnt_ == 0) return 0;
    int acc_amount = mall_module::do_check_mall_limit_buy(player, npc_cid, item_cid);
    if ((acc_amount + amount) > go->limit_cnt_)
      return ERR_MALL_BUY_OUT_OF_LIMIT;
  }
  return 0;
}
int mall_module::do_check_mall_limit_buy(player_obj *player,
                                         const int npc_cid,
                                         const int item_cid)
{
  int acc_amount = 0;
  for (ilist_node<mall_buy_log *> *itor = player->mall_buy_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (npc_cid == itor->value_->npc_cid_
        && item_cid == itor->value_->item_cid_
        && time_util::diff_days(itor->value_->buy_time_) == 0)
      acc_amount += itor->value_->amount_;
  }
  return acc_amount;
}
void mall_module::on_buy_item_ok(player_obj *player,
                                 const int npc_cid,
                                 const int item_cid,
                                 const int amt,
                                 const int param)
{
  if (npc_cid == global_param_cfg::baowu_mall_npc)
  {
    player->baowu_mall_info_->item_info_[param - 1][1] = 1;

    stream_ostr so((const char *)player->baowu_mall_info_, sizeof(baowu_mall_info));
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << so;
    db_proxy::instance()->send_request(player->id(), REQ_UPDATE_BAOWU_MALL_INFO, &os);

    os.reset(client::send_buf, client::send_buf_len);
    os << param;
    player->send_request(NTF_BUY_BAOWU_MALL_ITEM, &os);
  }else // 每日限购的商城
  {
    const goods_obj *go = shop_config::instance()->get_goods(npc_cid, item_cid);
    if (go == NULL
        || go->limit_cnt_ == 0)
      return ;

    int total_amount = mall_module::do_insert_mall_buy_log(player,
                                                           npc_cid,
                                                           item_cid,
                                                           amt,
                                                           time_util::now);

    //
    {
      out_stream os(client::send_buf, client::send_buf_len);
      os << npc_cid << item_cid << total_amount;
      player->send_request(NTF_MALL_BUY_LOG, &os);
    }

    mall_buy_log v;
    v.char_id_ = player->id();
    v.npc_cid_ = npc_cid;
    v.item_cid_ = item_cid;
    v.amount_ = amt;
    v.buy_time_ = time_util::now;
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << stream_ostr((char *)&v, sizeof(mall_buy_log));
    db_proxy::instance()->send_request(player->id(), REQ_INSERT_MALL_BUY_LOG, &os);
  }
}
int mall_module::do_insert_mall_buy_log(player_obj *player,
                                        const int npc_cid,
                                        const int item_cid,
                                        const int amount,
                                        const int buy_time)
{
  int total_amount = amount;
  bool find = false;
  for (ilist_node<mall_buy_log *> *itor = player->mall_buy_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (npc_cid == itor->value_->npc_cid_
        && item_cid == itor->value_->item_cid_
        && time_util::diff_days(itor->value_->buy_time_) == 0)
    {
      itor->value_->amount_ += amount;
      total_amount = itor->value_->amount_;
      find = true;
      break;
    }
  }
  if (!find)
  {
    mall_buy_log *p = new mall_buy_log();
    p->char_id_ = player->id();
    p->npc_cid_ = npc_cid;
    p->item_cid_ = item_cid;
    p->amount_   = amount;
    p->buy_time_ = buy_time;
    player->mall_buy_list_.push_back(p);
  }
  return total_amount;
}
void mall_module::baowu_mall_refresh(player_obj *player)
{
  player->baowu_mall_info_->refresh_time_ = time_util::now;
  for (int i = 0; i < BAOWU_MALL_ITEM_COUNT; ++i)
  {
    const int index = baowu_mall_config::instance()->get_item_index(player->lvl());
    player->baowu_mall_info_->item_info_[i][0] = index;
    player->baowu_mall_info_->item_info_[i][1] = 0;
  }

  stream_ostr so((const char *)player->baowu_mall_info_, sizeof(baowu_mall_info));
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << so;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_BAOWU_MALL_INFO, &os);
}

