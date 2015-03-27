#include "rank_module.h"
#include "player_obj.h"
#include "sys_log.h"
#include "message.h"
#include "error.h"
#include "unused_arg.h"
#include "rank_data.h"
#include "global_param_cfg.h"
#include "time_util.h"
#include "player_mgr.h"
#include "cache_module.h"
#include "db_proxy.h"
#include "package_module.h"
#include "jing_ji_module.h"
#include "skiplist.h"
#include "sys.h"

// Lib header
#include <math.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("rank");
static ilog_obj *e_log = err_log::instance()->get_ilog("rank");

static skiplist<lvl_rank_data> s_lvl_rank;
static skiplist<zhanli_rank_data> s_zhanli_rank;
static skiplist<mstar_rank_data> s_mstar_rank;

template <typename VALUE>
void build_rank_info(skiplist<VALUE> &list, out_stream &os, const int)
{
  short *count = (short*)os.wr_ptr();
  os << short(0);
  skiplist_node<VALUE> *curr = list.begin();
  for (; curr != list.end(); curr = curr->next())
  {
    curr->value_.build_info(os);
    ++(*count);
    if (*count >= global_param_cfg::rank_amount) return;
  }
}
template <typename VALUE>
void get_char_list_before_rank_i(skiplist<VALUE> &list, ilist<int> &char_list, const int rank)
{
  skiplist_node<VALUE> *curr = list.begin();
  for (; curr != list.end(); curr = curr->next())
  {
    char_list.push_back(curr->value_.id());
    if (char_list.size() >= rank) return;
  }
}
template <typename VALUE>
void dump(skiplist<VALUE> &list, ilog_obj *log, const int count)
{
  log->debug("there is %d players and %d level", list.length(), list.max_level());
  int rank = 0;
  skiplist_node<VALUE> *curr = list.begin();
  for (; curr != list.end() && rank != count; curr = curr->next())
  {
    log->debug("rank %d char %d value %d",
               ++ rank, curr->value_.char_id_, curr->value_.value_);
  }
}

int rank_module::init()
{
  rank_module::update_world_lvl(0);
  return 0;
}
void rank_module::update_world_lvl(const int char_lvl)
{
  if (char_lvl > 0 && sys::world_lvl >= char_lvl) return ;
  skiplist_node<lvl_rank_data> *curr = s_lvl_rank.begin();
  int min_lvl = 10000;
  for (int i = 0;
       curr != s_lvl_rank.end() && i < global_param_cfg::world_lvl_rank;
       curr = curr->next(), ++i)
  {
    if (curr->value_.value_ < min_lvl)
      min_lvl = curr->value_.value_;
  }
  sys::world_lvl = min_lvl;
  s_log->rinfo("world lvl is %d", sys::world_lvl);
}
int rank_module::dispatch_msg(player_obj *player,
                              const int msg_id,
                              const char *msg,
                              const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = rank_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch (msg_id)
  {
    SHORT_CODE(REQ_OBTAIN_RANK_INFO, clt_obtain_rank_info);
    //
    SHORT_DEFAULT;
  }
  return ret;
}
int rank_module::clt_obtain_rank_info(player_obj *player,
                                      const char *msg,
                                      const int len)
{
  char rank_type = 0;
  int last_time = 0;
  in_stream is(msg, len);
  is >> rank_type >> last_time;

  out_stream os(client::send_buf, client::send_buf_len);
  os << rank_type << time_util::now;
  if (rank_type == RANK_LVL)
    build_rank_info(s_lvl_rank, os, last_time);
  else if (rank_type == RANK_ZHANLI)
    build_rank_info(s_zhanli_rank, os, last_time);
  else if (rank_type == RANK_JING_JI)
    jing_ji_module::do_build_n_rank_info(os);
  else if (rank_type == RANK_MSTAR)
    build_rank_info(s_mstar_rank, os, last_time);
  else
    return player->send_respond_err(RES_OBTAIN_RANK_INFO,
                                    ERR_CLIENT_OPERATE_ILLEGAL);

  return player->send_respond_ok(RES_OBTAIN_RANK_INFO, &os);
}
void rank_module::do_timeout(const int /*now*/)
{
#if 0
  static int pre = 0;
  pre = (pre + 1) % 10;
  if (pre == 0)
  {
    e_log->debug("dump lvl rank");
    dump(s_lvl_rank, e_log, 10);
    e_log->debug("dump zhanli rank");
    dump(s_zhanli_rank, e_log, 10);
  }
#endif
}
void rank_module::on_new_day(const int /*now*/)
{
}
void rank_module::on_char_get_exp(const int char_id,
                                  const short org_lvl,
                                  const int64_t org_exp,
                                  const short cur_lvl,
                                  const int64_t cur_exp,
                                  const bool preload)
{
  lvl_rank_data lrd;
  lrd.char_id_ = char_id;

  if (!preload)
  {
    lrd.value_ = org_lvl;
    lrd.exp_ = org_exp;
    s_lvl_rank.drop(lrd);
  }

  lrd.value_ = cur_lvl;
  lrd.exp_ = cur_exp;
  s_lvl_rank.insert(lrd);
}
void rank_module::on_char_zhanli_change(const int char_id,
                                        const int org_zhanli,
                                        const int cur_zhanli,
                                        const bool preload)
{
  zhanli_rank_data zlrd;
  zlrd.char_id_ = char_id;

  if (!preload)
  {
    zlrd.value_ = org_zhanli;
    s_zhanli_rank.drop(zlrd);
  }

  zlrd.value_ = cur_zhanli;
  s_zhanli_rank.insert(zlrd);
}
void rank_module::on_char_mstar_change(const int char_id,
                                       const int org_mstar,
                                       const int cur_mstar,
                                       const bool preload)
{
  mstar_rank_data mrd;
  mrd.char_id_ = char_id;

  if (!preload)
  {
    mrd.value_ = org_mstar;
    s_mstar_rank.drop(mrd);
  }

  mrd.value_ = cur_mstar;
  s_mstar_rank.insert(mrd);
}
void rank_module::get_char_list_before_rank(const int type, ilist<int> &cl, const int rank)
{
  if (type == rank_module::RANK_LVL) 
    get_char_list_before_rank_i(s_lvl_rank, cl, rank);
  else if (type == rank_module::RANK_ZHANLI)
    get_char_list_before_rank_i(s_zhanli_rank, cl, rank);
  else if (type == rank_module::RANK_MSTAR)
    get_char_list_before_rank_i(s_mstar_rank, cl, rank);
}
int rank_module::rand_players_for_lue_duo(player_obj *player,
                                          const int material_cid,
                                          lueduo_check_handler handler,
                                          const int need_cnt,
                                          int result[])
{
  lvl_rank_data data;
  data.char_id_ = player->id();
  data.value_ = player->lvl();
  data.exp_ = player->char_info_->exp_;
  skiplist_node<lvl_rank_data> *my_rank = s_lvl_rank.find(data);
  if (my_rank == NULL)
  {
    e_log->error("char %d not in lvl rank", player->id());
    return 0;
  }

  int deal_cnt = 0;
  skiplist_node<lvl_rank_data> *iter = my_rank->next();
  for (; iter != NULL; iter = iter->next())
  {
    lvl_rank_data &lrd = iter->value_;
    if (lrd.value_ < global_param_cfg::lue_duo_lvl
        || ::abs(lrd.value_ - player->lvl()) > 5)
      break;

    if (handler(lrd.char_id_, material_cid))
    {
      if (deal_cnt < need_cnt)
        result[deal_cnt ++] = lrd.char_id_;
      else
      {
        if (rand() % 1000000 < int(need_cnt * 1000000.0 / (++ deal_cnt)))
          result[rand() % need_cnt] = lrd.char_id_;
      }
    }
  }
  for (iter = my_rank->prev(); iter != NULL; iter = iter->prev())
  {
    lvl_rank_data &lrd = iter->value_;
    if (lrd.value_ < global_param_cfg::lue_duo_lvl
        || ::abs(lrd.value_ - player->lvl()) > 5)
      break;

    if (handler(lrd.char_id_, material_cid))
    {
      if (deal_cnt < need_cnt)
        result[deal_cnt ++] = lrd.char_id_;
      else
      {
        if (rand() % 1000000 < int(need_cnt * 1000000.0 / (++ deal_cnt)))
          result[rand() % need_cnt] = lrd.char_id_;
      }
    }
  }
  return (deal_cnt > need_cnt ? need_cnt : deal_cnt);
}
