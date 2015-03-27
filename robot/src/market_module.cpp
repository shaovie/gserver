#include "market_module.h"
#include "player.h"
#include "message.h"
#include "item_obj.h"
#include "item_config.h"
#include "clsid.h"

int market_module::dispatch_msg(player *p, const int id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = market_module::FUNC(p, msg, len);    \
  break

  int ret = 0;
  switch(id)
  {
    SHORT_CODE(RES_GET_MARKET_LIST,           handle_error_ret);
    SHORT_CODE(RES_SORT_BY_MONEY,             handle_error_ret);
    SHORT_CODE(RES_SALE_MARKET_ITEM,          handle_error_ret);
    SHORT_CODE(RES_SALE_MARKET_MONEY,         handle_error_ret);
    SHORT_CODE(RES_CANCEL_MARKET_ITEM,        handle_error_ret);
    SHORT_CODE(RES_GET_SELF_SALE_LIST,        handle_get_self_sale_list);
  }
  return ret;
}
int market_module::handle_get_self_sale_list(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int count = 0;
  int market_id = 0;;
  int price = 0;
  char price_type = 0;
  int time = 0;
  int item_id = 0;
  int item_cid = 0;
  is >> count;
  if (count > 0)
    is >> market_id >> price >> price_type >> time >> item_id >> item_cid;

  char market_type = 0;
  if (clsid::is_equip(item_cid))
  {
    equip_cfg_obj *eco = item_config::instance()->get_equip(item_cid);
    if (eco != NULL)
      market_type = eco->part_;
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << market_type << market_id;
  return p->send_request(REQ_CANCEL_MARKET_ITEM, &os);
}
int market_module::handle_error_ret(player *, const char *, const int )
{
  return 0;
}
void market_module::get_market_list(player *p)
{
  char market_part = rand() % 10;
  char career = rand() % 3 + 1;
  short page = 1;

  out_stream os(client::send_buf, client::send_buf_len);
  os << market_part << career << page;

  int value_r = rand() % 6;
  if (value_r == 0)
    os << 0;
  else if (value_r == 1)
    os << 100000;
  else if (value_r == 2)
    os << 500000;
  else if (value_r == 3)
    os << 2000000;
  else if (value_r == 4)
    os << 10000000;
  else if (value_r == 5)
    os << 20000000;

  p->send_request(REQ_GET_MARKET_LIST, &os);
}
void market_module::sort_by_money(player *p)
{
  char market_part = rand() % 10;
  char sort_type = rand() % 2 + 1;
  char career = rand() % 3 + 1;
  short page = 1;

  out_stream os(client::send_buf, client::send_buf_len);
  os << market_part << career << page;

  int value_r = rand() % 6;
  if (value_r == 0)
    os << 0;
  else if (value_r == 1)
    os << 100000;
  else if (value_r == 2)
    os << 500000;
  else if (value_r == 3)
    os << 2000000;
  else if (value_r == 4)
    os << 10000000;
  else if (value_r == 5)
    os << 20000000;

  os << sort_type;

  p->send_request(REQ_SORT_BY_MONEY, &os);
}
void market_module::sale_market_item(player *p)
{
  out_stream os(client::send_buf, client::send_buf_len);
  for (ilist_node<item_obj *> *itor = p->item_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    item_obj *io = itor->value_;
    if (io->bind_ != UNBIND_TYPE) continue;

    os << io->id_ << io->amount_;
  }

  int price = rand() % 1000000 + 1;
  os << price;

  p->send_request(REQ_SALE_MARKET_ITEM, &os);
}
void market_module::sale_market_money(player *p)
{
  out_stream os(client::send_buf, client::send_buf_len);

  int value_r = rand() % 5 + 1;
  if (value_r == 1)
    os << 100000 << 20;
  else if (value_r == 2)
    os << 500000 << 100;
  else if (value_r == 3)
    os << 2000000 << 400;
  else if (value_r == 4)
    os <<  10000000 << 2000;
  else if (value_r == 5)
    os <<  20000000 << 4000;

  p->send_request(REQ_SALE_MARKET_MONEY, &os);
}
void market_module::get_self_sale_list(player *p)
{
  p->send_request(REQ_GET_SELF_SALE_LIST, NULL);
}

