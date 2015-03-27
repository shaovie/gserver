#include "goods_award_module.h"
#include "water_tree_cfg.h"
#include "behavior_id.h"
#include "package_module.h"
#include "global_param_cfg.h"
#include "player_obj.h"
#include "notice_module.h"
#include "lucky_goods_turn_cfg.h"
#include "sys_log.h"
#include "ilist.h"

// Lib header

static char s_max_wt_goods_cnt = 0;
static char s_max_lucky_turn_goods_cnt = 0;

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

void goods_award_module::on_preload_ok(const int max_wt_goods_cnt,
                                       const int max_lucky_turn_goods_cnt)
{
  s_max_wt_goods_cnt = max_wt_goods_cnt;
  s_max_lucky_turn_goods_cnt = max_lucky_turn_goods_cnt;
}
void goods_award_module::on_guan_gai(player_obj *player, water_tree_obj *wto)
{
  ilist_node<water_tree_random_award *> *itor = wto->random_award_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    int r = rand() % 10000 + 1;
    if (r < itor->value_->rate_
        && !package_module::would_be_full(player,
                                          PKG_PACKAGE,
                                          itor->value_->award_.cid_,
                                          itor->value_->award_.amount_,
                                          itor->value_->award_.bind_))
    {
      if (itor->value_->award_.cid_ == global_param_cfg::water_tree_award_good.first_)
      {
        if (s_max_wt_goods_cnt >= global_param_cfg::water_tree_award_good.second_
            && player->wt_goods_cnt() >= (global_param_cfg::water_tree_award_good.second_ - 1))
          return ;
      }

      package_module::do_insert_item(player,
                                     PKG_PACKAGE,
                                     itor->value_->award_.cid_,
                                     itor->value_->award_.amount_,
                                     itor->value_->award_.bind_,
                                     ITEM_GOT_WATER_TREE_RANDOM_AWARD,
                                     0,
                                     0);
      if (itor->value_->award_.cid_ == global_param_cfg::water_tree_award_good.first_
          && s_max_wt_goods_cnt < global_param_cfg::water_tree_award_good.second_)
      {
        player->wt_goods_cnt(player->wt_goods_cnt() + itor->value_->award_.amount_);
        if (player->wt_goods_cnt() > s_max_wt_goods_cnt)
          s_max_wt_goods_cnt = player->wt_goods_cnt();
        player->db_save_char_extra_info();

        // notice
        item_obj io;
        io.cid_ = itor->value_->award_.cid_;
        io.bind_ = itor->value_->award_.bind_;
        io.amount_ = itor->value_->award_.amount_;
        notice_module::water_tree_goods_award(player->id(),
                                              player->name(),
                                              &io);
      }
    }
  }

  return ;
}
int player_obj::clt_goods_award_wei_xin_shared_ok(const char *, const int )
{
  if (this->daily_clean_info_->daily_wx_shared_ != 0) return 0;

  this->daily_clean_info_->daily_wx_shared_ = 1;
  this->db_save_daily_clean_info();
  this->do_notify_goods_lucky_turn_info_to_clt();
  return 0;
}
int player_obj::clt_lucky_goods_turn(const char *, const int )
{
  if (this->daily_clean_info_->goods_lucky_turn_cnt_ >= global_param_cfg::lucky_goods_turn_cnt)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  if (package_module::package_capacity(this, PKG_PACKAGE)
      - package_module::used_space(this, PKG_PACKAGE)
      < 1)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_PACKAGE_SPACE_NOT_ENOUGH);

  item_amount_bind_t award_item;
  bool notify = false;

  ilist<lucky_goods_turn_cfg_obj *> *cfg = lucky_goods_turn_cfg::instance()->get_cfg_list();
  int max_loop_cnt = 16;
  while (--max_loop_cnt > 0)
  {
    int rand_rate = rand() % 10000 + 1;
    int rate_1 = 0;
    int rate_2 = 0;
    bool first_item = true;
    for (ilist_node<lucky_goods_turn_cfg_obj *> *itor = cfg->head();
         itor != NULL;
         itor = itor->next_)
    {
      util::cake_rate(itor->value_->rate_,
                      first_item,
                      rate_1,
                      rate_2);
      if (rand_rate < rate_1
          || rand_rate > rate_2)
        continue;

      notify = itor->value_->notify_;
      award_item = itor->value_->award_item_;
      break;
    }
    if (award_item.cid_ == global_param_cfg::lucky_turn_award_good.first_)
    {
      if (s_max_lucky_turn_goods_cnt >= (char)global_param_cfg::lucky_turn_award_good.second_
          && this->lucky_turn_goods_cnt() >= (global_param_cfg::lucky_turn_award_good.second_ - 1))
      {
        award_item.cid_ = 0;
        continue ;
      }
    }
    break;
  }

  if (award_item.cid_ == 0)
    return this->send_respond_err(NTF_OPERATE_RESULT, ERR_CONFIG_NOT_EXIST);

  item_obj *new_item = package_module::alloc_new_item(this->id(),
                                                      award_item.cid_,
                                                      award_item.amount_,
                                                      award_item.bind_);
  if (notify)
    notice_module::lucky_goods_turn(this->id(), this->name(), new_item);
  package_module::release_item(new_item);

  ilist<item_amount_bind_t> award_list;
  award_list.push_back(award_item);
  package_module::do_insert_award_item_list(this,
                                            &award_list,
                                            blog_t(ITEM_GOT_GOODS_LUCKY_TURN, 0, 0));
  if (award_item.cid_ == global_param_cfg::lucky_turn_award_good.first_)
  {
    this->lucky_turn_goods_cnt(this->lucky_turn_goods_cnt() + award_item.amount_);
    if (this->lucky_turn_goods_cnt() > s_max_lucky_turn_goods_cnt)
      s_max_lucky_turn_goods_cnt = this->lucky_turn_goods_cnt();
    this->db_save_char_extra_info();
  }

  this->daily_clean_info_->goods_lucky_turn_cnt_ += 1;
  this->db_save_daily_clean_info();
  this->do_notify_goods_lucky_turn_info_to_clt();

  out_stream os(client::send_buf, client::send_buf_len);
  os << award_item.cid_ << award_item.amount_ << award_item.bind_;
  return this->send_respond_ok(RES_GOODS_AWARD_LUCKY_TURN, &os);
}
void player_obj::do_notify_goods_lucky_turn_info_to_clt()
{
  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << this->daily_clean_info_->daily_wx_shared_
    << this->daily_clean_info_->goods_lucky_turn_cnt_;
  this->do_delivery(NTF_GOODS_AWARD_WEI_XIN_SHARED_STATE, &mb);
}
