#include "task_module.h"
#include "daily_task_module.h"
#include "global_param_cfg.h"
#include "package_module.h"
#include "guild_module.h"
#include "task_config.h"
#include "player_obj.h"
#include "task_info.h"
#include "mblock_pool.h"
#include "behavior_id.h"
#include "task_def.h"
#include "message.h"
#include "sys_log.h"
#include "clsid.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("task");
static ilog_obj *e_log = err_log::instance()->get_ilog("task");

//= 'if` ai
bool task_module::do_task_if_ai(player_obj *player, task_ai_list_t *p_ai_list)
{
  if (p_ai_list == NULL) return 0;
  task_ai_list_itor itor = p_ai_list->head();
  for (; itor != NULL; itor = itor->next_)
  {
    task_ai_info *tai = itor->value_;
    switch (tai->type_)
    {
    case IF_IN_LEVEL:
      if (player->lvl() < tai->args_[0] || player->lvl() > tai->args_[1])
        return -1;
      break;
    case IF_IN_GUILD:
      if (player->guild_id() == 0)
        return -1;
      break;
    case IF_NO_TASK:
      if (task_module::find_task_by_type(player, tai->args_[0]) != NULL)
        return -1;
      break;
    case IF_DAILY_TASK_DONE_CNT_LT:
      if (player->daily_clean_info_->daily_task_done_cnt_ >= tai->args_[0])
        return -1;
      break;
    case IF_GUILD_TASK_DONE_CNT_LT:
      if (player->daily_clean_info_->guild_task_done_cnt_ >= tai->args_[0])
        return -1;
      break;
    default:
      e_log->error("unkonwn task if type[%d]", tai->type_);
      return -1;
    }
  }
  return 0;
}
//= 'do` ai
int task_module::do_task_do_ai(player_obj *player,
                               task_info *tinfo,
                               task_ai_list_t *p_ai_list)
{
  if (p_ai_list == NULL) return 0;
  task_ai_list_itor itor = p_ai_list->head();
  for (; itor != NULL; itor = itor->next_)
  {
    int result = 0;
    task_ai_info *tai = itor->value_;
    switch (tai->type_)
    {
    case DO_GIVE_ITEM:
      result = task_module::do_give_item(player,
                                         tinfo->task_cid_,
                                         tai->args_[0],
                                         tai->args_[1],
                                         tai->args_[2]);
      break;
    case DO_GIVE_EXP:
      {
        mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
        *mb << tai->args_[0];
        player->post_aev(AEV_GOT_EXP, mb);
      }
      break;
    case DO_GIVE_MONEY:
      if (!player->is_money_upper_limit(tai->args_[1], tai->args_[0]))
        player->do_got_money(tai->args_[0], tai->args_[1], MONEY_GOT_TASK, tinfo->task_cid_);
      break;
    case DO_GIVE_GUILD_CONTRIB:
      if (player->guild_id() != 0)
        guild_module::do_got_contrib(player->guild_id(), player->id(), tai->args_[0]);
      break;
    case DO_GIVE_GUILD_MONEY:
      if (player->guild_id() != 0)
        guild_module::do_got_resource(player->guild_id(), tai->args_[0]);
      break;
    case DO_AUTO_ACCEPT_TASK:
      if (player->task_data_->task_info_map_.find(tai->args_[0]) == player->task_data_->task_info_map_.end()
          && !task_module::is_in_acceptable_list(player, tai->args_[0]))
      {
        const task_cfg_obj *tco = task_config::instance()->find(tai->args_[0]);
        if (tco != NULL)
          task_module::do_accept_common_task(player, tco);
      }
      break;
    case DO_RANDOM_ACCEPT_TASK:
      task_module::do_random_accept_task(player, tai->args_[0]);
      break;
    default:
      e_log->error("unkonwn task do type[%d]", tai->type_);
      break;
    }
    if (result != 0)
      s_log->debug("char do task do_ai %d failed! ret = %d",
                   tai->type_,
                   result);
  }
  return 0;
}
int task_module::do_give_item(player_obj *player,
                              const int task_cid,
                              const int item_cid,
                              const int amount,
                              const char bind)
{
  if (item_config::item_is_money(item_cid))
  {
    player->do_got_item_money(item_cid,
                              amount,
                              MONEY_GOT_TASK,
                              0);
  }else
  {
    int real_cid = item_cid;
    if (clsid::is_char_equip(item_cid))
      real_cid = clsid::get_equip_cid_by_career(player->career(), item_cid);
    package_module::do_insert_item(player,
                                   PKG_PACKAGE,
                                   real_cid,
                                   amount,
                                   bind,
                                   ITEM_GOT_TASK,
                                   task_cid,
                                   0);
  }
  return 0;
}
void task_module::do_random_accept_task(player_obj *player, const int task_type)
{
  if (task_type == DAILY_TASK)
  {
    if (player->daily_clean_info_->daily_task_done_cnt_
        >= task_config::daily_task_done_cnt())
      return ;
  }else if (task_type == GUILD_TASK)
  {
    if (player->daily_clean_info_->guild_task_done_cnt_
        >= task_config::guild_task_done_cnt())
      return ;
  }
  int random_cid = task_guide_cfg::instance()->get_random_task(task_type, player->lvl());
  if (random_cid == 0) return ;
  if (player->task_data_->task_info_map_.find(random_cid) == player->task_data_->task_info_map_.end()
      && !task_module::is_in_acceptable_list(player, random_cid))
  {
    const task_cfg_obj *tco = task_config::instance()->find(random_cid);
    if (tco != NULL)
      task_module::do_accept_common_task(player, tco);
  }
}
//= 'to` ai
void task_module::do_build_all_task_todo_info(player_obj *player)
{
  task_info_map_itor itor = player->task_data_->task_info_map_.begin();
  for (; itor != player->task_data_->task_info_map_.end(); ++itor)
  {
    task_info *tinfo = itor->second;
    const task_cfg_obj *tco = task_config::instance()->find(tinfo->task_cid_);
    if (tco == NULL) continue ;

    task_module::do_task_to_ai(player,
                               tco->task_cid_,
                               tco->target_to_ai_);

    tinfo->dirty(false);
    if (task_module::do_check_task_todo_list(player,
                                             tco->target_to_ai_,
                                             tinfo) == 0)
    {
      if (tinfo->state() != TASK_ST_COMPLETED)
        tinfo->state(TASK_ST_COMPLETED);
    }else
    {
      if (tinfo->state() == TASK_ST_COMPLETED)
        tinfo->state(TASK_ST_INPROGRESS);
    }

    if (tinfo->dirty())
      task_module::do_update_task_info_2_db(player,
                                            REQ_UPDATE_TASK,
                                            tinfo);
  }
}
int task_module::do_task_to_ai(player_obj *player,
                               const int task_cid,
                               task_ai_list_t *p_ai_list)
{
  if (p_ai_list == NULL) return 0;
  int result = 0;
  task_ai_list_itor itor = p_ai_list->head();
  for (; itor != NULL; itor = itor->next_)
  {
    task_ai_info *tai = itor->value_;
    switch (tai->type_)
    {
    case TO_KILL_MST:
    case TO_FINISH_MISSION:
    case TO_FINISH_GUIDE:
      result = task_module::do_add_task_todo_info(player,
                                                  task_cid,
                                                  tai->type_,
                                                  tai->args_[0],
                                                  0);
      break;
    case TO_FINISH_STAR_MISSION:
      result = task_module::do_add_task_todo_info(player,
                                                  task_cid,
                                                  tai->type_,
                                                  tai->args_[0],
                                                  tai->args_[1]);
      break;
    case TO_UP_LEVEL:
    case TO_KILL_BOSS:
    case TO_KILL_CHAR:
    case TO_EQUIP_STRENGTHEN:
    case TO_UPGRADE_PSKILL:
    case TO_GUAN_GAI:
    case TO_MO_BAI:
    case TO_JING_JI:
    case TO_ADD_FRIENDS:
      result = task_module::do_add_task_todo_info(player,
                                                  task_cid,
                                                  tai->type_,
                                                  0,
                                                  0);
      break;
    default:
      e_log->error("unkonwn task to type[%d]", tai->type_);
      break;
    }
  }
  return 0;
}
int task_module::do_add_task_todo_info(player_obj *player,
                                       const int task_cid,
                                       const int type,
                                       const int arg_0,
                                       const int arg_1)
{
  task_todo_info *tti = task_todo_info_pool::instance()->alloc();
  tti->type_      = type;
  tti->task_cid_  = task_cid;
  tti->arg_0_     = arg_0;
  tti->arg_1_     = arg_1;
  player->task_data_->task_todo_list_.push_back(tti);
  return 0;
}
void task_module::do_del_task_todo_info(player_obj *player, const int task_cid)
{
  int size = player->task_data_->task_todo_list_.size();
  for (int i = 0; i < size; ++i)
  {
    task_todo_info *tti = player->task_data_->task_todo_list_.pop_front();
    if (tti->task_cid_ == task_cid)
      task_todo_info_pool::instance()->release(tti);
    else
      player->task_data_->task_todo_list_.push_back(tti);
  }
}
int task_module::do_check_task_todo_list(player_obj *player,
                                         task_ai_list_t *p_ai_list,
                                         task_info *tinfo)
{
  if (p_ai_list == NULL) return 0;
  task_ai_list_itor itor = p_ai_list->head();
  for (; itor != NULL; itor = itor->next_)
  {
    task_ai_info *tai = itor->value_;
    switch (tai->type_)
    {
    case TO_KILL_MST:
    case TO_FINISH_MISSION:
    case TO_FINISH_GUIDE:
      if (tai->args_[1] > 0
          && tinfo->value(tai->idx_) < tai->args_[1])
        return -1;
      break;
    case TO_FINISH_STAR_MISSION:
      if (tai->args_[2] > 0
          && tinfo->value(tai->idx_) < tai->args_[2])
        return -1;
      break;
    case TO_KILL_BOSS:
    case TO_KILL_CHAR:
    case TO_EQUIP_STRENGTHEN:
    case TO_UPGRADE_PSKILL:
    case TO_GUAN_GAI:
    case TO_MO_BAI:
    case TO_JING_JI:
    case TO_ADD_FRIENDS:
      if (tai->args_[0] > 0
          && tinfo->value(tai->idx_) < tai->args_[0])
        return -1;
      break;
    case TO_UP_LEVEL:
      if (player->lvl() < tai->args_[0]) return -1;
      break;
    default:
      e_log->error("unkonwn task %d to type[%d]", tinfo->task_cid_, tai->type_);
      break;
    }
  }
  return 0;
}
int task_module::do_update_task_todo_info(player_obj *,
                                          task_ai_list_t *p_ai_list,
                                          task_info *tinfo,
                                          const int type,
                                          const int arg_0,
                                          const int arg_1,
                                          const int /*arg_2*/ /*= 0*/)
{
  if (p_ai_list == NULL) return 0;
  int ret = -1;
  task_ai_list_itor itor = p_ai_list->head();
  for (; itor != NULL; itor = itor->next_)
  {
    task_ai_info *tai = itor->value_;
    int v = 0;
    switch (tai->type_)
    {
    case TO_KILL_MST:
    case TO_FINISH_MISSION:
    case TO_FINISH_GUIDE:
      if (tai->type_ == type && tai->args_[0] == arg_0)
      {
        v = tinfo->value(tai->idx_);
        if (v < tai->args_[1])
        {
          tinfo->value(tai->idx_, v + 1);
          ret = 0;
        }
      }
      break;
    case TO_FINISH_STAR_MISSION:
      if (tai->type_ == type
          && tai->args_[0] == arg_0
          && tai->args_[1] == arg_1)
      {
        v = tinfo->value(tai->idx_);
        if (v < tai->args_[2])
        {
          tinfo->value(tai->idx_, v + 1);
          ret = 0;
        }
      }
      break;
    case TO_KILL_BOSS:
    case TO_KILL_CHAR:
    case TO_EQUIP_STRENGTHEN:
    case TO_UPGRADE_PSKILL:
    case TO_GUAN_GAI:
    case TO_MO_BAI:
    case TO_JING_JI:
    case TO_ADD_FRIENDS:
      if (tai->type_ == type)
      {
        v = tinfo->value(tai->idx_);
        if (v < tai->args_[0])
        {
          tinfo->value(tai->idx_, v + 1);
          ret = 0;
        }
      }
      break;
    case TO_UP_LEVEL:
      if (tai->type_ == type)
        ret = 0;
      break;
    default:
      e_log->error("unkonwn task to type[%d] [%s][arg_1 = %d]",
                   tai->type_,
                   __func__,
                   arg_1);
      break;
    }
  }

  return ret;
}
int task_module::do_update_task_todo_list(player_obj *player,
                                          const int type,
                                          const int arg_0,
                                          const int arg_1,
                                          const int arg_2/*= 0*/)
{
  task_todo_list_itor itor = player->task_data_->task_todo_list_.head();
  for (; itor != NULL; itor = itor->next_)
  {
    task_todo_info *tti = itor->value_;
    if (tti->type_ != type
        || tti->arg_0_ != arg_0
        || tti->arg_1_ != arg_1)
      continue;

    const task_cfg_obj* tco = task_config::instance()->find(tti->task_cid_);
    if (tco == NULL) continue;

    task_info_map_itor itor = player->task_data_->task_info_map_.find(tti->task_cid_);
    if (itor == player->task_data_->task_info_map_.end()) continue;

    task_info *tinfo = itor->second;
    tinfo->dirty(false);

    if (task_module::do_update_task_todo_info(player,
                                              tco->target_to_ai_,
                                              tinfo,
                                              type,
                                              arg_0,
                                              arg_1,
                                              arg_2) == 0)
    {
      task_module::on_update_task_todo_info_ok(player, tinfo, tco);
    }
  }
  return 0;
}
void task_module::on_update_task_todo_info_ok(player_obj *player,
                                              task_info *tinfo,
                                              const task_cfg_obj *tco)
{
  if (task_module::do_check_task_todo_list(player,
                                           tco->target_to_ai_,
                                           tinfo) != 0)
  {
    if (tinfo->state() == TASK_ST_COMPLETED)
      tinfo->state(TASK_ST_INPROGRESS);
  }else
  {
    if (tinfo->state() != TASK_ST_COMPLETED)
      tinfo->state(TASK_ST_COMPLETED);
  }

  if (tinfo->dirty())
  {
    task_module::do_update_task_info_2_db(player, REQ_UPDATE_TASK, tinfo);
    task_module::do_update_task_info_2_client(player,
                                              TASK_UPDATE,
                                              tinfo);
  }
}
