#include "player_obj.h"
#include "item_obj.h"
#include "product_cfg.h"
#include "package_module.h"
#include "notice_module.h"
#include "behavior_id.h"
#include "istream.h"
#include "sys_log.h"
#include "error.h"
#include "message.h"

// Defines
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

int player_obj::clt_produce_item(const char *msg, const int len)
{
  in_stream is(msg, len);
  int item_clsid = 0;

  is >> item_clsid;
  const product *pct = product_cfg::instance()->get_product(item_clsid);
  if (pct == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (this->is_money_enough(M_COIN, pct->expend_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_COIN_NOT_ENOUGH);
  ilist_node<pair_t<int> > *cur_mat = pct->material_.head();
  for (; cur_mat != NULL; cur_mat = cur_mat->next_)
  {
    if (package_module::calc_item_amount(this, cur_mat->value_.first_)
        < cur_mat->value_.second_)
      return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CAI_LIAO_NOT_ENOUGH);
  }

  if (package_module::would_be_full(this,
                                    PKG_PACKAGE,
                                    item_clsid,
                                    1,
                                    pct->bind_))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  cur_mat = pct->material_.head();
  for (; cur_mat != NULL; cur_mat = cur_mat->next_)
  {
    package_module::do_remove_item(this,
                                   cur_mat->value_.first_,
                                   cur_mat->value_.second_,
                                   ITEM_LOSE_PRODUCE,
                                   item_clsid);
  }
  this->do_lose_money(pct->expend_, M_COIN, MONEY_LOSE_PRODUCE, 0, 0, 0);

  package_module::do_insert_item(this,
                                 PKG_PACKAGE,
                                 item_clsid,
                                 1,
                                 pct->bind_,
                                 ITEM_GOT_PRODUCE,
                                 0,
                                 0);

  if (pct->notify_)
  {
    item_obj item;
    item.cid_ = item_clsid;
    item.amount_ = 1;
    item.bind_ = pct->bind_;
    notice_module::produce_special_item(this->id(), this->name(), &item);
  }

  return this->send_respond_ok(RES_PRODUCE_ITEM);
}
