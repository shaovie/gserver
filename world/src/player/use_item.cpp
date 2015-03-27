#include "player_obj.h"
#include "package_module.h"
#include "global_param_cfg.h"
#include "gift_config.h"
#include "behavior_id.h"
#include "scene_config.h"
#include "item_effect_def.h"
#include "daily_clean_info.h"
#include "spawn_monster.h"
#include "vip_module.h"
#include "sys_log.h"
#include "clsid.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

static int open_gift_all(player_obj *player, item_obj *item);
static int open_random_gift(player_obj *player,
                            const int open_item_cid,
                            const short use_amount,
                            ilist<item_amount_bind_t> *rlist);
static int open_activity_box(player_obj *player,
                             const int box_cid,
                             const int key_cid,
                             const short use_amount,
                             const int retain_cid);

int player_obj::clt_use_item(const char *msg, const int len)
{
  int item_id = 0;
  int use_amount = 0;
  in_stream is(msg, len);
  is >> item_id >> use_amount;

  item_obj *item = package_module::find_item(this, item_id);
  if (item == NULL)
    return this->send_respond_err(RES_USE_ITEM, ERR_ITEM_NOT_EXIST);
  item_cfg_obj* ico = item_config::instance()->get_item(item->cid_);
  if (ico == NULL)
    return this->send_respond_err(RES_USE_ITEM, ERR_NOT_FOUND_IN_CONFIG);

  int ret = this->can_use_item(item, ico, use_amount);
  if (ret != 0)
    return this->send_respond_err(RES_USE_ITEM, ret);

  ret = this->do_use_item(item, ico, use_amount);
  if (ret < 0)
    return this->send_respond_err(RES_USE_ITEM, ret);

  int used_item_cid = item->cid_;
  this->on_item_used_ok(item, ico, ret); // end of `item' life

  out_stream os(client::send_buf, client::send_buf_len);
  os << used_item_cid;
  return this->send_respond_ok(RES_USE_ITEM, &os);
}
int player_obj::can_use_item(item_obj *item, item_cfg_obj *ico, const int use_amount)
{
  if (use_amount < 1)
    return ERR_CLIENT_OPERATE_ILLEGAL;
  if (item->amount_ < use_amount)
    return ERR_ITEM_AMOUNT_NOT_ENOUGH;

  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    return ERR_CAN_NOT_USE_ITEM;

  if (this->lvl() < ico->use_lvl_)
    return ERR_LVL_NOT_ENOUGH;
  if (!ico->can_use_)
    return ERR_CAN_NOT_USE_ITEM;
  if (ico->cd_ > 0)
  {
    item_used_time_map_itor itor = this->item_used_time_map_.find(ico->effect_);
    if (itor != this->item_used_time_map_.end())
    {
      if ((time_value::gettimeofday() - itor->second).msec() < (long)ico->cd_)
        return ERR_ITEM_CD_LIMIT;
    }
  }
  if (!scene_config::instance()->can_use_item(this->scene_cid_, item->cid_))
    return ERR_CAN_NOT_USE_ITEM;
  return 0;
}
void player_obj::on_item_used_ok(item_obj *item,
                                 item_cfg_obj *ico,
                                 const int used_amount)
{
  this->item_used_time_map_[ico->effect_] = time_value::gettimeofday();
  package_module::do_remove_item(this,
                                 item,
                                 used_amount,
                                 ITEM_LOSE_USE,
                                 0);
}
int player_obj::do_use_item(item_obj *item, const item_cfg_obj *ico, const int use_amount)
{
  if (ico->effect_ == item_effect::CSM_EFFECT_RECOVER_HP)
  {
    if (this->hp() == this->total_hp())
      return ERR_HP_IS_FULL;
    if (this->do_add_hp(ico->value_) != 0)
      this->broadcast_hp_mp();
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_RECOVER_MP)
  {
    if (this->mp() == this->total_mp())
      return ERR_MP_IS_FULL;
    if (this->do_add_mp(ico->value_) != 0)
      this->broadcast_hp_mp();
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_RECOVER_HP_PER)
  {
    if (this->hp() == this->total_hp())
      return ERR_HP_IS_FULL;
    if (this->do_add_hp(this->total_hp() * ico->value_ / 100) != 0)
      this->broadcast_hp_mp();
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_RECOVER_MP_PER)
  {
    if (this->mp() == this->total_mp())
      return ERR_MP_IS_FULL;
    if (this->do_add_mp(this->total_mp() * ico->value_ / 100) != 0)
      this->broadcast_hp_mp();
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_GET_COIN)
  {
    if (ico->value_ <= 0) return ERR_CONFIG_NOT_EXIST;
    int can_use_amt = use_amount;
    int64_t v = ico->value_ * use_amount;
    if (v > MONEY_UPPER_LIMIT)
      can_use_amt = (MONEY_UPPER_LIMIT - this->coin()) / ico->value_;
    if (can_use_amt <= 0) return ERR_MONEY_UPPER_LIMIT;
    this->do_got_money(ico->value_ * can_use_amt, M_COIN, MONEY_GOT_USE_ITEM, item->cid_);
    return can_use_amt;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_GIFT_ALL)
  {
    int ret = open_gift_all(this, item);
    if (ret == 0) return 1;
    return ret;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_GIFT_ONE)
  {
    int ret = open_random_gift(this, item->cid_, use_amount, NULL);
    if (ret == 0) return use_amount;
    return ret;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_VIP_EXP)
  {
    if (vip_module::vip_lvl(this) == 0) return ERR_NOT_VIP;
    vip_module::do_add_vip_exp(this, use_amount * ico->value_);
    return use_amount;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_SIN_DECREASE)
  {
    if (this->sin_val() <= 0)
      return ERR_SIN_VAL_IS_EMPTY;
    this->do_decrease_sin(ico->value_);
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_ZHANXING_FU)
  {
    this->do_add_zhan_xing_value(ico->value_ * use_amount);
    return use_amount;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_BACK_TOWN)
  {
    if (clsid::is_tui_tu_scp_scene(this->scene_cid_)
        || clsid::is_scp_scene(this->scene_cid_))
      return ERR_CAN_NOT_USE_ITEM;

    if (!scene_config::instance()->can_move(global_param_cfg::capital_relive_coord.cid_,
                                            global_param_cfg::capital_relive_coord.coord_.x_,
                                            global_param_cfg::capital_relive_coord.coord_.y_))
      return ERR_COORD_IS_ILLEGAL;
    int ret = this->can_transfer_to(global_param_cfg::capital_relive_coord.cid_);
    if (ret != 0) return ret;
    this->do_transfer_to(global_param_cfg::capital_relive_coord.cid_,
                         global_param_cfg::capital_relive_coord.cid_,
                         global_param_cfg::capital_relive_coord.coord_.x_,
                         global_param_cfg::capital_relive_coord.coord_.y_);
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_RANDOM_TRANSFER)
  {
    if (clsid::is_tui_tu_scp_scene(this->scene_cid_)
        || clsid::is_scp_scene(this->scene_cid_))
      return ERR_CAN_NOT_USE_ITEM;

    coord_t pos = scene_config::instance()->get_scene_random_pos(this->scene_cid_,
                                                                 this->coord_x_,
                                                                 this->coord_y_);
    int ret = this->can_transfer_to(this->scene_cid_);
    if (ret != 0) return ret;
    this->do_transfer_to(this->scene_cid_,
                         this->scene_id_,
                         pos.x_,
                         pos.y_);
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_BOSS)
  {
    if (this->scene_cid_ != global_param_cfg::guild_zhu_di_cid)
      return ERR_CALL_BOSS_IN_ZHU_DI;

    coord_t pos = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                           this->coord_x_,
                                                           this->coord_y_,
                                                           4);
    spawn_monster::spawn_one(ico->value_,
                             800,
                             this->scene_id_,
                             this->scene_cid_,
                             DIR_XX,
                             pos.x_,
                             pos.y_);
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_RECOVER_TL)
  {
    this->do_add_ti_li(ico->value_, true, true);
    return 1;
  }else if (ico->effect_ == item_effect::CSM_EFFECT_ACTIVITY_BOX)
  {
    return open_activity_box(this, item->cid_, ico->value_, use_amount, item->cid_);
  }else if (ico->effect_ == item_effect::CSM_EFFECT_ACTIVITY_KEY)
  {
    return open_activity_box(this, ico->value_, item->cid_, use_amount, item->cid_);
  }else
    e_log->rinfo("unknown item %d effect %d", item->cid_, ico->effect_);
  return ERR_CAN_NOT_USE_ITEM;
}
int open_gift_all(player_obj *player, item_obj *item)
{
  gift_obj *go = gift_config::instance()->get_gift(item->cid_);
  if (go == NULL
      || go->gift_list_.empty())
    return ERR_CONFIG_NOT_EXIST;

  int insert_item_num = 0;
  for (ilist_node<gift_info *> *itor = go->gift_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    gift_info *gi = itor->value_;
    if (item_config::item_is_money(gi->item_cid_))
      continue;
    int real_amt = gi->max_amt_;
    if (gi->min_amt_ != gi->max_amt_)
      real_amt = gi->min_amt_ + (rand() % (gi->max_amt_ - gi->min_amt_ + 1));
    g_item_amount_bind[0][insert_item_num] = gi->item_cid_;
    g_item_amount_bind[1][insert_item_num] = real_amt;
    g_item_amount_bind[2][insert_item_num] = gi->bind_;
    insert_item_num++;
  }

  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE)
      < insert_item_num)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  for (ilist_node<gift_info *> *itor = go->gift_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    gift_info *gi = itor->value_;
    int item_cid = gi->item_cid_;
    int real_amt = gi->max_amt_;
    if (gi->min_amt_ != gi->max_amt_)
      real_amt = gi->min_amt_ + (rand() % (gi->max_amt_ - gi->min_amt_ + 1));
    if (item_config::item_is_money(item_cid))
    {
      player->do_got_item_money(item_cid,
                                real_amt,
                                MONEY_GOT_USE_ITEM,
                                0);
    }else
    {
      if (clsid::is_char_equip(item_cid))
        item_cid = clsid::get_equip_cid_by_career(player->career(), item_cid);

      package_module::do_insert_item(player,
                                     PKG_PACKAGE,
                                     item_cid,
                                     real_amt,
                                     gi->bind_,
                                     ITEM_GOT_GIFT,
                                     item->cid_,
                                     0);
    }
  }
  return 0;
}
int open_random_gift_obj(player_obj *player,
                         gift_obj *go,
                         const int open_item_cid,
                         const short use_amount,
                         ilist<item_amount_bind_t> *rlist)
{
  if (go == NULL
      || go->gift_list_.empty())
    return ERR_CONFIG_NOT_EXIST;

  int insert_item_num = 0;
  for (int i = 0; i < use_amount; ++i)
  {
    const int gift_rand_rate = rand() % 10000 + 1;
    bool first = true;
    int gift_rand_rate_1 = 0;
    int gift_rand_rate_2 = 0;
    gift_info *gi = NULL;
    for (ilist_node<gift_info *> *itor = go->gift_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      gi = itor->value_;
      util::cake_rate(gi->p_, first, gift_rand_rate_1, gift_rand_rate_2);
      if (gift_rand_rate >= gift_rand_rate_1
          && gift_rand_rate <= gift_rand_rate_2)
        break;
    }
    if (gi == NULL)
      return ERR_CONFIG_NOT_EXIST;

    int real_amt = gi->max_amt_;
    if (gi->min_amt_ != gi->max_amt_)
      real_amt = gi->min_amt_ + (rand() % (gi->max_amt_ - gi->min_amt_ + 1));

    if (rlist != NULL)
      rlist->push_back(item_amount_bind_t(gi->item_cid_, real_amt, gi->bind_));
    // stack
    int j = 0;
    for (; j < insert_item_num; ++j)
    {
      if (g_item_amount_bind[0][j] == gi->item_cid_
          && ((g_item_amount_bind[1][j] + real_amt)
              < item_config::instance()->dj_upper_limit(gi->item_cid_))
          && g_item_amount_bind[2][j] == gi->bind_)
      {
        g_item_amount_bind[1][j] += real_amt;
        break;
      }
    }
    if (j == insert_item_num)
    {
      g_item_amount_bind[0][insert_item_num] = gi->item_cid_;
      g_item_amount_bind[1][insert_item_num] = real_amt;
      g_item_amount_bind[2][insert_item_num] = gi->bind_;
      insert_item_num++;
    }
  }

  // if call would_be_full then player can select gift type by package space
  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE)
      < insert_item_num)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  for (int i = 0; i < insert_item_num; ++i)
  {
    const int item_cid  = g_item_amount_bind[0][i];
    const int item_amt  = g_item_amount_bind[1][i];
    const int item_bind = g_item_amount_bind[2][i];
    if (item_config::item_is_money(item_cid))
    {
      player->do_got_item_money(item_cid,
                                item_amt,
                                MONEY_GOT_USE_ITEM,
                                open_item_cid);
    }else
    {
      int real_cid = item_cid;
      if (clsid::is_char_equip(item_cid))
        real_cid = clsid::get_equip_cid_by_career(player->career(), item_cid);

      package_module::do_insert_item(player,
                                     PKG_PACKAGE,
                                     real_cid,
                                     item_amt,
                                     item_bind,
                                     ITEM_GOT_GIFT,
                                     open_item_cid,
                                     0);
    }
  }
  return 0;
}
int open_random_gift(player_obj *player,
                     const int open_item_cid,
                     const short use_amount,
                     ilist<item_amount_bind_t> *rlist)
{
  gift_obj *go = gift_config::instance()->get_gift(open_item_cid);
  return open_random_gift_obj(player, go, open_item_cid, use_amount, rlist);
}
int open_activity_box(player_obj *player,
                      const int box_cid,
                      const int key_cid,
                      const short use_amount,
                      const int retain_cid)
{
  int box_count = package_module::calc_item_amount(player, box_cid);
  if (box_count < use_amount)
    return ERR_ITEM_AMOUNT_NOT_ENOUGH;

  int key_count = package_module::calc_item_amount(player, key_cid);
  if (key_count < use_amount)
    return ERR_ITEM_AMOUNT_NOT_ENOUGH;

  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE) < use_amount)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  gift_obj *go = gift_byvip_config::instance()->get_gift(box_cid, vip_module::vip_lvl(player));
  ilist<item_amount_bind_t> new_item_list;
  int ret = open_random_gift_obj(player, go, box_cid, use_amount, &new_item_list);
  if (ret != 0) return ret;

  if (box_cid != retain_cid)
    package_module::do_remove_item(player,
                                   box_cid,
                                   use_amount,
                                   ITEM_LOSE_USE,
                                   0);
  if (key_cid != retain_cid)
    package_module::do_remove_item(player,
                                   key_cid,
                                   use_amount,
                                   ITEM_LOSE_USE,
                                   0);

  out_stream os(client::send_buf, client::send_buf_len);
  os << short(new_item_list.size());
  while (!new_item_list.empty())
  {
    item_amount_bind_t v = new_item_list.pop_front();
    os << v.cid_ << v.amount_ << v.bind_;
  }
  player->send_respond_ok(NTF_OPEN_ACTIVITY_BOX, &os);
  return use_amount;
}
