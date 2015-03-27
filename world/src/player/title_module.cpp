#include "title_module.h"
#include "title_info.h"
#include "title_cfg.h"
#include "player_obj.h"
#include "istream.h"
#include "db_proxy.h"
#include "message.h"
#include "sys_log.h"
#include "behavior_id.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("title");
static ilog_obj *e_log = err_log::instance()->get_ilog("title");

int title_module::handle_db_get_title_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;

  for (int i = 0; i < res_cnt; ++i)
  {
    char bf[sizeof(title_info) + 4] = {0};
    stream_istr buff_si(bf, sizeof(bf));
    is >> buff_si;

    title_info *p = (title_info *)bf;
    if (title_cfg::instance()->get_title_cfg_obj(p->title_cid_) == NULL)
      continue;
    player->title_list_.push_back(p->title_cid_);
  }

  return 0;
}
void title_module::add_new_title(player_obj *player, const short title_cid)
{
  if (player->title_list_.find(title_cid)) return;

  if (title_cfg::instance()->get_title_cfg_obj(title_cid) == NULL)
    return;

  player->title_list_.push_back(title_cid);

  title_info ti;
  ti.char_id_   = player->id();
  ti.title_cid_ = title_cid;

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << stream_ostr((char *)&ti, sizeof(title_info));
  db_proxy::instance()->send_request(player->id(), REQ_INSERT_TITLE_INFO, &os);

  os.reset(client::send_buf, client::send_buf_len);
  os << title_cid;
  player->send_request(NTF_ADD_NEW_TITLE, &os);

  if (player->cur_title() != 0)
  {
    player->do_calc_attr_affected_by_title();
    player->on_attr_update(ZHAN_LI_TITLE, title_cid);
  }
}
//======================================================================
int player_obj::clt_get_title_list(const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << (short)this->title_list_.size();
  for (ilist_node<short> *iter = this->title_list_.head();
       iter != NULL;
       iter = iter->next_)
    os << iter->value_;

  return this->send_request(RES_GET_TITLE_LIST, &os);
}
int player_obj::clt_use_title(const char *msg, const int len)
{
  in_stream is(msg, len);
  short title_cid = 0;
  is >> title_cid;

  if (!this->title_list_.find(title_cid))
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  this->cur_title(title_cid);
  this->db_save_char_info();
  this->broadcast_cur_title();

  this->do_calc_attr_affected_by_title();
  this->on_attr_update(ZHAN_LI_TITLE, title_cid);

  return 0;
}
int player_obj::clt_cancel_cur_title(const char *, const int )
{
  if (this->cur_title() == 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL); 

  this->cur_title(0);
  this->db_save_char_info();
  this->broadcast_cur_title();

  this->do_calc_attr_affected_by_title();
  this->on_attr_update(ZHAN_LI_TITLE, 0);
  return 0;
}
void player_obj::broadcast_cur_title()
{
  mblock mb(client::send_buf, client::send_buf_len);
  proto_head *ph = (proto_head *)mb.rd_ptr();
  mb.wr_ptr(sizeof(proto_head));
  mb << this->id() << this->cur_title();
  ph->set(0, NTF_BROADCAST_CUR_TITLE, 0, mb.length());
  this->do_broadcast(&mb, true);
}
