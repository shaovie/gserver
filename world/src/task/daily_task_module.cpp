#include "daily_task_module.h"
#include "player_obj.h"
#include "task_config.h"
#include "task_module.h"
#include "db_proxy.h"
#include "task_def.h"
#include "istream.h"

// Lib header

void daily_task_module::handle_db_get_daily_task_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char daily_task_bf[sizeof(daily_task_info) + 4] = {0};
    stream_istr si(daily_task_bf, sizeof(daily_task_bf));
    is >> si;
    daily_task_info *info = (daily_task_info *)daily_task_bf;

    const task_cfg_obj *tco = task_config::instance()->find(info->task_cid_);
    if (tco == NULL) continue;

    daily_task_info *new_info = new daily_task_info();
    ::memcpy(new_info, info, sizeof(daily_task_info));
    player->task_data_->daily_task_list_.push_back(new_info);
  }
}
void daily_task_module::on_new_day(player_obj *player)
{
  bool to_clean = false;
  for (ilist_node<daily_task_info *> *itor = player->task_data_->daily_task_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->done_cnt_ > 0)
    {
      itor->value_->done_cnt_ = 0;
      to_clean = true;
    }
  }
  if (!to_clean) return ;
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id() << (int)0;
  db_proxy::instance()->send_request(player->id(), REQ_CLEAN_ALL_DAILY_TASK, &os);
}
void daily_task_module::on_finish_task(player_obj *player, const int task_cid)
{
  daily_task_info *info = daily_task_module::get_daily_task_info(player, task_cid);
  if (info == NULL)
  {
    info = new daily_task_info();
    info->char_id_  = player->id();
    info->task_cid_ = task_cid;
    info->done_cnt_ = 1;
    player->task_data_->daily_task_list_.push_back(info);
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << stream_ostr((const char *)info, sizeof(daily_task_info));
    db_proxy::instance()->send_request(player->id(), REQ_INSERT_DAILY_TASK, &os);
  }else
  {
    info->done_cnt_ += 1;
    out_stream os(client::send_buf, client::send_buf_len);
    os << player->db_sid() << stream_ostr((const char *)info, sizeof(daily_task_info));
    db_proxy::instance()->send_request(player->id(), REQ_UPDATE_DAILY_TASK, &os);
  }
}
int daily_task_module::task_done_cnt(player_obj *player, const int task_cid)
{
  daily_task_info *info = daily_task_module::get_daily_task_info(player, task_cid);
  if (info == NULL) return 0;
  return info->done_cnt_;
}
daily_task_info *daily_task_module::get_daily_task_info(player_obj *player,
                                                        const int task_cid)
{
  for (ilist_node<daily_task_info *> *itor = player->task_data_->daily_task_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->task_cid_ == task_cid)
      return itor->value_;
  }
  return NULL;
}
