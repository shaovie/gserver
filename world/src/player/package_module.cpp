#include "package_module.h"
#include "sys_log.h"
#include "message.h"
#include "item_obj.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "istream.h"
#include "error.h"
#include "def.h"
#include "behavior_log.h"
#include "item_config.h"
#include "db_proxy.h"
#include "time_util.h"
#include "global_param_cfg.h"
#include "mail_config.h"
#include "mail_module.h"
#include "mail_info.h"
#include "equip_module.h"
#include "dropped_item.h"
#include "scene_config.h"
#include "iu_module.h"
#include "clsid.h"

// Lib header
#include <map>
#include <math.h>
#include <json/json.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("package");
static ilog_obj *e_log = err_log::instance()->get_ilog("package");

int g_item_amount_bind[3][MAX_PACKAGE_CAPACITY];
int g_item_update_info[package_module::UPD_END];

typedef std::map<int, item_obj*> item_map_t;
typedef std::map<int, item_obj*>::iterator item_map_iter;

class pack_info
{
public:
  pack_info() : max_item_id_(0) { }
  int assign_item_id() { return ++this->max_item_id_; }

  int max_item_id_;
  item_map_t item_map_;
};

void package_module::init(player_obj *player)
{ player->pack_info_ = new pack_info(); }
void package_module::destroy(player_obj *player)
{
  item_map_iter iter = player->pack_info_->item_map_.begin();
  for (; iter != player->pack_info_->item_map_.end(); ++iter)
    package_module::release_item(iter->second);
  delete player->pack_info_;
  player->pack_info_ = NULL;
}
item_obj* package_module::alloc_new_item()
{ return item_obj_pool::instance()->alloc(); }
item_obj* package_module::alloc_new_item(const int char_id,
                                         const int item_cid,
                                         const int amount,
                                         const char bind_type)
{
  item_obj *io = item_obj_pool::instance()->alloc();
  io->char_id_  = char_id;
  io->cid_      = item_cid;
  io->amount_   = amount;
  io->bind_     = bind_type;
  package_module::on_got_new_equip(io);
  return io;
}
void package_module::release_item(item_obj *item)
{ item_obj_pool::instance()->release(item); }
int package_module::handle_db_get_item_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  for (int i = 0; i < cnt; ++i)
  {
    item_obj *io = package_module::alloc_new_item();
    is.rd_ptr(sizeof(short)); // Refer: db_proxy::proxy_obj.cpp::proc_result::for (mblock
    is >> io;
    if (io->id_ > player->pack_info_->max_item_id_)
      player->pack_info_->max_item_id_ = io->id_;
    player->pack_info_->item_map_.insert(std::make_pair(io->id_, io));
  }
  return 0;
}
int package_module::dispatch_msg(player_obj *player,
                                 const int msg_id,
                                 const char *msg,
                                 const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = package_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_ITEM_LIST, clt_obtain_item_list);
    SHORT_CODE(REQ_MOVE_ITEM, clt_move_item);
    SHORT_CODE(REQ_DESTROY_ITEM, clt_destroy_item);
    SHORT_CODE(REQ_ENLARGE_CAPACITY, clt_enlarge_capacity);
    SHORT_DEFAULT;
  }
  return ret;
}
int package_module::clt_obtain_item_list(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  char package = 0;
  is >> package;
  if (package == PKG_EQUIP
      || package == PKG_PACKAGE)
    e_log->wning("client request equip|package item list!");

  out_stream os(client::send_buf, client::send_buf_len);
  os << package << package_module::package_capacity(player, package);
  package_module::do_build_item_list(player, package, os);
  return player->send_request(RES_OBTAIN_ITEM_LIST, &os);
}
int package_module::clt_move_item(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int item_id = 0;
  char to_pack = 0;
  is >> item_id >> to_pack;

  item_obj *item = package_module::find_item(player, item_id);
  if (item == NULL
      || !package_module::can_move_to(item->pkg_, to_pack))
    return player->send_respond_err(NTF_OPERATE_RESULT,
                                    ERR_CLIENT_OPERATE_ILLEGAL);

  if (package_module::would_be_full(player,
                                    to_pack,
                                    item->cid_,
                                    item->amount_,
                                    item->bind_))
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  package_module::do_move_item(player, item, to_pack);
  return 0;
}
int package_module::clt_destroy_item(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  int item_id = 0;
  is >> item_id;

  item_obj *item = package_module::find_item(player, item_id);
  if (item == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT,
                                    ERR_CLIENT_OPERATE_ILLEGAL);
  package_module::do_remove_item(player, item, item->amount_, ITEM_LOSE_DESTROY, 0);
  return 0;
}
int package_module::clt_enlarge_capacity(player_obj *player, const char *msg, const int len)
{
  in_stream is(msg, len);
  char package = 0;
  is >> package;
  if (package != PKG_PACKAGE
      && package != PKG_STORAGE)
    return player->send_respond_err(NTF_OPERATE_RESULT,
                                    ERR_CLIENT_OPERATE_ILLEGAL);

  int init_cap = global_param_cfg::pkg_cap;
  if (package == PKG_STORAGE)
    init_cap = global_param_cfg::storage_cap;
  int cost = global_param_cfg::enlarge_cap_cost *
    ((package_module::package_capacity(player, package) - init_cap) /
     global_param_cfg::enlarge_cap_num + 1);
  if (cost < 0 ) cost = 0;
  int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND, cost);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);

  short new_cap = 0;
  if (package == PKG_PACKAGE)
  {
    if (player->char_info_->pkg_cap_ + global_param_cfg::enlarge_cap_num
        > global_param_cfg::max_pkg_cap)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAPACITY_OUT_OF_LIMIT);
    player->char_info_->pkg_cap_ += global_param_cfg::enlarge_cap_num;
    player->db_save_char_info();
    new_cap = player->char_info_->pkg_cap_;
  }else if (package == PKG_STORAGE)
  {
    if (player->char_info_->storage_cap_ + global_param_cfg::enlarge_cap_num
        > global_param_cfg::max_storage_cap)
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CAPACITY_OUT_OF_LIMIT);
    player->char_info_->storage_cap_ += global_param_cfg::enlarge_cap_num;
    player->db_save_char_info();
    new_cap = player->char_info_->storage_cap_;
  }

  player->do_lose_money(cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_ENLARGE_CAPACITY,
                        package,
                        0,
                        0);

  out_stream os(client::send_buf, client::send_buf_len);
  os << package << new_cap;
  return player->send_request(NTF_UPDATE_PACK_CAPACITY, &os);
}
//= logic
void package_module::on_enter_game(player_obj *player)
{
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << (char)PKG_EQUIP << package_module::package_capacity(player, PKG_EQUIP);
    package_module::do_build_item_list(player, PKG_EQUIP, os);
    player->send_request(NTF_PUSH_ITEM_LIST, &os);
  }
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << (char)PKG_PACKAGE << package_module::package_capacity(player, PKG_PACKAGE);
    package_module::do_build_item_list(player, PKG_PACKAGE, os);
    player->send_request(NTF_PUSH_ITEM_LIST, &os);
  }
  return ;
}
void package_module::do_build_item_list(player_obj *player,
                                        const char package,
                                        out_stream &os)
{
  short *cnt = (short *)os.wr_ptr();
  os << (short)0;
  for (item_map_iter itor = player->pack_info_->item_map_.begin();
       itor != player->pack_info_->item_map_.end();
       ++itor)
  {
    if (itor->second->pkg_ != package) continue;
    package_module::do_build_item_info(itor->second, os);
    ++(*cnt);
  }
}
bool package_module::can_move_to(const int from, const int to)
{
  if ((from == PKG_EQUIP && to == PKG_PACKAGE)
      || (from == PKG_STORAGE && to == PKG_PACKAGE)
     )
    return true;
  if (from == PKG_PACKAGE
      && (to == PKG_EQUIP
          || to == PKG_STORAGE))
    return true;
  return false;
}
bool package_module::would_be_full(player_obj *player,
                                   const char package,
                                   const int item_cid[],
                                   const int amount[],
                                   const int bind_type[],
                                   const int size)
{
  int total_need = 0;
  int used_space = 0;
  for (int i = 0; i < size; ++i)
  {
    used_space = 0;
    total_need += package_module::need_space(player,
                                             used_space,
                                             package,
                                             item_cid[i],
                                             amount[i],
                                             bind_type[i]);
  }
  int left = package_module::package_capacity(player, package) - used_space;
  if (total_need > left) return true;
  return false;
}
bool package_module::would_be_full(player_obj *player,
                                   const char package,
                                   const int item_cid,
                                   const int amount,
                                   const int bind_type)
{
  int used_space = 0;
  int need = package_module::need_space(player,
                                        used_space,
                                        package,
                                        item_cid,
                                        amount,
                                        bind_type);
  int left = package_module::package_capacity(player, package) - used_space;
  if (need > left) return true;
  return false;
}
int package_module::need_space(player_obj *player,
                               int &used_space,
                               const char package,
                               const int item_cid,
                               const int amount,
                               const char bind_type)
{
  int left_item_amount = amount;
  int dj_limit = item_config::instance()->dj_upper_limit(item_cid);
  for (item_map_iter itor = player->pack_info_->item_map_.begin();
       itor != player->pack_info_->item_map_.end();
       ++itor)
  {
    if (itor->second->pkg_ != package) continue;
    ++used_space;
    if (itor->second->cid_ == item_cid
        && itor->second->bind_ == bind_type
        && itor->second->amount_ < dj_limit)
      left_item_amount = left_item_amount + itor->second->amount_ - dj_limit;
    if (left_item_amount <= 0) return 0;
  }
  return (int)ceil(1.0 * left_item_amount / dj_limit);
}
int package_module::used_space(player_obj *player, const char package)
{
  int used_space = 0;
  for (item_map_iter itor = player->pack_info_->item_map_.begin();
       itor != player->pack_info_->item_map_.end();
       ++itor)
  {
    if (itor->second->pkg_ != package) continue;
    ++used_space;
  }
  return used_space;
}
short package_module::package_capacity(player_obj *player, const char package)
{
  if (package == PKG_EQUIP)
    return PART_END - 1;
  else if (package == PKG_PACKAGE)
    return player->char_info_->pkg_cap_;
  else if (package == PKG_STORAGE)
    return player->char_info_->storage_cap_;
  return 0;
}
void package_module::on_item_update(player_obj *player,
                                    const int type,
                                    item_obj *item,
                                    const int arg[],
                                    const int size,
                                    const int behavior_sub_type)
{
  if (package_module::on_item_update_for_db(player,
                                            type,
                                            item) == 0)
  {
    package_module::on_item_update_for_clt(player,
                                           type,
                                           item,
                                           arg,
                                           size,
                                           behavior_sub_type);
  }
}
int package_module::on_item_update_for_db(player_obj *player,
                                          const int type,
                                          item_obj *item)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid();
  if (type == DEL_ITEM)
  {
    os << player->id() << item->id_;
    return db_proxy::instance()->send_request(player->id(), REQ_DELETE_ITEM, &os);
  }else if (type == ADD_ITEM)
  {
    os << item;
    return db_proxy::instance()->send_request(player->id(), REQ_INSERT_ITEM, &os);
  }else if (type == UPD_ITEM)
  {
    os << item;
    return db_proxy::instance()->send_request(player->id(), REQ_UPDATE_ITEM, &os);
  }else
  {
    e_log->rinfo("unknow type:%d in %s", type, __func__);
  }
  return 0;
}
void package_module::on_item_update_for_clt(player_obj *player,
                                            const int type,
                                            item_obj *item,
                                            const int arg[],
                                            const int size,
                                            const int behavior_sub_type)
{
  out_stream os(client::send_buf, client::send_buf_len);
  if (type == DEL_ITEM)
  {
    os << item->id_ << behavior_sub_type;
    player->send_request(NTF_DEL_ITEM, &os);
  }else if (type == ADD_ITEM)
  {
    os << item->pkg_ << behavior_sub_type;
    package_module::do_build_item_info(item, os);
    player->send_request(NTF_ADD_ITEM, &os);
  }else if (type == UPD_ITEM)
  {
    os << item->id_;
    for (int i = 0; i < size; ++i)
    {
      os << (char)(arg[i]);
      if (arg[i] == package_module::UPD_PKG)
        os << item->pkg_;
      else if (arg[i] == package_module::UPD_AMOUNT)
        os << item->amount_;
      else if (arg[i] == package_module::UPD_BIND)
        os << item->bind_;
      else if (arg[i] == package_module::UPD_EXTRA_INFO)
        os << stream_ostr(item->extra_info_->rd_ptr(), item->extra_info_->length());
      else if (arg[i] == package_module::UPD_CID)
        os << item->cid_;
    }
    player->send_request(NTF_UPD_ITEM, &os);
  }else
  {
    assert(0);
    return ;
  }
}
void package_module::on_add_item(player_obj *player,
                                 const int item_cid,
                                 const int amount,
                                 const int bind_type,
                                 const int behavior_sub_type,
                                 const int src_id_1,
                                 const int src_id_2)
{
  if (behavior_sub_type == 0) return ;
  behavior_log::instance()->store(BL_ITEM_GOT,
                                  time_util::now,
                                  "%d|%d|%d|%d|%d|%d|%d|%d",
                                  player->id(),
                                  item_cid,
                                  0,
                                  amount,
                                  bind_type,
                                  behavior_sub_type,
                                  src_id_1,
                                  src_id_2);
}
void package_module::on_remove_item(player_obj *player,
                                    const int item_cid,
                                    const int amount,
                                    const int bind_type,
                                    const int behavior_sub_type,
                                    const int src_id_1)
{
  if (behavior_sub_type == 0) return ;
  behavior_log::instance()->store(BL_ITEM_LOSE,
                                  time_util::now,
                                  "%d|%d|%d|%d|%d|%d|%d",
                                  player->id(),
                                  item_cid,
                                  0,
                                  amount,
                                  bind_type,
                                  behavior_sub_type,
                                  src_id_1);
}
void package_module::on_got_new_equip(item_obj *equip)
{
  if (!clsid::is_char_equip(equip->cid_)) return ;
  const equip_cfg_obj *eco = item_config::instance()->get_equip(equip->cid_);
  if (eco == NULL) return ;

  equip->attr_float_coe_ = 100;
  if (eco->attr_float_range_ > 0)
    equip->attr_float_coe_ = (100 - eco->attr_float_range_) + (rand() % eco->attr_float_range_ + 1);
#if 0 // 策划说先不自动洗炼
  iu_module::on_new_equip_for_xi_lian(equip);
#endif
}
void package_module::do_build_item_info(item_obj *io, out_stream &os)
{
  os << io->id_
    << io->cid_
    << io->amount_
    << io->bind_
    << io->attr_float_coe_;
  if (io->extra_info_ == NULL)
    os << stream_ostr("", 0);
  else
    os << stream_ostr(io->extra_info_->rd_ptr(), io->extra_info_->length());
}
item_obj *package_module::do_insert_item(player_obj *player,
                                         const char package,
                                         const int item_cid,
                                         const int amount,
                                         const char bind_type,
                                         const int behavior_sub_type,
                                         const int src_id_1,
                                         const int src_id_2)
{
  if (amount <= 0) return NULL;
  item_obj *item = package_module::alloc_new_item(player->id(),
                                                  item_cid,
                                                  amount,
                                                  bind_type);
  package_module::on_got_new_equip(item);
  if (package_module::do_insert_item(player,
                                     package,
                                     item,
                                     behavior_sub_type,
                                     src_id_1,
                                     src_id_2) == 0)
    return item;
  package_module::release_item(item);
  return NULL;
}
int package_module::do_insert_item(player_obj *player,
                                   const char package,
                                   item_obj *item,
                                   const int behavior_sub_type,
                                   const int src_id_1,
                                   const int src_id_2)
{
  if (item == NULL || item->amount_ <= 0) return -1;

  // 这里是没有检查背包空间的，所以一定会插入成功
  package_module::on_add_item(player,
                              item->cid_,
                              item->amount_,
                              item->bind_,
                              behavior_sub_type,
                              src_id_1,
                              src_id_2);

  // 先叠加
  package_module::do_item_dj(player, package, item, behavior_sub_type);

  // 叠加完后，当做一个新的道具插入背包，道具的个数允许超过叠加上限，自行拆分
  if (item->amount_ > 0)
  {
    package_module::do_insert_new_items(player, package, item, behavior_sub_type);
  }else  // 已经被合并了
    package_module::release_item(item);

  return 0;
}
item_obj *package_module::find_item(player_obj *player, const int item_id)
{
  item_map_iter itor = player->pack_info_->item_map_.find(item_id);
  if (itor == player->pack_info_->item_map_.end()) return NULL;
  return itor->second;
}
item_obj *package_module::find_item_by_part(player_obj *player, const int part)
{
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end(); ++itor)
  {
    if (itor->second->pkg_ != PKG_EQUIP) continue;
    const equip_cfg_obj *eco = item_config::instance()->get_equip(itor->second->cid_);
    if (eco == NULL) continue;
    if (eco->part_ == part) return itor->second;
  }
  return NULL;
}
void package_module::find_all_item_in_pkg(player_obj *player,
                                          const int pkg,
                                          ilist<item_obj*> &list)
{
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end(); ++itor)
  {
    if (itor->second->pkg_ == pkg)
      list.push_back(itor->second);
  }
}
int package_module::calc_item_amount_all(player_obj *player, const int item_cid)
{
  int cnt = 0;
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end(); ++itor)
  {
    if (itor->second->cid_ == item_cid)
      cnt += itor->second->amount_;
  }
  return cnt;
}
int package_module::calc_item_amount(player_obj *player, const int item_cid)
{
  int cnt = 0;
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end(); ++itor)
  {
    if (itor->second->pkg_ == PKG_PACKAGE
        && itor->second->cid_ == item_cid)
      cnt += itor->second->amount_;
  }
  return cnt;
}
int package_module::calc_item_amount(player_obj *player, const int item_cid, const char bind)
{
  int cnt = 0;
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end(); ++itor)
  {
    if (itor->second->pkg_ == PKG_PACKAGE
        && itor->second->cid_ == item_cid
        && itor->second->bind_ == bind)
      cnt += itor->second->amount_;
  }
  return cnt;
}
void package_module::get_equip_for_view(player_obj *player, int &zhu_wu, int &fu_wu)
{
  item_obj *equip = NULL;
  equip = package_module::find_item_by_part(player, PART_ZHU_WU);
  if (equip != NULL)
    zhu_wu = equip->cid_;
  equip = package_module::find_item_by_part(player, PART_FU_WU);
  if (equip != NULL)
    fu_wu = equip->cid_;
}
void package_module::do_move_item(player_obj *player,
                                  item_obj *item,
                                  const char to_pack)
{
  // 必须是主角已有的道具之间的操作
  int dj_limit = item_config::instance()->dj_upper_limit(item->cid_);
  if (dj_limit > 1)
  {
    item_obj *new_item = package_module::alloc_new_item();
    *new_item = *item;
    package_module::do_remove_item(player, item, item->amount_, 0, 0);
    package_module::do_insert_item(player,
                                   to_pack,
                                   new_item,
                                   0,
                                   0,
                                   0);
  }else if (dj_limit == 1)
  {
    item->pkg_ = to_pack;
    int item_update_size = 0;
    g_item_update_info[item_update_size++] = package_module::UPD_PKG;
    package_module::on_item_update(player,
                                   UPD_ITEM,
                                   item,
                                   g_item_update_info,
                                   item_update_size,
                                   0);
  }
}
int package_module::do_remove_item(player_obj *player,
                                   item_obj *item,
                                   const int remove_amount,
                                   const int behavior_sub_type,
                                   const int src_id_1)
{
  int removed_amount = 0;
  if (remove_amount >= item->amount_)
  {
    package_module::on_item_update(player,
                                   DEL_ITEM,
                                   item,
                                   NULL,
                                   0,
                                   behavior_sub_type);
    player->pack_info_->item_map_.erase(item->id_);
    package_module::on_remove_item(player,
                                   item->cid_,
                                   item->amount_,
                                   item->bind_,
                                   behavior_sub_type,
                                   src_id_1);
    removed_amount = item->amount_;
    package_module::release_item(item);
  }else if (remove_amount > 0)
  {
    item->amount_ -= remove_amount;
    int item_update_size = 0;
    g_item_update_info[item_update_size++] = package_module::UPD_AMOUNT;
    package_module::on_item_update(player,
                                   UPD_ITEM,
                                   item,
                                   g_item_update_info,
                                   item_update_size,
                                   behavior_sub_type);
    removed_amount = remove_amount;
  }
  return removed_amount;
}
int package_module::do_remove_item(player_obj *player,
                                   const int item_cid,
                                   const int remove_amount,
                                   const int behavior_sub_type,
                                   const int src_id_1)
{
  if (remove_amount <= 0) return 0;
  int removed_amount = 0;
  removed_amount = package_module::do_remove_item(player,
                                                  item_cid,
                                                  BIND_TYPE,
                                                  remove_amount - removed_amount,
                                                  behavior_sub_type,
                                                  src_id_1);
  if (removed_amount < remove_amount)
    removed_amount += package_module::do_remove_item(player,
                                                     item_cid,
                                                     UNBIND_TYPE,
                                                     remove_amount - removed_amount,
                                                     behavior_sub_type,
                                                     src_id_1);
  return removed_amount;
}
int package_module::do_remove_item(player_obj *player,
                                   const int item_cid,
                                   const char bind,
                                   const int remove_amount,
                                   const int behavior_sub_type,
                                   const int src_id_1)
{
  int removed_amount = 0;
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end()
       && removed_amount < remove_amount;
       ++itor)
  {
    if (itor->second->pkg_ == PKG_PACKAGE
        && itor->second->cid_ == item_cid
        && itor->second->bind_ == bind)
    {
      removed_amount += package_module::do_remove_item(player,
                                                       itor->second,
                                                       remove_amount - removed_amount,
                                                       behavior_sub_type,
                                                       src_id_1);
    }
  }
  return removed_amount;
}
void package_module::do_item_dj(player_obj *player,
                                const char package,
                                item_obj *item,
                                const int behavior_sub_type)
{
  int dj_limit = item_config::instance()->dj_upper_limit(item->cid_);
  item_map_iter itor = player->pack_info_->item_map_.begin();
  for (; itor != player->pack_info_->item_map_.end() && item->amount_ > 0; ++itor)
  {
    item_obj *cur_item = itor->second;
    if (cur_item->pkg_ == package
        && cur_item->cid_ == item->cid_
        && cur_item->bind_ == item->bind_
        && cur_item->amount_ < dj_limit)
    {
      int sum_amount = cur_item->amount_ + item->amount_;
      cur_item->amount_ = sum_amount > dj_limit ? dj_limit : sum_amount;
      item->amount_ = sum_amount - cur_item->amount_;

      int item_update_size = 0;
      g_item_update_info[item_update_size++] = package_module::UPD_AMOUNT;
      package_module::on_item_update(player,
                                     UPD_ITEM,
                                     cur_item,
                                     g_item_update_info,
                                     item_update_size,
                                     behavior_sub_type);
    }
  }
}
void package_module::do_insert_new_items(player_obj *player,
                                         const char package,
                                         item_obj *item,
                                         const int behavior_sub_type)
{
  int left_amount = item->amount_;
  int dj_limit = item_config::instance()->dj_upper_limit(item->cid_);
  while (true)
  {
    item->amount_  = left_amount > dj_limit ? dj_limit : left_amount;
    left_amount   -= item->amount_;
    item->pkg_     = package;
    item->id_      = player->pack_info_->assign_item_id();
    item->char_id_ = player->id();
    player->pack_info_->item_map_.insert(std::make_pair(item->id_, item));
    package_module::on_item_update(player, ADD_ITEM, item, NULL, 0, behavior_sub_type);

    if (left_amount > 0)
    {
      item_obj *io = package_module::alloc_new_item();
      *io = *item;
      item = io;
    }else
      break;
  }
}
void package_module::on_char_dead(player_obj *player, ilist<item_amount_bind_t> &drop_list)
{
  if (!scene_config::instance()->can_drop(player->scene_cid()))
    return ;

  if (package_module::calc_item_amount(player,
                                       global_param_cfg::mian_bao_fu) > 0)
  {
    package_module::do_remove_item(player,
                                   global_param_cfg::mian_bao_fu,
                                   1,
                                   ITEM_LOSE_USE,
                                   0);
    return ;
  }

  int to_drop_cnt = 0;
  int to_drop_coin_per = 0;
  for (ilist_node<sin_val_drop_param *> *itor = global_param_cfg::sin_val_drop_param_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (player->sin_val() >= itor->value_->sin_min_
        && player->sin_val() <= itor->value_->sin_max_)
    {
      int r = rand() % 100 + 1;
      if (r > itor->value_->drop_rate_) continue;

      to_drop_cnt = itor->value_->drop_cnt_max_;
      if (itor->value_->drop_cnt_max_ > itor->value_->drop_cnt_min_)
        to_drop_cnt = itor->value_->drop_cnt_min_ +\
                      rand() % (itor->value_->drop_cnt_max_ - itor->value_->drop_cnt_min_ + 1);
      to_drop_coin_per = itor->value_->drop_coin_max_;
      if (itor->value_->drop_coin_max_ > itor->value_->drop_coin_min_)
        to_drop_coin_per = itor->value_->drop_coin_min_ +\
                           rand() % (itor->value_->drop_coin_max_ - itor->value_->drop_coin_min_ + 1);
      break;
    }
  }

  dropped_item::do_clear_dropped_item_list();

  int to_drop_coin_amount = player->coin() * to_drop_coin_per / 100;
  player->do_lose_money(to_drop_coin_amount,
                        M_COIN,
                        MONEY_LOSE_DROP_TO_SCENE,
                        player->scene_cid(),
                        0,
                        0);
  drop_list.push_back(item_amount_bind_t(global_param_cfg::item_coin_cid,
                                         to_drop_coin_amount,
                                         UNBIND_TYPE));
  if (global_param_cfg::drop_coin_system_recovery > 0)
  {
    int p = 100 - global_param_cfg::drop_coin_system_recovery;
    to_drop_coin_amount = to_drop_coin_amount * p / 100;
  }

  if (to_drop_coin_amount > 0)
  {
    dropped_item *di = dropped_item::do_build_one_dropped_item(0,
                                                               player->id(),
                                                               0,
                                                               false,
                                                               UNBIND_TYPE,
                                                               to_drop_coin_amount,
                                                               global_param_cfg::item_coin_cid);
    dropped_item::dropped_item_list.push_back(di);
  }

  if (to_drop_cnt > 0)
  {
    ilist<item_obj *> can_dropped_list;
    for (item_map_iter itor = player->pack_info_->item_map_.begin();
         itor != player->pack_info_->item_map_.end();
         ++itor)
    {
      if (itor->second->pkg_ == PKG_PACKAGE
          && itor->second->bind_ == UNBIND_TYPE)
        can_dropped_list.push_back(itor->second);
    }
    if (!can_dropped_list.empty())
    {
      if (can_dropped_list.size() < to_drop_cnt)
        to_drop_cnt = can_dropped_list.size();
      int dropped_cnt = 0;
      while (dropped_cnt < to_drop_cnt
             && !can_dropped_list.empty())
      {
        int i = 0;
        int idx = rand() % can_dropped_list.size();
        for (ilist_node<item_obj *> *itor = can_dropped_list.head();
             itor != NULL;
             itor = itor->next_, ++i)
        {
          if (i == idx)
          {
            can_dropped_list.remove(itor->value_);
            dropped_item *di = dropped_item::do_build_one_dropped_item(0,
                                                                       player->id(),
                                                                       0,
                                                                       false,
                                                                       itor->value_->bind_,
                                                                       itor->value_->amount_,
                                                                       itor->value_->cid_);
            dropped_item::dropped_item_list.push_back(di);
            drop_list.push_back(item_amount_bind_t(itor->value_->cid_,
                                                   itor->value_->amount_,
                                                   itor->value_->bind_));
            ++dropped_cnt;
            package_module::do_remove_item(player,
                                           itor->value_,
                                           itor->value_->amount_,
                                           ITEM_LOSE_DROP_TO_SCENE,
                                           0);
            break;
          }
        }
      }
    }
  }
  player->do_drop_items_on_scene(dropped_item::dropped_item_list, DROP_AREA_RADIUS);
}
int package_module::do_insert_award_item_list(player_obj *player,
                                              ilist<item_amount_bind_t> *al,
                                              const blog_t &blog)
{
  int insert_item_num = 0;
  for (ilist_node<item_amount_bind_t> *itor = al->head();
       itor != NULL;
       itor = itor->next_)
  {
    item_amount_bind_t *io = &(itor->value_);
    if (item_config::item_is_money(io->cid_))
      continue;
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

  for (ilist_node<item_amount_bind_t> *itor = al->head();
       itor != NULL;
       itor = itor->next_)
  {
    item_amount_bind_t *item = &(itor->value_);
    int item_cid = item->cid_;
    if (item_config::item_is_money(item_cid))
    {
      player->do_got_item_money(item_cid,
                                item->amount_,
                                MONEY_GOT_USE_ITEM, // ????
                                item_cid);
    }else
    {
      if (clsid::is_char_equip(item_cid))
        item_cid = clsid::get_equip_cid_by_career(player->career(), item_cid);
      package_module::do_insert_item(player,
                                     PKG_PACKAGE,
                                     item_cid,
                                     item->amount_,
                                     item->bind_,
                                     blog.type_,
                                     blog.src1_,
                                     blog.src2_);
    }
  }
  return 0;
}
int package_module::do_insert_or_mail_item(player_obj *player,
                                           const int item_cid,
                                           const int amount,
                                           const char bind,
                                           const int mail_cfg_id,
                                           const int behavior_sub_type_money,
                                           const int behavior_sub_type,
                                           const int src_id_1,
                                           const int src_id_2)
{
  if (item_config::item_is_money(item_cid))
  {
    player->do_got_item_money(item_cid, amount, behavior_sub_type_money, 0);
    return 0;
  }
  int real_item_cid = item_cid;
  if (clsid::is_char_equip(real_item_cid)
      && behavior_sub_type != ITEM_GOT_PICKUP)
    real_item_cid = clsid::get_equip_cid_by_career(player->career(), real_item_cid);

  if (package_module::would_be_full(player,
                                    PKG_PACKAGE,
                                    real_item_cid,
                                    amount,
                                    bind))
  {
    const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_cfg_id);
    if (mo != NULL)
    {
      item_obj *io = package_module::alloc_new_item(player->id(),
                                                    real_item_cid,
                                                    amount,
                                                    bind);
      item_obj il[MAX_ATTACH_NUM];
      il[0] = *io;
      package_module::release_item(io);
      mail_module::do_send_mail(player->id(), mail_info::MAIL_SEND_SYSTEM_ID,
                                mo->sender_name_, mo->title_, mo->content_,
                                mail_info::MAIL_TYPE_PACKAGE,
                                0, 0, 0,
                                1, il,
                                player->db_sid(), time_util::now);
      mail_module::do_notify_haved_new_mail(player, 1);
    }else
      e_log->rinfo("not found mail_cfg_id %d in [%s]", mail_cfg_id, __func__);
  }else
  {
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   real_item_cid,
                                   amount,
                                   bind,
                                   behavior_sub_type,
                                   src_id_1,
                                   src_id_2);
  }
  return 0;
}
