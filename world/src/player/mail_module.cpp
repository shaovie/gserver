#include "mail_module.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "account_info.h"
#include "char_info.h"
#include "package_module.h"
#include "item_obj.h"
#include "behavior_id.h"
#include "mail_info.h"
#include "sys.h"
#include "db_proxy.h"
#include "istream.h"
#include "global_macros.h"
#include "message.h"
#include "sys_log.h"
#include "error.h"
#include "util.h"
#include "all_char_info.h"
#include "time_util.h"
#include "client.h"
#include "load_json.h"
#include "ilist.h"
#include "clsid.h"
#include "global_param_cfg.h"
#include "mail_config.h"

#define ONCE_SEND_MAIL_AMT 50

static ilog_obj *s_log = sys_log::instance()->get_ilog("mail");
static ilog_obj *e_log = err_log::instance()->get_ilog("mail");

char mail_module::replace_str[MAX_REPLACE_SIZE][MAX_REPLACE_STR + 1];
char mail_module::replace_value[MAX_REPLACE_SIZE][MAX_REPLACE_STR + 1];

// > 1 requset pick up to db will cause return_handle wrong
static ilist<int> s_mail_had_pick_up_attach_list; // mail_id

class mail_timer : public ev_handler
{
public:
  mail_timer()
  {
    player_mgr::instance()->get_ol_char_list(this->ol_char_list_);
  }

  virtual int handle_timeout(const time_value &)
  {
    for (int i = 0; i < ONCE_SEND_MAIL_AMT; ++i)
    {
      if (this->ol_char_list_.empty()) return -1;
      const int ol_char_id = this->ol_char_list_.pop_front();
      player_obj *player = player_mgr::instance()->find(ol_char_id);
      if (player == NULL) continue;
      mail_module::to_get_system_mail_list(player);
    }
    return 0;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    delete this;
    return 0;
  }
private:
  ilist<int> ol_char_list_;
};

int mail_module::dispatch_msg(player_obj *player,
                              const int msg_id,
                              const char *msg,
                              const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = mail_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_MAIL_LIST,      clt_get_mail_list);
    SHORT_CODE(REQ_SEND_MAIL,             clt_send_mail);
    SHORT_CODE(REQ_OBTAIN_MAIL_DETAIL,    clt_obtain_mail_detail);
    SHORT_CODE(REQ_PICK_UP_MAIL_ATTACH,   clt_pick_up_mail_attach);
    SHORT_CODE(REQ_DELETE_MAIL,           clt_delete_mail);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int mail_module::clt_get_mail_list(player_obj *player, const char *, const int )
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_GET_MAIL_LIST, &os);
  return 0;
}
int mail_module::handle_db_get_mail_list_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt == 0) return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  char *wr_ptr = os.wr_ptr();
  os.wr_ptr(sizeof(int));

  int count = 0;
  for (int i = 0; i < res_cnt; ++i)
  {
    char header_tmp[sizeof(mail_header) + 4] = {0};
    stream_istr header_istr(header_tmp, sizeof(header_tmp));
    is >> header_istr;

    in_stream mail_is(header_tmp, header_istr.str_len());
    mail_header mi;
    mail_is >> &mi;
    if (mail_module::do_build_mail_head_info(&mi, os) == 0)
      count++;
  }
  *((int*)wr_ptr) = count;
  player->send_respond_ok(RES_OBTAIN_MAIL_LIST, &os);
  return 0;
}
int mail_module::do_build_mail_head_info(mail_header *mi, out_stream &out)
{
  out << mi->id_
    << stream_ostr(mi->sender_name_, ::strlen(mi->sender_name_))
    << stream_ostr(mi->title_, ::strlen(mi->title_))
    << mi->send_time_
    << mi->readed_
    << mi->has_attach_
    << mi->mail_type_;
  return 0;
}
int mail_module::clt_send_mail(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::mail_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  char recv_name[MAX_NAME_LEN + 1] = {0};
  stream_istr rn_si(recv_name, sizeof(recv_name));
  in_stream is(msg, len);
  is >> rn_si;

  char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(recv_name);
  if (cbi == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_NOT_HAVE_CHAR);

  const int recv_id = cbi->char_id_;
  if (recv_id == player->id())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MAIL_RECEIVER_IS_SELF);

  char title[MAX_MAIL_TITLE_LEN + 1] = {0};
  stream_istr tl_si(title, sizeof(title));
  char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
  stream_istr ct_si(content, sizeof(content));
  int coin = 0, diamond = 0, attach_num = 0;
  is >> coin >> diamond >> tl_si >> ct_si >> attach_num;
  if (attach_num > MAX_ATTACH_NUM)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  int item_num[MAX_ATTACH_NUM] = {0};
  int item_id[MAX_ATTACH_NUM] = {0};
  for (int i = 0; i < attach_num; ++i)
  {
    is >> item_id[i] >> item_num[i];
    if (item_id[i] <= 0
        || item_num[i] <= 0)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }
  // 附件中物品id不能重复
  for (int i = 0; i < attach_num ; ++i)
  {
    for (int j = i + 1; j < attach_num; ++j)
      if (item_id[i] == item_id[j])
        return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }
  // 判断邮件是否正常
  if (rn_si.str_len() == 0 || rn_si.str_len() > MAX_NAME_LEN
      || !util::verify_name(rn_si.str())
      || tl_si.str_len() == 0 || tl_si.str_len() > MAX_MAIL_TITLE_LEN
      || !util::verify_db(tl_si.str())
      || ct_si.str_len() > MAX_MAIL_CONTENT_LEN
      || !util::verify_db(ct_si.str()))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_INPUT_IS_ILLEGAL);

  // 对附件中物品进行检查
  for (int i = 0; i < attach_num; ++i)
  {
    item_obj *io = package_module::find_item(player, item_id[i]);
    if (NULL == io
        || io->bind_ == BIND_TYPE
        || io->amount_ < item_num[i])
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MAIL_ATTACHMENT);
  }

  if (coin < 0 || diamond < 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  int ret = player->is_money_enough(M_DIAMOND, diamond);
  if (ret != 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  ret = player->is_money_enough(M_COIN, coin + global_param_cfg::send_mail_cost);
  if (ret != 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  item_obj attach_list[MAX_ATTACH_NUM];
  for (int i = 0; i < attach_num; ++i)
  {
    item_obj *io = package_module::find_item(player, item_id[i]);
    attach_list[i] = *io;
    attach_list[i].amount_ = item_num[i];
  }
  // 发送邮件
  mail_module::do_send_mail(recv_id, player->id(),
                            player->name(), tl_si.str(), ct_si.str(),
                            mail_info::MAIL_TYPE_PRIVATE,
                            coin, diamond, 0,
                            attach_num, attach_list,
                            player->db_sid(), time_util::now);
  if (ret != 0) return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  // 扣除发件人附件中的物品
  for (int i = 0; i < attach_num; ++i)
  {
    item_obj *io = package_module::find_item(player, item_id[i]);
    package_module::do_remove_item(player,
                                   io,
                                   item_num[i],
                                   ITEM_LOSE_MAIL,
                                   recv_id);
  }

  // 扣除邮费
  player->do_lose_money(coin + global_param_cfg::send_mail_cost,
                        M_COIN,
                        MONEY_LOSE_MAIL,
                        recv_id, 0, 0);
  player->do_lose_money(diamond,
                        M_DIAMOND,
                        MONEY_LOSE_MAIL_ATTACH,
                        recv_id, 0, 0);

  player_obj *reciver = player_mgr::instance()->find(recv_id);
  if (reciver != NULL)
    mail_module::do_notify_haved_new_mail(reciver, 1);
  return player->send_respond_ok(RES_SEND_MAIL);
}
int mail_module::clt_obtain_mail_detail(player_obj *player, const char *msg, const int len)
{
  int mail_id = 0;
  in_stream is(msg, len);
  is >> mail_id;
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << mail_id;
  db_proxy::instance()->send_request(player->id(), REQ_GET_MAIL_INFO, &os);
  return 0;
}
int mail_module::handle_db_get_mail_detail_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt <= 0) return 0;

  char db_info_bf[sizeof(mail_info) + 4] = {0};
  stream_istr db_info_si(db_info_bf, sizeof(db_info_bf));
  is >> db_info_si;

  in_stream mail_is(db_info_bf, db_info_si.str_len());
  mail_info mi;
  mail_is >> &mi;

  out_stream os(client::send_buf, client::send_buf_len);
  mail_module::do_build_mail_detail_info(player, &mi, os);
  player->send_respond_ok(RES_OBTAIN_MAIL_DETAIL, &os);
  mail_module::do_set_mail_readed(player, mi.id_);
  return 0;
}
void mail_module::do_build_mail_detail_info(player_obj *,
                                            mail_info *mi,
                                            out_stream &out)
{
  out << mi->id_
    << stream_ostr(mi->mail_header_.sender_name_, ::strlen(mi->mail_header_.sender_name_))
    << stream_ostr(mi->mail_header_.title_, ::strlen(mi->mail_header_.title_))
    << stream_ostr(mi->mail_detail_.content_, ::strlen(mi->mail_detail_.content_))
    << mi->mail_detail_.coin_
    << mi->mail_detail_.diamond_
    << mi->mail_detail_.b_diamond_
    << mi->mail_detail_.attach_num_;

  ilist<item_obj*> attach_list;
  mail_module::json2item(attach_list, mi->mail_detail_.items_, sizeof(mi->mail_detail_.items_));
  while (!attach_list.empty())
  {
    item_obj *io = attach_list.pop_front();
    package_module::do_build_item_info(io, out);
    item_obj_pool::instance()->release(io);
  }
}
int mail_module::do_set_mail_readed(player_obj *player, int mail_id)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << mail_id;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_MAIL_READED, &os);
  return 0;
}
int mail_module::clt_pick_up_mail_attach(player_obj *player,
                                         const char *msg,
                                         const int len)
{
  if (!global_param_cfg::mail_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  // check for syn
  in_stream is(msg, len);
  int mail_id = 0;
  is >> mail_id;
  if (s_mail_had_pick_up_attach_list.find(mail_id))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MAIL_ATTACHMENT);

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << mail_id;
  db_proxy::instance()->send_request(player->id(), REQ_GET_MAIL_ATTACH, &os);
  return 0;
}
int mail_module::handle_db_get_mail_attach_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt <= 0) return 0;

  char db_info_bf[sizeof(mail_detail) + 4] = {0};
  stream_istr db_info_si(db_info_bf, sizeof(db_info_bf));
  is >> db_info_si;

  in_stream mail_is(db_info_bf, db_info_si.str_len());
  mail_detail md;
  int sender_id = 0;
  mail_is >> &md >> sender_id;
  if (md.char_id_ != player->id())
  {
    e_log->rinfo("char %d get mail %d detail, but real owner id is %d!",
                 player->id(), md.id_, md.char_id_);
    return 0;
  }

  if (s_mail_had_pick_up_attach_list.find(md.id_))
    return 0;

  const int ret = mail_module::do_get_mail_attach(player, md, sender_id);
  if (ret != 0)
  {
    player->send_respond_err(NTF_OPERATE_RESULT, ret);
    return 0;
  }

  s_mail_had_pick_up_attach_list.push_back(md.id_);
  out_stream os(client::send_buf, client::send_buf_len);
  os << md.id_;
  player->send_respond_ok(RES_PICK_UP_MAIL_ATTACH, &os);
  return 0;
}
int mail_module::do_get_mail_attach(player_obj *player, mail_detail &md, const int sender_id)
{
  // 检查背包空间
  ilist<item_obj*> item_list;
  mail_module::json2item(item_list, md.items_, sizeof(md.items_));
  int insert_item_num = 0;
  for (ilist_node<item_obj *> *itor = item_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    item_obj *io = itor->value_;
    g_item_amount_bind[0][insert_item_num] = io->cid_;
    g_item_amount_bind[1][insert_item_num] = io->amount_;
    g_item_amount_bind[2][insert_item_num] = io->bind_;
    insert_item_num++;
  }
  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    g_item_amount_bind[0],
                                    g_item_amount_bind[1],
                                    g_item_amount_bind[2],
                                    insert_item_num))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  // 检查金钱
  if (player->is_money_upper_limit(M_COIN, md.coin_)
      || player->is_money_upper_limit(M_DIAMOND, md.diamond_)
      || player->is_money_upper_limit(M_BIND_DIAMOND, md.b_diamond_))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MONEY_UPPER_LIMIT);

  while (!item_list.empty())
  {
    item_obj *item = item_list.pop_front();
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   item,
                                   ITEM_GOT_MAIL,
                                   sender_id,
                                   md.id_);
  }
  if (md.coin_ > 0)
    player->do_got_money(md.coin_, M_COIN, MONEY_GOT_MAIL, sender_id);
  if (md.diamond_ > 0)
    player->do_got_money(md.diamond_, M_DIAMOND, MONEY_GOT_MAIL, sender_id);
  if (md.b_diamond_ > 0)
    player->do_got_money(md.b_diamond_, M_BIND_DIAMOND, MONEY_GOT_MAIL, sender_id);

  mail_module::do_set_mail_attach_removed(player, md.id_);
  return 0;
}
void mail_module::do_set_mail_attach_removed(player_obj *player, int mail_id)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << mail_id;
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_MAIL_ATTACH, &os);
}
int mail_module::clt_delete_mail(player_obj *player, const char *msg, const int len)
{
  if (!global_param_cfg::mail_valid)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_MARKET_CLOSED);

  int mail_cnt = 0;
  int mail_id  = 0;
  in_stream is(msg, len);
  is >> mail_cnt;
  if (mail_cnt <= 0 || mail_cnt > MAX_MAIL_COUNT)
  {
    e_log->wning("char %d delete mail cnt:%d is invalid!", player->id(), mail_cnt);
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << time_util::now << mail_cnt;
  ilist<int> mail_id_list;
  for (int i = 0; i < mail_cnt; ++i)
  {
    is >> mail_id;
    os << mail_id;
    mail_id_list.push_back(mail_id);
  }
  db_proxy::instance()->send_request(player->id(), REQ_UPDATE_MAIL_DELETE, &os);
  os.set_wr_ptr(0);
  os << mail_cnt;
  while (!mail_id_list.empty())
  {
    mail_id = mail_id_list.pop_front();
    os << mail_id;
    s_mail_had_pick_up_attach_list.remove(mail_id);
  }
  return player->send_respond_ok(RES_DELETE_MAIL, &os);
}
void mail_module::on_new_day()
{
  if (s_mail_had_pick_up_attach_list.size() < 512) return ;
  int c = s_mail_had_pick_up_attach_list.size() / 2;
  for (int i = 0; i < c; ++i)
    s_mail_had_pick_up_attach_list.pop_front();
}
void mail_module::on_enter_game(player_obj *player)
{
  mail_module::to_check_haved_new_mail(player);
  mail_module::to_get_system_mail_list(player);
}
void mail_module::to_check_haved_new_mail(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_CHECK_NEW_MAIL, &os);
}
int mail_module::handle_db_get_check_new_mail_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;

  if (res_cnt > 0)
  {
    char mail_count[sizeof(int) + 4] = {0};
    stream_istr count_istr(mail_count, sizeof(mail_count));
    is >> count_istr;
    int *p = (int*)mail_count;  // avoid warning
    if (*p > 0)
      mail_module::do_notify_haved_new_mail(player, *p);
  }
  return 0;
}
int mail_module::to_get_system_mail_list(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  int max_count = client::send_buf_len / sizeof(mail_info) - 1;
  os << player->db_sid()
    << player->id()
    << (player->check_sys_mail_time() > player->c_time()
        ? player->check_sys_mail_time()
        : player->c_time())
    << (char)(max_count > 126 ? 126 : max_count)
    << player->lvl();
  if (db_proxy::instance()->send_request(player->id(), REQ_GET_SYSTEM_MAIL_LIST, &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
int mail_module::handle_db_get_system_mail_list_result(player_obj *player, in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;

  for (int i = 0; i < res_cnt; ++i)
  {
    char mail_temp[sizeof(mail_info) + 4] = {0};
    stream_istr mail_istr(mail_temp, sizeof(mail_temp));
    is >> mail_istr;

    in_stream mail_is(mail_temp, mail_istr.str_len());
    mail_info mi;
    mail_is >> &mi;

    if (player->get_system_mail_list_.find(mi.id_)) continue;
    mi.char_id_ = player->id();
    mi.mail_header_.mail_type_ = mail_info::MAIL_TYPE_GM;

    player->get_system_mail_list_.push_back(mi.id_);
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << &mi;
    db_proxy::instance()->send_request(player->id(), REQ_INSERT_MAIL, &os);
  }

  if (res_cnt > 0)
    mail_module::do_notify_haved_new_mail(player, res_cnt);

  player->check_sys_mail_time(time_util::now);
  player->db_save_char_extra_info();
  return 0;
}
void mail_module::do_notify_haved_new_mail(player_obj *player, const int cnt)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << cnt;
  player->send_request(NTF_HAVED_NEW_MAIL, &os);
}
// ============================================== mail static ================================================
/** "1":{
 *   "m_cid":1233434 //
 *   "m_bind":5,     //
 *   "m_amt":6,      // amount
 *   "m_extra":{     // extra_info
 *    }
 *   },
 * "2":{
 *   "m_cid":1233434 //
 *   "m_bind":5,     //
 *   "m_amt":6,      // amount
 *   "m_extra":{     // extra info
 *    }
 *   },
 */
void mail_module::item2json(item_obj *attach_list, int attach_num, char *items, const int items_size)
{
  if (attach_list == NULL
      || attach_num <= 0)
    return ;

  Json::Value root;
  Json::Reader j_reader;
  j_reader.parse(items, items + items_size, root, false);

  for (int i = 0; i < attach_num; ++i)
  {
    char idx[] = {0, 0};
    sprintf(idx, "%d", i + 1);
    root[idx]["m_cid"]  = attach_list[i].cid_;
    root[idx]["m_bind"] = attach_list[i].bind_;
    root[idx]["m_amt"]  = attach_list[i].amount_;
    root[idx]["m_coe"]  = attach_list[i].attr_float_coe_;
    if (attach_list[i].extra_info_ != NULL)
    {
      Json::Value jv;
      Json::Reader jr;
      jr.parse(attach_list[i].extra_info_->rd_ptr(),
               attach_list[i].extra_info_->rd_ptr() + attach_list[i].extra_info_->length(),
               jv,
               false);
      root[idx]["m_extra"]= jv;
    }
  }

  // write to str
  Json::FastWriter j_writer;
  static std::string new_str;
  new_str = j_writer.write(root);
  strncpy(items, new_str.c_str(), items_size - 1);
}
int mail_module::json2item(ilist<item_obj*> &attach_list, const char *items, const int items_size)
{
  if (items == NULL
      || ::strlen(items) == 0)
    return 0;
  Json::Value root;
  Json::Reader j_reader;
  if (!j_reader.parse(items, items + items_size, root, false))
    return -1;

  int ret = 0;
  for (int i = 1; i <= MAX_ATTACH_NUM; ++i)
  {
    char idx[] = {0, 0};
    ::sprintf(idx, "%d", i);
    if (root[idx].empty()) break;

    item_obj *io = item_obj_pool::instance()->alloc();
    io->cid_    = root[idx]["m_cid"].asInt();
    io->bind_   = root[idx]["m_bind"].asInt();
    io->amount_ = root[idx]["m_amt"].asInt();
    if (!item_config::instance()->find(io->cid_)
        || (io->bind_ != UNBIND_TYPE && io->bind_ != BIND_TYPE)
        || io->amount_ <= 0)
    {
      ret = -1;
      item_obj_pool::instance()->release(io);
      continue;
    }
    io->attr_float_coe_ = root[idx]["m_coe"].asInt();
    if (root[idx].isMember("m_extra")
        && !root[idx]["m_extra"].empty())
    {
      Json::FastWriter j_writer;
      std::string str = j_writer.write(root[idx]["m_extra"]);
      if (!str.empty())
      {
        io->extra_info_ = mblock_pool::instance()->alloc(str.length() + 1);
        io->extra_info_->copy(str.c_str(), str.length());
        io->extra_info_->set_eof();
      }
    }
    attach_list.push_back(io);
  }
  return ret;
}
void mail_module::replace_mail_info(const char *mail_info,
                                    const char replace_str[][MAX_REPLACE_STR + 1],
                                    const char replace_value[][MAX_REPLACE_STR + 1],
                                    const int size,
                                    char *new_mail_info)
{
  std::string mail_tmp(mail_info);
  std::string::size_type pos;
  for (int i = 0; i < size; ++i)
  {
    if ((pos = mail_tmp.find(replace_str[i])) != std::string::npos)
      mail_tmp.replace(pos, strlen(replace_str[i]), replace_value[i]);
  }
  ::strncpy(new_mail_info, mail_tmp.c_str(), mail_tmp.size());
}
void mail_module::do_send_mail(const int recv_id, const int sender_id,
                               const char *sender_name, const char *title, const char *content,
                               const int mail_type,
                               const int coin, const int diamond, const int b_diamond,
                               const int attach_num, item_obj *attach_list,
                               const int db_session_id, const int send_time, const int lvl_limit)
{
  char items[ALL_ITEMS_LEN + 1] = {0};
  mail_module::item2json(attach_list, attach_num, items, sizeof(items));

  mail_info mi(recv_id, sender_id,
               sender_name, title, content,
               send_time, 0,
               attach_num, mail_type,
               coin, diamond, b_diamond,
               0, items, lvl_limit);

  out_stream os(client::send_buf, client::send_buf_len);
  os << db_session_id << &mi;
  db_proxy::instance()->send_request(recv_id, REQ_INSERT_MAIL, &os);
}
int mail_module::do_send_mail(const int recv_id, const int sender_id,
                              const char *sender_name, const char *title, const char *content,
                              const int mail_type,
                              const int coin, const int diamond, const int b_diamond,
                              const char *items,
                              const int db_session_id, const int send_time, const int lvl_limit)
{
  // check items
  ilist<item_obj*> attach_list;
  int ret = 0;
  if (items != NULL)
    ret = mail_module::json2item(attach_list, items, ::strlen(items));
  int attach_num = attach_list.size();
  while (!attach_list.empty())
    item_obj_pool::instance()->release(attach_list.pop_front());
  if (ret != 0 || attach_num > MAX_ATTACH_NUM)
    return -1;

  mail_info mi(recv_id, sender_id,
               sender_name, title, content,
               send_time, 0,
               attach_num, mail_type,
               coin, diamond, b_diamond,
               0, items == NULL ? "" : items, lvl_limit);

  out_stream os(client::send_buf, client::send_buf_len);
  os << db_session_id << &mi;
  db_proxy::instance()->send_request(recv_id, REQ_INSERT_MAIL, &os);
  return 0;
}
void mail_module::do_gen_mail_item(item_obj &io,
                                   const int item_cid,
                                   const int amount,
                                   const char bind_type)
{
  io.cid_ = item_cid;
  io.amount_ = amount;
  io.bind_ = bind_type;
}
int mail_module::do_send_mail_2_all(const char *sender_name, const char *title, const char *content,
                                    const int coin, const int diamond, const int b_diamond,
                                    const char *items,
                                    const short lvl_limit)
{
  const int ret = mail_module::do_send_mail(0, mail_info::MAIL_SEND_SYSTEM_ID,
                                            sender_name, title, content,
                                            mail_info::MAIL_TYPE_CACHE,
                                            coin, diamond, b_diamond,
                                            items == NULL ? "" : items,
                                            0, time_util::now, lvl_limit);
  if (ret != 0) return ret;

  sys::r->schedule_timer(new mail_timer(),
                         time_value((rand() % 5 + 1) * 2, rand() % 999 * 1000),
                         time_value(1, 0));
  return 0;
}
void mail_module::do_send_mail(const int char_id,
                               const char career,
                               const int db_sid,
                               const int mail_type,
                               const char *sender_name,
                               const char *title,
                               const char *content,
                               ilist<item_amount_bind_t> &attach_list)
{
  for (ilist_node<item_amount_bind_t> *itor = attach_list.head(); itor != NULL; )
  {
    int coin = 0;
    int diamond = 0;
    int b_diamond = 0;
    item_obj il[MAX_ATTACH_NUM];
    int att_amount = 0;
    for (int i = 0; i < MAX_ATTACH_NUM && itor != NULL; ++i)
    {
      item_amount_bind_t &award = itor->value_;
      if (item_config::item_is_money(award.cid_))
      {
        player_obj::do_exchange_item_money(award.cid_,
                                           award.amount_,
                                           coin,
                                           diamond,
                                           b_diamond);
        itor = itor->next_;
        continue;
      }
      int item_cid = award.cid_;
      if (clsid::is_char_equip(item_cid))
        item_cid = clsid::get_equip_cid_by_career(career, item_cid);
      item_obj *io = package_module::alloc_new_item(char_id,
                                                    item_cid,
                                                    award.amount_,
                                                    award.bind_);
      il[att_amount++] = *io;
      package_module::release_item(io);
      itor = itor->next_;
    }

    if (att_amount > 0
        || coin > 0
        || diamond > 0
        || b_diamond > 0) //
    {
      mail_module::do_send_mail(char_id, mail_info::MAIL_SEND_SYSTEM_ID,
                                sender_name, title, content,
                                mail_type,
                                coin, diamond, b_diamond,
                                att_amount, il,
                                db_sid, time_util::now);
      player_obj *player = player_mgr::instance()->find(char_id);
      if (player != NULL)
        mail_module::do_notify_haved_new_mail(player, 1);
    }
  }
}
