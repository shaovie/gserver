#include "task_module.h"
#include "task_def.h"
#include "package_module.h"
#include "behavior_log.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "task_config.h"
#include "player_obj.h"
#include "error.h"
#include "time_util.h"
#include "def.h"
#include "sys.h"
#include "client.h"
#include "clsid.h"
#include "db_proxy.h"
#include "item_config.h"
#include "monster_cfg.h"

// Lib header
#include <string.h>

static ilog_obj *s_log = sys_log::instance()->get_ilog("task");
static ilog_obj *e_log = err_log::instance()->get_ilog("task");

void task_module::init(player_obj *player)
{ player->task_data_ = new task_data(); }
void task_module::destroy(player_obj *player)
{
  delete player->task_data_;
  player->task_data_ = NULL;
}
int task_module::handle_db_get_task_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char db_info_bf[sizeof(task_info) + 4] = {0};
    stream_istr db_info_si(db_info_bf, sizeof(db_info_bf));
    is >> db_info_si;
    task_info *ti = (task_info *)db_info_bf;

    const task_cfg_obj *tco = task_config::instance()->find(ti->task_cid_);
    if (tco == NULL) continue;

    task_info *ninfo = task_info_pool::instance()->alloc();
    ::memcpy(ninfo, ti, sizeof(task_info));
    player->task_data_->task_info_map_.insert(std::make_pair(ninfo->task_cid_, ninfo));
  }

  task_module::on_load_task_list_ok(player);
  return 0;
}
int task_module::handle_db_get_task_bit_array_result(player_obj *player,
                                                     in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  if (cnt == 0) return 0;
  char bf[sizeof(task_bit_array) + 16] = {0};
  stream_istr bf_si(bf, sizeof(bf));
  is >> bf_si;
  in_stream bit_is(bf_si.str(), bf_si.str_len());
  bit_is >> &(player->task_data_->bit_array_);
  return 0;
}
//== gm
int task_module::gm_add_task(player_obj *player, const int task_cid)
{
  int t = TASK_TYPE(task_cid);
  if (t != TRUNK_TASK
      && t != BRANCH_TASK)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_CLIENT_OPERATE_ILLEGAL);

  const task_cfg_obj* tco = task_config::instance()->find(task_cid);
  if (tco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_EXIST);
  if (player->task_data_->task_info_map_.find(task_cid) != \
      player->task_data_->task_info_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_HAD_ACCEPTED);

  task_info *tinfo = task_info_pool::instance()->alloc();
  tinfo->char_id_    = player->id();
  tinfo->task_cid_   = tco->task_cid_;
  tinfo->state_      = TASK_ST_INPROGRESS;
  int ret = task_module::do_accept_task(player, tco, tinfo);
  if (ret != 0)
  {
    task_info_pool::instance()->release(tinfo);
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  }
  return 0;
}
int task_module::gm_submit_task(player_obj *player, const int task_cid)
{
  const task_cfg_obj* tco = task_config::instance()->find(task_cid);
  if (tco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_EXIST);

  task_info_map_itor itor = player->task_data_->task_info_map_.find(tco->task_cid_);
  if (itor == player->task_data_->task_info_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_ACCEPTED);

  task_info *tinfo = itor->second;
  if (tinfo->state() != TASK_ST_COMPLETED)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_CAN_NOT_SUBMIT);

  int ret = task_module::do_submit_task(player, tinfo, tco);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int task_module::gm_rm_task(player_obj *player, const int task_cid)
{
  task_info_map_itor itor = player->task_data_->task_info_map_.find(task_cid);
  if (itor != player->task_data_->task_info_map_.end())
  {
    task_info * tinfo = itor->second;
    player->task_data_->task_info_map_.erase(task_cid);
    task_module::do_del_task_todo_info(player, task_cid);
    task_module::do_update_task_info_2_db(player, REQ_DELETE_TASK, tinfo);
    task_module::do_update_task_info_2_client(player, TASK_DEL, tinfo);
    task_info_pool::instance()->release(tinfo);
  }
  if (player->task_data_->task_acceptable_list_.find(task_cid))
  {
    player->task_data_->task_acceptable_list_.remove(task_cid);
    task_info tinfo(player->id(), task_cid, TASK_ST_ACCEPTABLE);
    task_module::do_update_task_info_2_client(player, TASK_DEL, &tinfo);
  }
  return 0;
}
int task_module::dispatch_msg(player_obj *player,
                              const int msg_id,
                              const char *msg,
                              const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = task_module::FUNC(player, msg, len);   \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", msg_id);       \
  break

  int ret = 0;
  switch(msg_id)
  {
    SHORT_CODE(REQ_ACCEPT_TASK,         clt_accept_task);
    SHORT_CODE(REQ_SUBMIT_TASK,         clt_submit_task);
    SHORT_CODE(REQ_ABANDON_TASK,        clt_abandon_task);
    SHORT_CODE(REQ_DO_GUIDE,            clt_do_guide);

    SHORT_DEFAULT;
  }
  return ret;
}
int task_module::clt_accept_task(player_obj *player, const char *msg, const int len)
{
  int task_cid = 0;
  in_stream in(msg, len);
  in >> task_cid;

  const task_cfg_obj* tco = task_config::instance()->find(task_cid);
  if (tco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_EXIST);
  if (player->task_data_->task_info_map_.find(task_cid) != \
      player->task_data_->task_info_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_HAD_ACCEPTED);

  int ret = 0;
  switch (TASK_TYPE(task_cid))
  {
  case TRUNK_TASK:
  case BRANCH_TASK:
  case DAILY_TASK:
  case GUILD_TASK:
    if (!task_module::is_in_acceptable_list(player, tco->task_cid_))
      return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_CAN_NOT_ACCEPT);
    ret = task_module::do_accept_common_task(player, tco);
    break;
  default:
    ret = ERR_TASK_NOT_EXIST;
    break;
  }
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int task_module::do_accept_common_task(player_obj *player,
                                       const task_cfg_obj *tco)
{
  task_info *tinfo = task_info_pool::instance()->alloc();
  tinfo->char_id_    = player->id();
  tinfo->task_cid_   = tco->task_cid_;
  tinfo->state_      = TASK_ST_INPROGRESS;

  int ret = task_module::do_accept_task(player, tco, tinfo);
  if (ret != 0)
    task_info_pool::instance()->release(tinfo);
  return ret;
}
int task_module::do_accept_task(player_obj *player,
                                const task_cfg_obj *tco,
                                task_info *tinfo)
{
  if (task_module::is_package_full(player, tco->accept_do_ai_))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  // 执行接受任务时的AI
  task_module::do_task_do_ai(player, tinfo, tco->accept_do_ai_);

  // 执行接受任务时的目标AI
  task_module::do_task_to_ai(player, tco->task_cid_, tco->target_to_ai_);

  // 检查任务是否达成目标
  if (task_module::do_check_task_todo_list(player, tco->target_to_ai_, tinfo) == 0)
    tinfo->state(TASK_ST_COMPLETED);

  // 从可接列表删除
  player->task_data_->task_acceptable_list_.remove(tco->task_cid_);

  // update db
  task_module::do_update_task_info_2_db(player, REQ_INSERT_TASK, tinfo);

  // 任务应该都是从可接任务列表中接受的，更新任务状态
  task_module::do_update_task_info_2_client(player, TASK_UPDATE, tinfo);

  player->task_data_->task_info_map_.insert(std::make_pair(tco->task_cid_, tinfo));
  behavior_log::instance()->store(BL_TASK_ACCEPT,
                                  time_util::now,
                                  "%d|%d",
                                  player->id(),
                                  tinfo->task_cid_);
  return 0;
}
int task_module::clt_submit_task(player_obj *player, const char *msg, const int len)
{
  int task_cid = 0;
  in_stream in(msg, len);
  in >> task_cid;

  const task_cfg_obj* tco = task_config::instance()->find(task_cid);
  if (tco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_EXIST);

  task_info_map_itor itor = player->task_data_->task_info_map_.find(tco->task_cid_);
  if (itor == player->task_data_->task_info_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_ACCEPTED);

  task_info *tinfo = itor->second;
  if (tinfo->state() != TASK_ST_COMPLETED)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_CAN_NOT_SUBMIT);

  int ret = task_module::do_submit_task(player, tinfo, tco);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int task_module::do_submit_task(player_obj *player,
                                task_info *tinfo,
                                const task_cfg_obj *tco)
{
  if (task_module::is_package_full(player, tco->award_do_ai_))
    return ERR_PACKAGE_SPACE_NOT_ENOUGH;

  // delete task info
  player->task_data_->task_info_map_.erase(tco->task_cid_);

  task_module::do_del_task_todo_info(player, tco->task_cid_);

  // delete db
  task_module::do_update_task_info_2_db(player, REQ_DELETE_TASK, tinfo);

  if (IS_ONLY_ONCE_TASK(tco->task_cid_))
  {
    player->task_data_->bit_array_.char_id_ = player->id(); // !!!!
    int task_type = TASK_TYPE(tco->task_cid_);
    player->task_data_->bit_array_.set(task_type-1, TASK_BIT_IDX(tco->task_cid_));
    task_module::do_update_task_bit_array_2_db(player);
  }

  task_module::do_task_do_ai(player, tinfo, tco->award_do_ai_);

  // notify to player
  task_module::do_update_task_info_2_client(player, TASK_DEL, tinfo);

  // done_cnt++ in finish
  // complete must be after do_finish because of DO_RANDOM_ASSIGN_XXX_TASK
  task_module::on_finish_task(player, tinfo);

  // 完成AI 里边可能会自动接受任务，为了避免自动接受重复任务，
  // 必须先删除当前的任务，再接受新任务
  task_module::do_task_do_ai(player, tinfo, tco->complete_do_ai_);

  task_info_pool::instance()->release(tinfo);

  task_module::do_update_acceptable_task_list_to_clt(player);

  task_module::do_notify_to_clt_if_no_task(player);

  behavior_log::instance()->store(BL_TASK_SUBMIT,
                                  time_util::now,
                                  "%d|%d",
                                  player->id(),
                                  tco->task_cid_);
  return 0;
}
int task_module::clt_abandon_task(player_obj *player, const char *msg, const int len)
{
  int task_cid = 0;
  in_stream in(msg, len);
  in >> task_cid;

  const task_cfg_obj* tco = task_config::instance()->find(task_cid);
  if (tco == NULL)
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_EXIST);

  task_info_map_itor itor = player->task_data_->task_info_map_.find(task_cid);
  if (itor == player->task_data_->task_info_map_.end())
    return player->send_respond_err(NTF_OPERATE_RESULT, ERR_TASK_NOT_ACCEPTED);

  task_info *tinfo = itor->second;
  int ret = task_module::do_abandon_task(player, tco, tinfo);
  if (ret != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT, ret);
  return 0;
}
int task_module::do_abandon_task(player_obj *player,
                                 const task_cfg_obj *tco,
                                 task_info *tinfo)
{
  task_module::do_del_task_todo_info(player, tco->task_cid_);

  player->task_data_->task_info_map_.erase(tco->task_cid_);

  task_module::do_update_task_info_2_db(player, REQ_DELETE_TASK, tinfo);

  task_module::do_update_task_info_2_client(player, TASK_DEL, tinfo);

  task_info_pool::instance()->release(tinfo);

  task_module::do_update_acceptable_task_list_to_clt(player);
  return 0;
}
int task_module::clt_do_guide(player_obj *player, const char *msg, const int len)
{
  int id = 0;
  in_stream in(msg, len);
  in >> id;
  task_module::do_update_task_todo_list(player,
                                        TO_FINISH_GUIDE,
                                        id,
                                        0);
  return 0;
}
//== on event
void task_module::on_enter_game(player_obj *player)
{
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << player->id();
    db_proxy::instance()->send_request(player->id(), REQ_GET_TASK_BIT_ARRAY, &os);
  }
  { // MUST BE LAST !!
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << player->id();
    db_proxy::instance()->send_request(player->id(), REQ_GET_TASK_LIST, &os);
  }
}
void task_module::on_load_task_list_ok(player_obj *player)
{
  task_module::do_build_all_task_todo_info(player);
  task_module::do_build_acceptable_task_list(player);
  task_module::do_send_task_list_2_client(player);
  task_module::do_notify_to_clt_if_no_task(player);
}
void task_module::on_new_day(player_obj *player, const bool when_login)
{
  if (!when_login)
    task_module::do_update_acceptable_task_list_to_clt(player);
}
void task_module::on_kill_char(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_KILL_CHAR,
                                        0,
                                        0);
}
void task_module::on_kill_mst(player_obj* player, const int mst_cid, const int sort)
{
  if (sort == MST_SORT_BOSS)
    task_module::do_update_task_todo_list(player,
                                          TO_KILL_BOSS,
                                          0,
                                          0);
  else
    task_module::do_update_task_todo_list(player,
                                          TO_KILL_MST,
                                          mst_cid,
                                          0);
}
void task_module::on_char_lvl_up(player_obj *player)
{
  task_module::do_update_acceptable_task_list_to_clt(player);
  task_module::do_update_task_todo_list(player, TO_UP_LEVEL, 0, 0);
}
void task_module::on_finish_tui_tu(player_obj *player,
                                   const int scene_cid,
                                   const int star)
{
  task_module::do_update_task_todo_list(player,
                                        TO_FINISH_MISSION,
                                        scene_cid,
                                        0);
  task_module::do_update_task_todo_list(player,
                                        TO_FINISH_STAR_MISSION,
                                        scene_cid,
                                        star);
}
void task_module::on_equip_strengthen(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_EQUIP_STRENGTHEN,
                                        0,
                                        0);
}
void task_module::on_upgrade_pskill(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_UPGRADE_PSKILL,
                                        0,
                                        0);
}
void task_module::on_guan_gai(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_GUAN_GAI,
                                        0,
                                        0);
}
void task_module::on_mo_bai(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_MO_BAI,
                                        0,
                                        0);
}
void task_module::on_jing_ji(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_JING_JI,
                                        0,
                                        0);
}
void task_module::on_add_friends(player_obj* player)
{
  task_module::do_update_task_todo_list(player,
                                        TO_ADD_FRIENDS,
                                        0,
                                        0);
}
void task_module::on_join_guild(player_obj *player)
{
  task_module::do_update_acceptable_task_list_to_clt(player);
}
void task_module::on_exit_guild(player_obj *player)
{
  task_info *tinfo = task_module::find_task_by_type(player, GUILD_TASK);
  if (tinfo != NULL)
  {
    const task_cfg_obj* tco = task_config::instance()->find(tinfo->task_cid_);
    if (tco != NULL)
      task_module::do_abandon_task(player, tco, tinfo);
  }else
    task_module::do_update_acceptable_task_list_to_clt(player);
}
//== do_xx method
int task_module::do_send_task_list_2_client(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  char *cnt_ptr = os.wr_ptr();
  *cnt_ptr = 0;
  os << (char)0;

  task_info_map_itor itor = player->task_data_->task_info_map_.begin();
  for (int n = 0;
       itor != player->task_data_->task_info_map_.end() && n < 127;
       ++itor, ++n)
  {
    task_module::do_fetch_task_info(player, TASK_ADD, itor->second, os);
    (*cnt_ptr)++;
  }

  ilist_node<int> *acpt_itor = player->task_data_->task_acceptable_list_.head();
  for (; acpt_itor != NULL; acpt_itor = acpt_itor->next_)
  {
    task_info ti(player->id(), acpt_itor->value_, TASK_ST_ACCEPTABLE);
    task_module::do_fetch_task_info(player, TASK_ADD, &ti, os);
    (*cnt_ptr)++; 
  }
  return player->send_respond_ok(NTF_TASK_UPDATE, &os);
}
void task_module::do_build_acceptable_task_list(player_obj *player)
{
  ilist<int> &tl = task_config::instance()->get_all_task_list();
  for (ilist_node<int> *itor = tl.head();
       itor != NULL;
       itor = itor->next_)
  {
    int task_cid = itor->value_;
    const task_cfg_obj *tco = task_config::instance()->find(task_cid);
    if (NULL == tco) continue;

    if (task_module::can_accept(player, tco))
      player->task_data_->task_acceptable_list_.push_back(task_cid);
  }
}
int task_module::do_update_acceptable_task_list_to_clt(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  char *cnt_ptr = os.wr_ptr();
  os << (char)0;

  int cnt = task_module::do_update_acceptable_task_list(player, os);
  if (cnt == 0) return 0;

  *cnt_ptr = cnt;
  return player->send_respond_ok(NTF_TASK_UPDATE, &os);
}
int task_module::do_update_acceptable_task_list(player_obj *player, out_stream &os)
{
  static int t_int_vect[MAX_TASK_NUM];
  int add_len = 0;
  int del_len = 0;
  // head:add               tail:del
  ilist<int> &tl = task_config::instance()->get_all_task_list();
  for (ilist_node<int> *itor = tl.head();
       itor != NULL;
       itor = itor->next_)
  {
    int task_cid = itor->value_;
    bool in_acpt_list = task_module::is_in_acceptable_list(player, task_cid);

    const task_cfg_obj *tco = task_config::instance()->find(task_cid);
    if (NULL == tco) continue;

    if (!task_module::can_accept(player, tco))
    {
      if (in_acpt_list)
      {
        del_len++;
        t_int_vect[MAX_TASK_NUM - del_len] = task_cid;
        player->task_data_->task_acceptable_list_.remove(task_cid);
      }
    }else
    {
      if (!in_acpt_list)
      {
        t_int_vect[add_len] = task_cid;
        add_len++;
        player->task_data_->task_acceptable_list_.push_back(task_cid);
      }
    }
  }
  int total = add_len + del_len;
  if (0 == total) return 0;

  if (total >= 127)
  {
    e_log->wning("char %d update task list is too large [%d]!",
                 player->id(), total);
    total = 126;
  }

  for (int i = 0; i < add_len; ++i)
  {
    task_info ti(player->id(), t_int_vect[i], TASK_ST_ACCEPTABLE);
    task_module::do_fetch_task_info(player, TASK_ADD, &ti, os);
  }
  for (int i = MAX_TASK_NUM - del_len; i < MAX_TASK_NUM; ++i)
  {
    task_info ti(player->id(), t_int_vect[i], TASK_ST_ACCEPTABLE);
    task_module::do_fetch_task_info(player, TASK_DEL, &ti, os);
  }
  return total;
}
void task_module::do_notify_to_clt_if_no_task(player_obj *player)
{
  if (!player->task_data_->task_acceptable_list_.empty()
      || !player->task_data_->task_info_map_.empty())
    return ;
  player->send_msg(NTF_NO_TASK, 0, NULL);
}
bool task_module::can_accept(player_obj* player, const task_cfg_obj *tco)
{
  task_info_map_itor itor = player->task_data_->task_info_map_.find(tco->task_cid_);
  if (itor != player->task_data_->task_info_map_.end())
    return false;

  //判断任务是否已经完成过了
  if (IS_ONLY_ONCE_TASK(tco->task_cid_))
  {
    int task_type = TASK_TYPE(tco->task_cid_);
    if (player->task_data_->bit_array_.in_set(task_type-1, TASK_BIT_IDX(tco->task_cid_)))
      return false;
  }
  if (tco->pre_task_cid_ != 0 && IS_ONLY_ONCE_TASK(tco->pre_task_cid_))
  {
    int task_type = TASK_TYPE(tco->pre_task_cid_);
    if (!player->task_data_->bit_array_.in_set(task_type-1, TASK_BIT_IDX(tco->pre_task_cid_)))
      return false;
  }

  if (task_module::do_task_if_ai(player, tco->accept_if_ai_) != 0)
    return false;
  return true;
}
bool task_module::is_in_acceptable_list(player_obj *player, const int task_cid)
{
  for (ilist_node<int> *itor = player->task_data_->task_acceptable_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (task_cid == itor->value_)
      return true;
  }
  return false;
}
int task_module::do_update_task_bit_array_2_db(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << &(player->task_data_->bit_array_);
  return db_proxy::instance()->send_request(player->id(), REQ_UPDATE_TASK_BIT_ARRAY, &os);
}
int task_module::do_update_task_info_2_db(player_obj *player, const int op, task_info *tinfo)
{
  out_stream os(client::send_buf, client::send_buf_len);
  if (op == REQ_DELETE_TASK)
    os << player->db_sid() << player->id() << tinfo->task_cid_;
  else
    os << player->db_sid() << stream_ostr((const char *)tinfo, sizeof(task_info));
  if (db_proxy::instance()->send_request(player->id(), op, &os) != 0)
    return -1;
  tinfo->dirty(false);
  return 0;
}
int task_module::do_update_task_info_2_client(player_obj *player,
                                              const int task_op,
                                              task_info *tinfo)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << (char)1;
  task_module::do_fetch_task_info(player, task_op, tinfo, os);
  return player->send_respond_ok(NTF_TASK_UPDATE, &os);
}
void task_module::do_fetch_task_info(player_obj *,
                                     const int task_op,
                                     task_info *tinfo,
                                     out_stream &os)
{
  os << (char)task_op
    << tinfo->task_cid_
    << tinfo->state_
    << tinfo->value_[0]
    << tinfo->value_[1]
    << tinfo->value_[2]
    << tinfo->extra_value_;
}
bool task_module::is_package_full(player_obj* player, task_ai_list_t *p_ai_list)
{
  if (p_ai_list == NULL) return false;

  int pkg_item_amount = 0;
  for (task_ai_list_itor itor = p_ai_list->head();
       itor != NULL;
       itor = itor->next_)
  {
    task_ai_info *tai = itor->value_;
    if (tai->type_ == DO_GIVE_ITEM)
    {
      if (item_config::item_is_money(tai->args_[0]))
        continue;
      g_item_amount_bind[0][pkg_item_amount] = tai->args_[0];
      g_item_amount_bind[1][pkg_item_amount] = tai->args_[1];
      g_item_amount_bind[2][pkg_item_amount] = tai->args_[2];
      pkg_item_amount++;
    }
  }
  if (pkg_item_amount == 0) return false;
  return package_module::would_be_full(player,
                                       PKG_PACKAGE,
                                       g_item_amount_bind[0],
                                       g_item_amount_bind[1],
                                       g_item_amount_bind[2],
                                       pkg_item_amount);
}
void task_module::on_finish_task(player_obj *player, task_info *tinfo)
{
  const int task_type = TASK_TYPE(tinfo->task_cid_);
  if (task_type == DAILY_TASK)
  {
    player->daily_clean_info_->daily_task_done_cnt_ += 1;
    player->db_save_daily_clean_info();
  }else if (task_type == GUILD_TASK)
  {
    player->daily_clean_info_->guild_task_done_cnt_ += 1;
    player->db_save_daily_clean_info();
  }
}
task_info *task_module::find_task_by_type(player_obj *player, const int task_type)
{
  for (task_info_map_itor itor = player->task_data_->task_info_map_.begin();
       itor != player->task_data_->task_info_map_.end();
       ++itor)
  {
    if (TASK_TYPE(itor->second->task_cid_) == task_type)
      return itor->second;
  }
  return NULL;
}
