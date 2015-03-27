#include "goods_award_act.h"
#include "global_param_cfg.h"
#include "player_obj.h"
#include "lucky_goods_draw_cfg.h"
#include "activity_mgr.h"
#include "activity_cfg.h"
#include "package_module.h"
#include "player_mgr.h"
#include "behavior_id.h"

// Lib header

goods_award_act::goods_award_act() :
  activity_obj(ACTIVITY_GOODS_AWARD)
{ }
void goods_award_act::on_enter_game(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GOODS_AWARD);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() < activity_cfg::instance()->open_lvl(ACTIVITY_GOODS_AWARD)
      || player->daily_goods_lucky_draw_cnt() >= global_param_cfg::daily_luck_draw_award_cnt
      || player->if_got_goods_lucky_draw() != 0)
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
void goods_award_act::on_char_lvl_up(player_obj *player)
{
  activity_obj *ao = activity_mgr::instance()->find(ACTIVITY_GOODS_AWARD);
  if (ao == NULL
      || !ao->is_opened()
      || player->lvl() != activity_cfg::instance()->open_lvl(ACTIVITY_GOODS_AWARD))
    return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ao->act_id() << ao->left_time();
  player->do_delivery(NTF_ACTIVITY_OPENED, &mb);
}
int goods_award_act::do_lucky_goods_draw(player_obj *player)
{
  if (player->daily_goods_lucky_draw_cnt() >= global_param_cfg::daily_luck_draw_award_cnt
      || player->if_got_goods_lucky_draw() != 0)
    return ERR_CLIENT_OPERATE_ILLEGAL;
  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE)
      < 1)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  ilist<lucky_goods_draw_cfg_obj *> *cfg =
    lucky_goods_draw_cfg::instance()->get_cfg_obj(player->daily_goods_lucky_draw_cnt() + 1);
  if (cfg == NULL) return ERR_CONFIG_NOT_EXIST;

  int item_rand_rate = rand() % 10000 + 1;
  int item_draw_rate_1 = 0;
  int item_draw_rate_2 = 0;
  bool first_item = true;
  item_amount_bind_t award_item;
  for (ilist_node<lucky_goods_draw_cfg_obj *> *item_l = cfg->head();
       item_l != NULL;
       item_l = item_l->next_)
  {
    lucky_goods_draw_cfg_obj *draw_i = item_l->value_;

    util::cake_rate(draw_i->rate_,
                    first_item,
                    item_draw_rate_1,
                    item_draw_rate_2);
    if (item_rand_rate < item_draw_rate_1
        || item_rand_rate > item_draw_rate_2)
      continue;

    award_item.cid_ = draw_i->item_cid_;
    award_item.bind_ = draw_i->bind_type_;
    if (draw_i->min_cnt_ == draw_i->max_cnt_)
      award_item.amount_ = draw_i->max_cnt_;
    else
      award_item.amount_ = draw_i->min_cnt_ + \
                           (rand() % (draw_i->max_cnt_ - draw_i->min_cnt_ + 1));

    break;
  }
  if (award_item.cid_ == 0) return ERR_CONFIG_NOT_EXIST;

  ilist<item_amount_bind_t> award_list;
  award_list.push_back(award_item);
  package_module::do_insert_award_item_list(player,
                                            &award_list,
                                            blog_t(ITEM_GOT_GOODS_LUCKY_DRAW, 0, 0));

  player->daily_goods_lucky_draw_cnt(player->daily_goods_lucky_draw_cnt() + 1);
  player->if_got_goods_lucky_draw(1);
  player->db_save_daily_clean_info();

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->daily_goods_lucky_draw_cnt()
    << award_item.cid_;
  player->send_respond_ok(RES_DO_DAILY_LUCKY_GOODS_DRAW, &os);

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));
  mb << ACTIVITY_GOODS_AWARD;
  player_mgr::instance()->broadcast_to_world(NTF_ACTIVITY_CLOSED, &mb);
  return 0;
}
