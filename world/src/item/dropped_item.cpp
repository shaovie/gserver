#include "dropped_item.h"
#include "sys_log.h"
#include "monster_drop_cfg.h"
#include "dropped_item_mgr.h"
#include "package_module.h"
#include "notice_module.h"
#include "mail_config.h"
#include "item_config.h"
#include "behavior_id.h"
#include "player_obj.h"
#include "scene_mgr.h"
#include "time_util.h"
#include "item_obj.h"
#include "team_mgr.h"
#include "message.h"
#include "client.h"
#include "istream.h"
#include "clsid.h"
#include "util.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("drop");
static ilog_obj *e_log = err_log::instance()->get_ilog("drop");

ilist<dropped_item *> dropped_item::dropped_item_list;

void dropped_item::reset_pos(const int scene_id,
                             const int scene_cid,
                             const short x,
                             const short y)
{
  this->scene_id_ = scene_id;
  this->scene_cid_ = scene_cid;
  this->coord_x_ = x;
  this->coord_y_ = y;
}
int dropped_item::do_enter_scene()
{
  if (scene_unit::do_enter_scene() != 0) return -1;
  return 0;
}
int dropped_item::do_exit_scene()
{
  scene_unit::do_exit_scene();
  return 0;
}
void dropped_item::do_clear_dropped_item_list()
{
  while (!dropped_item::dropped_item_list.empty())
  {
    dropped_item *p = dropped_item::dropped_item_list.pop_front();
    dropped_item_pool::instance()->release(p);
  }
}
bool dropped_item::do_build_snap_info(const int ,
                                      out_stream &os,
                                      ilist<pair_t<int> > *old_snap_unit_list,
                                      ilist<pair_t<int> > *new_snap_unit_list)
{
  new_snap_unit_list->push_back(pair_t<int>(this->id_, this->cid_));

  if (this->is_new_one(old_snap_unit_list))
  {
    os << T_CID_ID << this->cid_ << this->id_;
    this->do_build_snap_base_info(os); 
    os << T_END;
    return true;
  }
  return false;
}
void dropped_item::do_build_snap_base_info(out_stream &os)
{ os << T_X_Y_INFO << this->coord_x_ << this->coord_y_ << this->src_id_ << this->owner_id_; }
void dropped_item::do_build_drop_item_list(const int owner_id,
                                           const int mst_id,
                                           const int mst_cid,
                                           ilist<dropped_item *> &dropped_item_list)
{
  const monster_drop_obj *mdo = 
    monster_drop_cfg::instance()->get_monster_drop_obj(mst_cid);
  if (mdo == NULL) return ;

  for (ilist_node<monster_drop_obj::_drop_box *> *box_l = mdo->drop_box_list_.head();
       box_l != NULL;
       box_l = box_l->next_)
  {
    monster_drop_obj::_drop_box *drop_b = box_l->value_;

    int box_rand_rate = rand() % 100 + 1;
    if (box_rand_rate > drop_b->box_rate_)
      continue;

    if (!drop_b->limit_time_list_.empty())
    {
      bool in_time = false;
      for (ilist_node<pair_t<int> > *time_l = drop_b->limit_time_list_.head();
           time_l != NULL && !in_time;
           time_l = time_l->next_)
      {
        if (time_l->value_.first_ < time_util::now
            && time_l->value_.second_ > time_util::now)
          in_time = true;
      }
      if (!in_time) continue;
    }

    dropped_item::do_drop_item_from_box(owner_id,
                                        mst_id,
                                        mst_cid,
                                        drop_b->box_cid_,
                                        dropped_item_list);
  }// end of box loop
}
void dropped_item::do_drop_item_from_box(const int owner_id,
                                         const int src_id,
                                         const int src_cid,
                                         const int box_cid,
                                         ilist<dropped_item *> &dropped_item_list)
{
  const box_item_obj *bio = box_item_cfg::instance()->get_box_item_obj(box_cid);
  if (bio == NULL) return ;

  int item_rand_rate = rand() % 10000 + 1;
  int item_drop_rate_1 = 0;
  int item_drop_rate_2 = 0;
  bool first_item = true;
  for (ilist_node<box_item_obj::_drop_item *> *item_l = bio->drop_item_list_.head();
       item_l != NULL;
       item_l = item_l->next_)
  {
    box_item_obj::_drop_item *drop_i = item_l->value_;

    if (bio->box_type_ == RANDOM_CALC_RATE)
    {
      item_rand_rate = rand() % 10000 + 1;
      if (item_rand_rate > drop_i->drop_rate_)
        continue;
    }else
    {
      util::cake_rate(drop_i->drop_rate_,
                      first_item,
                      item_drop_rate_1,
                      item_drop_rate_2);
      if (item_rand_rate < item_drop_rate_1
          || item_rand_rate > item_drop_rate_2)
        continue;
    }

    dropped_item *di = dropped_item::do_build_one_dropped_item(owner_id,
                                                               src_id,
                                                               src_cid,
                                                               drop_i->pick_notice_,
                                                               drop_i->bind_type_,
                                                               0,
                                                               drop_i->item_cid_);
    if (drop_i->min_cnt_ == drop_i->max_cnt_)
      di->amount_ = drop_i->max_cnt_;
    else
      di->amount_ = drop_i->min_cnt_ + \
                    (rand() % (drop_i->max_cnt_ - drop_i->min_cnt_ + 1));

    dropped_item_list.push_back(di);
  } // end of item loop
}
dropped_item *dropped_item::do_build_one_dropped_item(const int owner_id,
                                                      const int src_id,
                                                      const int src_cid,
                                                      const bool notice,
                                                      const char bind,
                                                      const int amount,
                                                      const int item_cid)
{
  dropped_item *di = dropped_item_pool::instance()->alloc();
  di->id_ = dropped_item_mgr::instance()->assign_dropped_item_id();
  di->owner_id_  = owner_id;
  di->src_id_    = src_id;
  di->src_cid_   = src_cid;
  di->notice_    = notice;
  di->bind_type_ = bind;
  di->amount_    = amount;
  di->dropped_time_ = time_util::now;
  di->cid_       = item_cid;
  return di;
}
int player_obj::clt_pick_up_item(const char *msg, const int len)
{
  int dropped_item_id = 0;
  in_stream is(msg, len);
  is >> dropped_item_id;

  dropped_item *di = dropped_item_mgr::instance()->find(dropped_item_id);
  if (di == NULL)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_DROPPED_ITEM_NOT_EXIST);

  if (di->owner_id_ != 0
      && di->owner_id_ != this->id_)
  {
    team_info *ti = team_mgr::instance()->find_by_char(di->owner_id_);
    if (ti == NULL
        || !ti->have_player(this->id_))
      return this->send_respond_err(NTF_OPERATE_RESULT, ERR_DROPPED_ITEM_NOT_YOURS);
  }

  // ok
  item_obj notice_item;
  notice_item.cid_ = di->cid();
  notice_item.amount_ = di->amount_;
  notice_item.bind_ = di->bind_type_;
  if (clsid::is_tui_tu_scp_scene(this->scene_cid_))
  {
    package_module::do_insert_or_mail_item(this,
                                           di->cid(),
                                           di->amount_,
                                           di->bind_type_,
                                           mail_config::TUI_TU_MST_DROP,
                                           MONEY_GOT_PICK_UP,
                                           ITEM_GOT_PICKUP,
                                           this->scene_cid_,
                                           0);
  }else
  {
    if (item_config::item_is_money(di->cid()))
    {
      this->do_got_item_money(di->cid(),
                              di->amount_,
                              MONEY_GOT_PICK_UP,
                              0);
    }else
    {
      if (package_module::would_be_full(this,
                                        PKG_PACKAGE,
                                        di->cid(),
                                        di->amount_,
                                        di->bind_type_))
        return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);
      item_obj *new_item = package_module::do_insert_item(this,
                                                          PKG_PACKAGE,
                                                          di->cid(),
                                                          di->amount_,
                                                          di->bind_type_,
                                                          ITEM_GOT_PICKUP,
                                                          this->scene_cid_,
                                                          0);
      if (item_config::instance()->dj_upper_limit(di->cid()) == 1)
        notice_item = *new_item;
    }
    if (di->notice_ && di->src_cid_ != 0)
    {
      notice_module::pick_up_dropped_item(this->id(),
                                          this->name(),
                                          this->scene_cid_,
                                          di->src_cid_,
                                          &notice_item);
    }
  }
  di->do_exit_scene();
  dropped_item_mgr::instance()->remove(di->id());
  dropped_item_pool::instance()->release(di);

  out_stream os(client::send_buf, client::send_buf_len);
  os << dropped_item_id;
  return this->send_respond_ok(RES_PICK_UP_ITEM, &os);
}
