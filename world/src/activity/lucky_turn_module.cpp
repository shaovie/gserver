#include "lucky_turn_module.h"
#include "lucky_turn_big_award.h"
#include "all_char_info.h"
#include "lucky_turn_score.h"
#include "package_module.h"
#include "notice_module.h"
#include "global_param_cfg.h"
#include "lucky_turn_cfg.h"
#include "mail_config.h"
#include "mail_module.h"
#include "mail_info.h"
#include "ltime_act_module.h"
#include "behavior_id.h"
#include "player_obj.h"
#include "time_util.h"
#include "rank_data.h"
#include "skiplist.h"
#include "db_proxy.h"
#include "message.h"
#include "error.h"
#include "sys.h"

// Lib header

#define LUCKY_TURN_RANK_N 20

class lucky_turn_score_rank_data : public rank_data
{
public:
  bool operator == (const lucky_turn_score_rank_data &data) const
  { return (this->char_id_ == data.char_id_); }
  bool operator > (const lucky_turn_score_rank_data &data) const
  {
    if (this->value_ == data.value_)
      return (this->time_ < data.time_);
    return this->value_ > data.value_;
  }
  int time_;
};

static int s_acc_award_pool = 0;   // bind diamond
static skiplist<lucky_turn_score_rank_data> s_lucky_turn_score_rank;

typedef std::map<int/*char_id*/, pair_t<int/*score,time*/> > char_lucky_turn_score_map_t;
typedef std::map<int/*char_id*/, pair_t<int/*score,time*/> >::iterator char_lucky_turn_score_map_itor;
char_lucky_turn_score_map_t s_char_lucky_turn_score_map;

void lucky_turn_module::on_act_open(const int t)
{
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << 0 << t;
    db_proxy::instance()->send_request(0, REQ_DELETE_LUCKY_TURN_SCORE, &os);
  }
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << 0 << t;
    db_proxy::instance()->send_request(0, REQ_DELETE_LUCKY_TURN_BIG_AWARD, &os);
  }
  if (time_util::now - sys::svc_launch_time > LTIME_ACT_SET_LIMITED_TIME)
    s_acc_award_pool = 0;
}
void lucky_turn_module::on_act_close()
{
  skiplist_node<lucky_turn_score_rank_data> *curr = s_lucky_turn_score_rank.begin();
  int rank = 0;
  for (; curr != s_lucky_turn_score_rank.end(); curr = curr->next())
  {
    ++rank;
    if (rank > LUCKY_TURN_RANK_N) break;

    int char_id = curr->value_.id();
    ilist<item_amount_bind_t> *award_lit =
      lucky_turn_rank_award_cfg::instance()->get_award_list(rank);
    if (award_lit == NULL) continue;
    const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::LUCKY_TURN_RANK_AWARD);
    if (mo == NULL) return ;
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
    if (cbi == NULL) continue;
    int size = 1;
    ::strncpy(mail_module::replace_str[0], MAIL_P_NUMBER, sizeof(mail_module::replace_str[0]) - 1);
    ::snprintf(mail_module::replace_value[0],
               sizeof(mail_module::replace_value[0]),
               "%s(%d)",
               STRING_NUMBER, rank);
    char content[MAX_MAIL_CONTENT_LEN + 1] = {0};
    mail_module::replace_mail_info(mo->content_,
                                   mail_module::replace_str,
                                   mail_module::replace_value,
                                   size,
                                   content);
    mail_module::do_send_mail(char_id,
                              cbi->career_,
                              0,
                              mail_info::MAIL_TYPE_GM,
                              mo->sender_name_,
                              mo->title_,
                              content,
                              *award_lit);
  }
  s_acc_award_pool = 0;
}
int lucky_turn_module::do_turn(player_obj *player)
{
  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE)
      < 1)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND,
                                    global_param_cfg::lucky_turn_cost);
  if (ret != 0) return ret;

  item_amount_bind_t award_item;
  bool notify = false;
  lucky_turn_module::one_turn_award(notify, award_item);
  if (award_item.cid_ == 0)
    return ERR_CONFIG_NOT_EXIST;

  player->do_lose_money(global_param_cfg::lucky_turn_cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_LUCKY_TURN,
                        1,
                        0,
                        0);

  lucky_turn_module::do_acc_award_pool(1);
  lucky_turn_module::do_acc_score(player->id(),
                                  global_param_cfg::lucky_turn_score,
                                  time_util::now,
                                  false);

  if (award_item.cid_ == global_param_cfg::lucky_turn_big_award_cid) // 中奖
    lucky_turn_module::on_got_big_award(player);

  item_obj *new_item = package_module::alloc_new_item(player->id(),
                                                      award_item.cid_,
                                                      award_item.amount_,
                                                      award_item.bind_);
  if (notify)
    notice_module::lucky_turn_award(player->id(),
                                    player->name(),
                                    new_item);
  package_module::release_item(new_item);

  if (award_item.cid_ != global_param_cfg::lucky_turn_big_award_cid) // 中奖
  {
    ilist<item_amount_bind_t> award_list;
    award_list.push_back(award_item);
    package_module::do_insert_award_item_list(player,
                                              &award_list,
                                              blog_t(ITEM_GOT_LUCKY_TURN, 1, 0));
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << award_item.cid_ << award_item.amount_ << award_item.bind_;
  player->send_respond_ok(RES_LUCKY_TURN_ONE, &os);
  return 0;
}
int lucky_turn_module::do_ten_turn(player_obj *player)
{
  if (package_module::package_capacity(player, PKG_PACKAGE)
      - package_module::used_space(player, PKG_PACKAGE)
      < 10)
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  int ret = player->is_money_enough(M_BIND_UNBIND_DIAMOND,
                                    global_param_cfg::lucky_turn_ten_cost);
  if (ret != 0) return ret;

  ilist<item_amount_bind_t> award_list;
  int turn_cnt = 0;
  bool got_big_award = false;
  while (turn_cnt < 10)
  {
    bool notify = false;
    item_amount_bind_t award_item;
    lucky_turn_module::one_turn_award(notify, award_item);

    if (award_item.cid_ == 0)
      return ERR_CONFIG_NOT_EXIST;
    else if (award_item.cid_ == global_param_cfg::lucky_turn_big_award_cid) // 中奖
    {
      if (got_big_award) continue; // 只中一次大奖
      got_big_award = true;
    }

    ++turn_cnt;
    if (award_item.cid_ != global_param_cfg::lucky_turn_big_award_cid) // 中奖
      award_list.push_back(award_item);

    // for notice
    item_obj *new_item = package_module::alloc_new_item(player->id(),
                                                        award_item.cid_,
                                                        award_item.amount_,
                                                        award_item.bind_);
    if (notify)
      notice_module::lucky_turn_award(player->id(),
                                      player->name(),
                                      new_item);
    package_module::release_item(new_item);
  }

  player->do_lose_money(global_param_cfg::lucky_turn_ten_cost,
                        M_BIND_UNBIND_DIAMOND,
                        MONEY_LOSE_LUCKY_TURN,
                        10,
                        0,
                        0);
  //
  lucky_turn_module::do_acc_award_pool(10);
  lucky_turn_module::do_acc_score(player->id(),
                                  global_param_cfg::lucky_turn_ten_score,
                                  time_util::now,
                                  false);
  package_module::do_insert_award_item_list(player,
                                            &award_list,
                                            blog_t(ITEM_GOT_LUCKY_TURN, 10, 0));
  if (got_big_award)
    lucky_turn_module::on_got_big_award(player);

  out_stream os(client::send_buf, client::send_buf_len);
  char cnt = award_list.size();
  if (got_big_award) ++cnt;
  os << cnt;
  while (!award_list.empty())
  {
    item_amount_bind_t v = award_list.pop_front();
    os << v.cid_ << v.amount_ << v.bind_;
  }
  if (got_big_award)
    os << global_param_cfg::lucky_turn_big_award_cid << (int)1 << (char)UNBIND_TYPE;
  player->send_respond_ok(RES_LUCKY_TURN_TEN, &os);
  return 0;
}
void lucky_turn_module::one_turn_award(bool &notify, item_amount_bind_t &award_item)
{
  ilist<lucky_turn_cfg_obj *> *cfg = lucky_turn_cfg::instance()->get_cfg_list();
  int rand_rate = rand() % 10000 + 1;
  int rate_1 = 0;
  int rate_2 = 0;
  bool first_item = true;
  for (ilist_node<lucky_turn_cfg_obj *> *itor = cfg->head();
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
}
void lucky_turn_module::on_got_big_award(player_obj *player)
{
  int add_b_diamond = s_acc_award_pool;
  if (player->b_diamond() + s_acc_award_pool > MONEY_UPPER_LIMIT)
    add_b_diamond = MONEY_UPPER_LIMIT - player->b_diamond();
  player->do_got_money(add_b_diamond, M_BIND_DIAMOND, MONEY_GOT_LUCKY_TURN, 0);
  notice_module::lucky_turn_big_award(player->id(), player->name(), add_b_diamond);

  // 奖池清零
  s_acc_award_pool = 0;
  lucky_turn_module::db_update_acc_award();

  lucky_turn_big_award ltba;
  ltba.char_id_ = player->id();
  ltba.award_time_ = time_util::now;
  ltba.award_v_ = add_b_diamond;
  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << stream_ostr((const char *)&ltba, sizeof(lucky_turn_big_award));
  db_proxy::instance()->send_request(0, REQ_INSERT_LUCKY_TURN_BIG_AWARD, &os);
}
void lucky_turn_module::do_acc_award_pool(const int cnt)
{
  for (int i = 0; i < cnt; ++i)
  {
    int acc_b_diamond = global_param_cfg::lucky_turn_award_diamond.first_;
    if (global_param_cfg::lucky_turn_award_diamond.first_
        < global_param_cfg::lucky_turn_award_diamond.second_)
      acc_b_diamond = global_param_cfg::lucky_turn_award_diamond.first_ + \
                      (rand() % (global_param_cfg::lucky_turn_award_diamond.second_ -
                                 global_param_cfg::lucky_turn_award_diamond.first_ + 1));
    s_acc_award_pool += acc_b_diamond;
  }

  lucky_turn_module::db_update_acc_award();
}
void lucky_turn_module::db_update_acc_award()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << s_acc_award_pool;
  db_proxy::instance()->send_request(0, REQ_UPDATE_LUCKY_TURN_ACC_AWARD, &os);
}
void lucky_turn_module::do_acc_score(const int char_id,
                                     const int score,
                                     const int t,
                                     const bool preload)
{
  pair_t<int> old_v(0, 0);
  char_lucky_turn_score_map_itor itor = s_char_lucky_turn_score_map.find(char_id);
  if (itor == s_char_lucky_turn_score_map.end())
  {
    s_char_lucky_turn_score_map.insert(std::make_pair(char_id, pair_t<int>(score, t)));
    itor = s_char_lucky_turn_score_map.find(char_id);
  }else
  {
    old_v = itor->second;
    itor->second.first_ += score;
    itor->second.second_ = t;
  }
  lucky_turn_module::on_char_lucky_turn_score_change(char_id,
                                                     old_v.first_,
                                                     old_v.second_,
                                                     itor->second.first_,
                                                     itor->second.second_,
                                                     preload);

  if (!preload)
  {
    lucky_turn_score lts;
    lts.char_id_ = char_id;
    lts.score_   = itor->second.first_;
    lts.turn_time_ = itor->second.second_;
    out_stream os(client::send_buf, client::send_buf_len);
    os << 0 << stream_ostr((const char *)&lts, sizeof(lucky_turn_score));
    db_proxy::instance()->send_request(0, REQ_UPDATE_LUCKY_TURN_SCORE, &os);
  }
}
void lucky_turn_module::on_load_lucky_turn_score(const int char_id,
                                                 const int score,
                                                 const int t,
                                                 const bool preload)
{
  lucky_turn_module::do_acc_score(char_id, score, t, preload);
}
void lucky_turn_module::on_char_lucky_turn_score_change(const int char_id,
                                                        const int org_score,
                                                        const int org_time,
                                                        const int cur_score,
                                                        const int cur_time,
                                                        const bool preload)
{
  lucky_turn_score_rank_data data;
  data.char_id_ = char_id;

  if (!preload
      && org_time != 0)
  {
    data.value_ = org_score;
    data.time_ = org_time;
    s_lucky_turn_score_rank.drop(data);
  }

  data.value_ = cur_score;
  data.time_ = cur_time;
  s_lucky_turn_score_rank.insert(data);
}
void lucky_turn_module::on_load_lucky_turn(const int v)
{
  s_acc_award_pool = v;
}
int lucky_turn_module::do_fetch_lucky_turn_info(player_obj *player, out_stream &os)
{
  char_lucky_turn_score_map_itor itor = s_char_lucky_turn_score_map.find(player->id());
  int my_score = 0;
  if (itor != s_char_lucky_turn_score_map.end())
    my_score = itor->second.first_;

  os << s_acc_award_pool
    << my_score;
  char *cnt = os.wr_ptr();
  os << (char)0;
  skiplist_node<lucky_turn_score_rank_data> *curr = s_lucky_turn_score_rank.begin();
  for (; curr != s_lucky_turn_score_rank.end(); curr = curr->next())
  {
    int char_id = curr->value_.id();
    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(char_id);
    if (cbi == NULL) continue;

    char_lucky_turn_score_map_itor itor = s_char_lucky_turn_score_map.find(char_id);
    int score = 0;
    if (itor != s_char_lucky_turn_score_map.end())
      score = itor->second.first_;
    os << score << stream_ostr(cbi->name_, ::strlen(cbi->name_));

    ++(*cnt);
    if (*cnt > LUCKY_TURN_RANK_N) break;
  }
  return 0;
}
int lucky_turn_module::do_fetch_big_award_log(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_GET_LUCKY_TURN_BIG_AWARD_LIST, &os);
  return 0;
}
int lucky_turn_module::handle_db_get_lucky_turn_big_award_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;

  out_stream os(client::send_buf, client::send_buf_len);
  os << (short)cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char bf[sizeof(lucky_turn_big_award) + 4] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    lucky_turn_big_award *p = (lucky_turn_big_award *)bf;

    char_brief_info *cbi = all_char_info::instance()->get_char_brief_info(p->char_id_);
    if (cbi == NULL)
      os << p->award_time_ << stream_ostr("null", 4) << p->award_v_;
    else
      os << p->award_time_ << stream_ostr(cbi->name_, ::strlen(cbi->name_)) << p->award_v_;
  }
  player->send_respond_ok(RES_OBTAIN_LUCKY_TURN_BIG_AWARD_LOG, &os);
  return 0;
}
