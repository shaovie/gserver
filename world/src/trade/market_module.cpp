#include "market_module.h"
#include "market_info.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "package_module.h"
#include "global_param_cfg.h"
#include "global_macros.h"
#include "mail_module.h"
#include "mail_info.h"
#include "db_proxy.h"
#include "svc_config.h"
#include "array_t.h"
#include "clsid.h"
#include "message.h"
#include "mail_config.h"
#include "behavior_id.h"
#include "time_util.h"
#include "notice_module.h"
#include "sys.h"
#include "sys_log.h"

// Lib header
#include <map>
#include <list>
#include <vector>
#include <algorithm>

#define MARKET_DB_SID         0

#define MONEY_SORT_UP         1
#define MONEY_SORT_DOWN       2

static ilog_obj *s_log = sys_log::instance()->get_ilog("market");
static ilog_obj *e_log = err_log::instance()->get_ilog("market");


int      market_module::last_check_time        = 0;
int      market_module::max_market_id          = 0;
ilist<int> market_module::market_id_pool;

typedef std::map<int/*market_id*/, market_info *> mk_info_map_t;
typedef std::map<int/*market_id*/, market_info *>::iterator  mk_info_map_iter;

typedef std::list<market_info *> mk_info_sort_list_t;
typedef std::list<market_info *>::iterator  mk_info_sort_list_iter;

class market_kind
{
public:
  void insert(market_info *mi)
  {
    this->mk_info_map_.insert(std::make_pair(mi->market_id_, mi));
    this->mk_info_sort_list_.push_front(mi);
  }
  void remove(const int market_id)
  {
    mk_info_map_iter iter = this->mk_info_map_.find(market_id);
    if (iter != this->mk_info_map_.end())
    {
      this->mk_info_map_.erase(iter);
      this->mk_info_sort_list_.remove(iter->second);
      market_info_pool::instance()->release(iter->second);
    }
  }
  market_info *find(const int market_id)
  {
    mk_info_map_iter iter = this->mk_info_map_.find(market_id);
    if (iter == this->mk_info_map_.end())
      return NULL;
    return iter->second;
  }
  static bool time_compare(market_info *mi_1, market_info *mi_2)
  { return mi_1->begin_time_ > mi_2->begin_time_; }
  void sort()
  {
    this->mk_info_sort_list_.clear();
    for (mk_info_map_iter iter = this->mk_info_map_.begin();
         iter != this->mk_info_map_.end();
         ++iter)
      this->mk_info_sort_list_.push_back(iter->second);
    this->mk_info_sort_list_.sort(time_compare);
  }

public:
  mk_info_map_t mk_info_map_;
  mk_info_sort_list_t mk_info_sort_list_;
};

typedef std::map<int/*market_type*/, market_kind *> mk_kind_map_t;
typedef std::map<int/*market_type*/, market_kind *>::iterator  mk_kind_map_itor;

static mk_kind_map_t s_mk_kind_map;

static bool money_compare_up(market_info *mi_1, market_info *mi_2)
{
  if (mi_1->item_ == NULL || mi_2->item_ == NULL)
    return true;
  if (mi_1->item_->cid_ == global_param_cfg::item_coin_cid)
    return mi_1->price_ < mi_2->price_;

  double p1 = mi_1->price_ / mi_1->item_->amount_;
  double p2 = mi_2->price_ / mi_2->item_->amount_;
  return p1 < p2;
}
static bool money_compare_down(market_info *mi_1, market_info *mi_2)
{
  if (mi_1->item_ == NULL || mi_2->item_ == NULL)
    return true;
  if (mi_1->item_->cid_ == global_param_cfg::item_coin_cid)
    return mi_1->price_ > mi_2->price_;

  double p1 = mi_1->price_ / mi_1->item_->amount_;
  double p2 = mi_2->price_ / mi_2->item_->amount_;
  return p1 > p2;
}

void market_module::market_init()
{
  for (mk_kind_map_itor iter = s_mk_kind_map.begin();
       iter != s_mk_kind_map.end();
       ++iter)
  {
    if (iter->second) continue;
    iter->second->sort();
  }
}
void market_module::do_insert_market_info(market_info *mi)
{
  if (mi->item_ == NULL) return;

  int market_type = item_config::instance()->market_classify(mi->item_->cid_);

  market_kind *mk = NULL;
  mk_kind_map_itor iter = s_mk_kind_map.find(market_type);
  if (iter == s_mk_kind_map.end())
  {
    mk = new market_kind();
    s_mk_kind_map.insert(std::make_pair(market_type, mk));
  }else
    mk = iter->second;
  mk->insert(mi);
}
void market_module::fetch_market_info(market_info *mi, out_stream &os)
{
  os << mi->market_id_
    << mi->price_
    << mi->price_type_;
  int remaind_time = (global_param_cfg::market_item_sale_time * 3600) - (time_util::now - mi->begin_time_);
  if (remaind_time <= 0) remaind_time = 0;
  os << remaind_time;

  package_module::do_build_item_info(mi->item_, os);
}
void market_module::insert_market_info_2_db(market_info *mi)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << MARKET_DB_SID << mi;
  db_proxy::instance()->send_request(MARKET_DB_SID, REQ_INSERT_MARKET_INFO, &os);
}
void market_module::delete_market_info_2_db(const int market_id)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << MARKET_DB_SID << market_id;
  db_proxy::instance()->send_request(MARKET_DB_SID, REQ_DELETE_MARKET_INFO, &os);
}
int market_module::assign_market_id()
{
  int market_id = 0;
  if (market_module::market_id_pool.empty())
  {
    ++market_module::max_market_id;
    if (market_module::max_market_id >= (svc_config::instance()->service_sn() + 1) * MARKET_ID_RANGE)
      return -1;
    market_id = market_module::max_market_id;
  }
  else
    market_id = market_module::market_id_pool.pop_front();
  return market_id;
}
void market_module::reclaim_market_id(const int market_id)
{
  if (market_id > svc_config::instance()->service_sn() * MARKET_ID_RANGE
      && (market_id < (svc_config::instance()->service_sn() + 1) * MARKET_ID_RANGE))
    market_module::market_id_pool.push_back(market_id);
}
void market_module::do_timeout(const int now)
{
  if (now - market_module::last_check_time < 300)
    return;
  market_module::last_check_time = now;

  for (mk_kind_map_itor iter = s_mk_kind_map.begin();
       iter != s_mk_kind_map.end();
       ++iter)
  {
    market_kind *mk = iter->second;

    while (!mk->mk_info_sort_list_.empty())
    {
      market_info *mi = mk->mk_info_sort_list_.back();
      if (now - mi->begin_time_
          < 3600 * global_param_cfg::market_item_sale_time)
        break;

      const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::MARKET_TIME_OVER_MAIL);
      if (mo != NULL)
      {
        if (mi->item_->cid_ == global_param_cfg::item_coin_cid)
          mail_module::do_send_mail(mi->char_id_, mail_info::MAIL_SEND_SYSTEM_ID,
                                    mo->sender_name_, mo->title_, mo->content_,
                                    mail_info::MAIL_TYPE_MARKET,
                                    mi->item_->amount_, 0, 0,
                                    0, NULL,
                                    mi->char_id_, time_util::now);
        else
          mail_module::do_send_mail(mi->char_id_, mail_info::MAIL_SEND_SYSTEM_ID,
                                    mo->sender_name_, mo->title_, mo->content_,
                                    mail_info::MAIL_TYPE_MARKET,
                                    0, 0, 0,
                                    1, mi->item_,
                                    mi->char_id_, time_util::now);
        player_obj *player = player_mgr::instance()->find(mi->char_id_);
        if (player != NULL)
          mail_module::do_notify_haved_new_mail(player, 1);
      }

      market_module::delete_market_info_2_db(mi->market_id_);
      market_module::reclaim_market_id(mi->market_id_);

      mk->mk_info_sort_list_.pop_back();
      mk_info_map_iter iter = mk->mk_info_map_.find(mi->market_id_);
      if (iter != mk->mk_info_map_.end())
      {
        mk->mk_info_map_.erase(iter);
        market_info_pool::instance()->release(iter->second);
      }
    }
  }
}

int market_module::dispatch_msg(player_obj *player,
                                const int msg_id,
                                const char *msg,
                                const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = market_module::FUNC(player, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_GET_MARKET_LIST,       clt_get_market_list);
    SHORT_CODE(REQ_SORT_BY_MONEY,         clt_sort_by_money);
    SHORT_CODE(REQ_BUY_MARKET_ITEM,       clt_buy_market_item);
    SHORT_CODE(REQ_SALE_MARKET_ITEM,      clt_sale_market_item);
    SHORT_CODE(REQ_SALE_MARKET_MONEY,     clt_sale_market_money);
    SHORT_CODE(REQ_CANCEL_MARKET_ITEM,    clt_cancel_market_item);
    SHORT_CODE(REQ_GET_SELF_SALE_LIST,    clt_get_self_sale_list);
    SHORT_CODE(REQ_SEARCH_MARKET_BY_NAME, clt_search_market_by_name);
  }

  return ret;
}
int market_module::clt_get_market_list(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  short market_type = 0;
  short page = 0;
  int value = 0;
  is >> market_type >> page >> value;
  if (page <= 0)
    return player->send_respond_err(RES_GET_MARKET_LIST, ERR_CLIENT_OPERATE_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  os << page;
  int *count = (int *)os.wr_ptr();
  os << 0;
  char *every_cnt = os.wr_ptr();
  os << (char)0;
  mk_kind_map_itor iter = s_mk_kind_map.find(market_type);
  if (iter == s_mk_kind_map.end())
    return player->send_respond_ok(RES_GET_MARKET_LIST, &os);
  market_kind *mk = iter->second;

  for (mk_info_sort_list_iter iter = mk->mk_info_sort_list_.begin();
       iter != mk->mk_info_sort_list_.end();
       ++iter)
  {
    market_info *mi = *iter;

    if (clsid::is_fa_bao(mi->item_->cid_))
    {
      fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao(mi->item_->cid_);
      if (fbco == NULL) continue;
    }else if (value > 0
              && (mi->item_->cid_ != global_param_cfg::item_coin_cid
                  || mi->item_->amount_ != value))
      continue;
    else if (value == 0
             && mi->item_->cid_ == global_param_cfg::item_coin_cid)
      continue;

    (*count)++;
    if (*count > (page - 1) * global_param_cfg::market_one_page_amt * 2
        && *count <= page * global_param_cfg::market_one_page_amt * 2)
    {
      market_module::fetch_market_info(mi, os);
      (*every_cnt)++;
    }
  }

  return player->send_respond_ok(RES_GET_MARKET_LIST, &os);
}
int market_module::clt_sort_by_money(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  short market_type = 0;
  char sort_type = 0;
  short page = 0;
  int value = 0;
  is >> market_type >> page >> value >> sort_type;
  if (page <= 0)
    return player->send_respond_err(RES_SORT_BY_MONEY, ERR_CLIENT_OPERATE_ILLEGAL);

  static std::vector<market_info *> money_sort_vector;
  money_sort_vector.clear();
  mk_kind_map_itor iter = s_mk_kind_map.find(market_type);
  if (iter == s_mk_kind_map.end())
    return player->send_respond_err(RES_SORT_BY_MONEY, ERR_MARKET_NOT_HAVE_ITEM);
  market_kind *mk = iter->second;

  for (mk_info_sort_list_iter iter = mk->mk_info_sort_list_.begin();
       iter != mk->mk_info_sort_list_.end();
       ++iter)
  {
    market_info *mi = *iter;
    if (clsid::is_fa_bao(mi->item_->cid_))
    {
      fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao((*iter)->item_->cid_);
      if (fbco == NULL) continue;
    }else if (value > 0
              && (mi->item_->cid_ != global_param_cfg::item_coin_cid
                  || mi->item_->amount_ != value))
      continue;
    else if (value == 0
             && mi->item_->cid_ == global_param_cfg::item_coin_cid)
      continue;

    money_sort_vector.push_back(*iter);
  }

  int count = money_sort_vector.size();
  if (count < (page - 1) * global_param_cfg::market_one_page_amt * 2)
    return player->send_respond_err(RES_SORT_BY_MONEY, ERR_CLIENT_OPERATE_ILLEGAL);

  if (sort_type == MONEY_SORT_UP)
    std::sort(money_sort_vector.begin(), money_sort_vector.end(), money_compare_up);
  else
    std::sort(money_sort_vector.begin(), money_sort_vector.end(), money_compare_down);

  out_stream os(client::send_buf, client::send_buf_len);
  os << page << count;
  char *every_cnt = os.wr_ptr();
  os << (char)0;

  for (int i = (page - 1) * global_param_cfg::market_one_page_amt * 2;
       i < page * global_param_cfg::market_one_page_amt * 2 && i < count;
       ++i)
  {
    market_module::fetch_market_info(money_sort_vector[i], os);
    (*every_cnt)++;
  }

  return player->send_respond_ok(RES_SORT_BY_MONEY, &os);
}
int market_module::clt_buy_market_item(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  short market_type = 0;
  int market_id = 0;
  is >> market_type >> market_id;

  mk_kind_map_itor iter = s_mk_kind_map.find(market_type);
  if (iter == s_mk_kind_map.end())
    return player->send_respond_err(RES_SORT_BY_MONEY, ERR_MARKET_NOT_HAVE_ITEM);
  market_kind *mk = iter->second;

  market_info *mi = mk->find(market_id);
  if (mi == NULL)
    return player->send_respond_err(RES_BUY_MARKET_ITEM, ERR_MARKET_NOT_HAVE_ITEM);

  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::MARKET_SALE_MAIL);
  if (mo == NULL)
    return player->send_respond_err(RES_BUY_MARKET_ITEM, ERR_CONFIG_NOT_EXIST);

  int ret = player->is_money_enough(mi->price_type_, mi->price_);
  if (ret < 0)
    return player->send_respond_err(RES_BUY_MARKET_ITEM, ret);

  if (mi->item_->cid_ == global_param_cfg::item_coin_cid)
  {
    if (player->is_money_upper_limit(M_COIN, mi->item_->amount_))
      return player->send_respond_err(RES_BUY_MARKET_ITEM, ERR_MONEY_UPPER_LIMIT);

    player->do_got_money(mi->item_->amount_, M_COIN, MONEY_GOT_MARKET_BUY, mi->char_id_);
  }else
  {
    if (package_module::would_be_full(player,
                                      PKG_PACKAGE,
                                      mi->item_->cid_,
                                      mi->item_->amount_,
                                      mi->item_->bind_))
      return player->send_respond_err(RES_BUY_MARKET_ITEM, ERR_PACKAGE_SPACE_NOT_ENOUGH);
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   mi->item_->cid_,
                                   mi->item_->amount_,
                                   mi->item_->bind_,
                                   ITEM_GOT_MARKET_BUY,
                                   mi->char_id_,
                                   0);
  }
  player->do_lose_money(mi->price_, mi->price_type_, MONEY_LOSE_MARKET_BUY, mi->char_id_, 0, 0);

  int fee = mi->price_ * global_param_cfg::market_service_charge / 100;
  if (fee < global_param_cfg::market_min_service_charge)
    fee = global_param_cfg::market_min_service_charge;

  int diamond = mi->price_ - fee;

  if (diamond < 0) diamond = 0;

  int size = 4;
  ::strncpy(mail_module::replace_str[0], MAIL_P_ITEM, sizeof(mail_module::replace_str[0]) - 1);
  ::strncpy(mail_module::replace_str[1], MAIL_P_NUMBER, sizeof(mail_module::replace_str[1]) - 1);
  ::strncpy(mail_module::replace_str[2], MAIL_P_NUMBER, sizeof(mail_module::replace_str[2]) - 1);
  ::strncpy(mail_module::replace_str[3], MAIL_P_MONEY_TYPE, sizeof(mail_module::replace_str[3]) - 1);
  notice_module::build_item_str(mi->item_, mail_module::replace_value[0], MAX_REPLACE_STR + 1);
  ::snprintf(mail_module::replace_value[1],
             sizeof(mail_module::replace_value[1]),
             "%s(%d)",
             STRING_NUMBER, mi->item_->amount_);
  ::snprintf(mail_module::replace_value[2],
             sizeof(mail_module::replace_value[2]),
             "%s(%d)",
             STRING_NUMBER, diamond);
  ::snprintf(mail_module::replace_value[3],
             sizeof(mail_module::replace_value[3]),
             "%s(%d)",
             STRING_MONEY_TYPE, mi->price_type_);

  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  mail_module::replace_mail_info(mo->content_,
                                 mail_module::replace_str,
                                 mail_module::replace_value,
                                 size,
                                 content);

  mail_module::do_send_mail(mi->char_id_, mail_info::MAIL_SEND_SYSTEM_ID,
                            mo->sender_name_, mo->title_, content,
                            mail_info::MAIL_TYPE_MARKET,
                            0, diamond, 0,
                            0, NULL,
                            player->db_sid(), time_util::now);

  player_obj *sail_player = player_mgr::instance()->find(mi->char_id_);
  if (sail_player != NULL)
    mail_module::do_notify_haved_new_mail(sail_player, 1);

  market_module::delete_market_info_2_db(mi->market_id_);
  market_module::reclaim_market_id(mi->market_id_);
  mk->remove(mi->market_id_);

  return player->send_respond_ok(RES_BUY_MARKET_ITEM);
}
int market_module::clt_sale_market_item(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  int item_id = 0;
  int amount = 0;
  int price = 0;
  is >> item_id >> amount >> price;

  if (price <= 0 || amount <= 0)
    return player->send_respond_err(RES_SALE_MARKET_ITEM, ERR_CLIENT_OPERATE_ILLEGAL);

  item_obj *item = package_module::find_item(player, item_id);
  if (item == NULL
      || item->bind_ == BIND_TYPE
      || item->amount_ < amount)
    return player->send_respond_err(RES_SALE_MARKET_ITEM, ERR_CLIENT_OPERATE_ILLEGAL);

  item_cfg_obj *ico = item_config::instance()->get_item(item->cid_);
  if (ico != NULL
      && !ico->can_sell_)
    return player->send_respond_err(RES_SALE_MARKET_ITEM, ERR_ITEM_CAN_NOT_SELL);

  int new_market_id = market_module::assign_market_id();
  if (new_market_id < 0)
    return player->send_respond_err(RES_SALE_MARKET_ITEM, ERR_MARKET_FULL);
  market_info *mi = market_info_pool::instance()->alloc();
  mi->market_id_  = new_market_id;
  mi->char_id_    = player->id();
  mi->price_      = price;
  mi->price_type_ = M_DIAMOND;
  mi->begin_time_ = time_util::now;
  mi->item_       = item_obj_pool::instance()->alloc();
  (*mi->item_) = (*item);
  mi->item_->amount_ = amount;
  market_module::do_insert_market_info(mi);

  market_module::insert_market_info_2_db(mi);

  if (price >= global_param_cfg::market_min_notice_charge)
    notice_module::market_sale_item(player->id(), player->name(), price, item);

  package_module::do_remove_item(player, item, amount, ITEM_LOSE_MARKET_SALE, 0);

  return player->send_respond_ok(RES_SALE_MARKET_ITEM);
}
int market_module::clt_sale_market_money(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  int value = 0;
  int price = 0;
  is >> value >> price;

  if (value <= 0 || price <= 0)
    return player->send_respond_err(RES_SALE_MARKET_MONEY, ERR_CLIENT_OPERATE_ILLEGAL);

  int ret = player->is_money_enough(M_COIN, value);
  if (ret < 0)
    return player->send_respond_err(RES_SALE_MARKET_MONEY, ret);

  int market_id = market_module::assign_market_id();
  if (market_id < 0)
    return player->send_respond_err(RES_SALE_MARKET_MONEY, ERR_MARKET_FULL);
  market_info *mi = market_info_pool::instance()->alloc();
  mi->market_id_  = market_id;
  mi->char_id_    = player->id();
  mi->price_      = price;
  mi->price_type_ = M_DIAMOND;
  mi->begin_time_ = time_util::now;
  mi->item_       = item_obj_pool::instance()->alloc();
  mi->item_->cid_ = global_param_cfg::item_coin_cid;
  mi->item_->amount_ = value;
  market_module::do_insert_market_info(mi);

  market_module::insert_market_info_2_db(mi);

  if (price > global_param_cfg::market_min_notice_charge)
    notice_module::market_sale_money(player->id(), player->name(), value, price);

  player->do_lose_money(value,
                        M_COIN,
                        MONEY_LOSE_MARKET_SALE,
                        mi->item_->cid_,
                        0,
                        0);

  return player->send_respond_ok(RES_SALE_MARKET_MONEY);
}
int market_module::clt_cancel_market_item(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  short market_type = 0;
  int market_id = 0;
  is >> market_type >> market_id;

  mk_kind_map_itor iter = s_mk_kind_map.find(market_type);
  if (iter == s_mk_kind_map.end())
    return player->send_respond_err(RES_CANCEL_MARKET_ITEM, ERR_MARKET_NOT_HAVE_ITEM);
  market_kind *mk = iter->second;

  market_info *mi = mk->find(market_id);
  if (mi == NULL
      || mi->char_id_ != player->id())
    return player->send_respond_err(RES_CANCEL_MARKET_ITEM, ERR_MARKET_NOT_HAVE_ITEM);

  if (mi->item_->cid_ == global_param_cfg::item_coin_cid)
  {
    if (player->is_money_upper_limit(M_COIN, mi->item_->amount_))
      return player->send_respond_err(RES_CANCEL_MARKET_ITEM, ERR_MONEY_UPPER_LIMIT);
    player->do_got_money(mi->item_->amount_, M_COIN, MONEY_GOT_CANCEL_MARKET_SAIL, 0);
  }else
  {
    if (package_module::would_be_full(player,
                                      PKG_PACKAGE,
                                      mi->item_->cid_,
                                      mi->item_->amount_,
                                      mi->item_->bind_))
      return player->send_respond_err(RES_BUY_MARKET_ITEM, ERR_PACKAGE_SPACE_NOT_ENOUGH);

    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   mi->item_->cid_,
                                   mi->item_->amount_,
                                   mi->item_->bind_,
                                   ITEM_GOT_MARKET_CANCEL,
                                   0,
                                   0);
  }

  market_module::delete_market_info_2_db(market_id);
  market_module::reclaim_market_id(mi->market_id_);
  mk->remove(market_id);

  return player->send_respond_ok(RES_CANCEL_MARKET_ITEM);
}
int market_module::clt_get_self_sale_list(player_obj *player, const char *, const int )
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  out_stream os(client::send_buf, client::send_buf_len);
  int *count = (int*)os.wr_ptr();
  os << 0;
  for (mk_kind_map_itor iter = s_mk_kind_map.begin();
       iter != s_mk_kind_map.end();
       ++iter)
  {
    market_kind *mk = iter->second;
    for (mk_info_sort_list_iter iter = mk->mk_info_sort_list_.begin();
         iter != mk->mk_info_sort_list_.end();
         ++iter)
    {
      // 最多看到40个自己寄售的物品
      if (*count >= 40) break;

      market_info *mi = *iter;

      if (mi->char_id_ != player->id()) continue;

      (*count)++;
      market_module::fetch_market_info(mi, os);
    }
  }
  return player->send_respond_ok(RES_GET_SELF_SALE_LIST, &os);
}
int market_module::clt_search_market_by_name(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::market_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  in_stream is(msg, len);
  short page = 0;
  char name[128] = {0};
  stream_istr si(name, sizeof(name));
  is >> page >> si;
  if (page <= 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  out_stream os(client::send_buf, client::send_buf_len);
  os << page;
  int *count = (int *)os.wr_ptr();
  os << 0;
  char *every_cnt = os.wr_ptr();
  os << (char)0;

  for (mk_kind_map_itor iter = s_mk_kind_map.begin();
       iter != s_mk_kind_map.end();
       ++iter)
  {
    market_kind *mk = iter->second;

    for (mk_info_sort_list_iter iter = mk->mk_info_sort_list_.begin();
         iter != mk->mk_info_sort_list_.end();
         ++iter)
    {
      market_info *mi = *iter;

      if (clsid::is_fa_bao(mi->item_->cid_))
      {
        fa_bao_cfg_obj *fbco = item_config::instance()->get_fa_bao(mi->item_->cid_);
        if (fbco == NULL || ::strstr(fbco->name_, name) == NULL)
          continue;
      }else
      {
        item_cfg_obj *ico = item_config::instance()->get_item(mi->item_->cid_);
        if (ico == NULL || ::strstr(ico->name_, name) == NULL)
          continue;
      }

      (*count)++;
      if (*count > (page - 1) * global_param_cfg::market_one_page_amt * 2
          && *count <= page * global_param_cfg::market_one_page_amt * 2)
      {
        market_module::fetch_market_info(mi, os);
        (*every_cnt)++;
      }
    }
  }

  return player->send_respond_ok(RES_SEARCH_MARKET_BY_NAME, &os);
}

