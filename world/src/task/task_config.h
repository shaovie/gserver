// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-03-01 10:45
 */
//========================================================================

#ifndef TASK_CONFIG_H_
#define TASK_CONFIG_H_

#include "def.h"

// Lib header
#include <cstring>
#include "ilist.h"
#include "singleton.h"

#define TASK_CFG_PATH                     "task.json"
#define TASK_GUIDE_CFG_PATH               "task_guide.json"

// Forward declarations
class task_config_impl;
class task_guide_cfg_impl;

enum
{
  TRUNK_TASK                    = 1, // 主线任务
  BRANCH_TASK                   = 2, // 支线任务
  DAILY_TASK                    = 3, // 日常任务
  GUILD_TASK                    = 4, // 公会任务
  TASK_TYPE_CNT
};
#define IS_ONLY_ONCE_TASK(N)    ((TASK_TYPE(N) == TRUNK_TASK)||(TASK_TYPE(N) == BRANCH_TASK))
#define TASK_TYPE(N)            (N/100000%100)
#define TASK_BIT_IDX(N)         (N%1000)
#define MAX_CMD_ARGS            3

enum task_ai_if_xx_t
{
  IF_IN_LEVEL                   = 1, // if_in_level(1, 100)
  IF_IN_GUILD                   = 2, // if_in_guild()
  IF_DAILY_TASK_DONE_CNT_LT     = 3, // if_daily_task_done_cnt_lt(number)
  IF_GUILD_TASK_DONE_CNT_LT     = 4, // if_guild_task_done_cnt_lt(number)
  IF_NO_TASK                    = 5, // if_no_task(type)
  IF_XXX_CNT
};
enum task_ai_do_xxt
{
  DO_GIVE_ITEM                  = 1, // do_give_item(item_cid,amount,bind)
  DO_GIVE_EXP                   = 2, // do_give_exp(number)
  DO_GIVE_MONEY                 = 3, // do_give_money(number,type)
  DO_GIVE_GUILD_CONTRIB         = 4, // do_give_guild_contrib(number)
  DO_GIVE_GUILD_MONEY           = 5, // do_give_guild_money(number)
  DO_AUTO_ACCEPT_TASK           = 6, // do_auto_accept_task(task_cid)
  DO_RANDOM_ACCEPT_TASK         = 7, // do_random_accept_task(type)

  DO_XXX_CNT
};
enum task_ai_to_xxt
{
  TO_KILL_MST                   = 1, // to_kill_mst(mst_cid,amount)
  TO_UP_LEVEL                   = 2, // to_up_lvl(number)
  TO_FINISH_MISSION             = 3, // to_finish_mission(scene_cid)
  TO_FINISH_GUIDE               = 4, // to_finish_guide(id)
  TO_KILL_BOSS                  = 5, // to_kill_boss(amount)
  TO_KILL_CHAR                  = 6, // to_kill_char(amount)
  TO_FINISH_STAR_MISSION        = 7, // to_finish_star_mission(scene_cid, star)
  TO_EQUIP_STRENGTHEN           = 8, // to_equip_strengthen(number)
  TO_UPGRADE_PSKILL             = 9, // to_upgrade_pskill(number)
  TO_GUAN_GAI                   = 10,// to_guan_gai(number)
  TO_MO_BAI                     = 11,// to_mo_bai(number)
  TO_JING_JI                    = 12,// to_jing_ji(number)
  TO_ADD_FRIENDS                = 13,// to_add_friends(number)

  TO_XXX_CNT
};
class task_ai_info
{
public:
  task_ai_info() : type_(-1), idx_(0)
  { ::memset(this->args_, 0, sizeof(this->args_)); }
  int type_;        // include if_xx to_xx do_xx
  int args_[MAX_CMD_ARGS];
  int idx_;         // 在命令串中排第几
};

typedef ilist<task_ai_info *> task_ai_list_t;
typedef ilist_node<task_ai_info *> *task_ai_list_itor;
/**
 * @class task_cfg_obj
 *
 * @brief
 */
class task_cfg_obj
{
public:
  task_cfg_obj(const int);
  ~task_cfg_obj();
public:
  int task_cid_;

  int pre_task_cid_;                   // 前置任务ID
  int accept_npc_cid_;
  int sub_npc_cid_;

  task_ai_list_t *accept_if_ai_;       // 任务接受条件判断
  task_ai_list_t *accept_do_ai_;       // 任务接受时执行AI
  task_ai_list_t *target_to_ai_;       // 任务执行过程中执行AI
  task_ai_list_t *complete_do_ai_;     // 任务完成时执行AI
  task_ai_list_t *award_do_ai_;        // 任务奖励AI
};
/**
 * @class task_config
 *
 * @brief
 */
class task_config : public singleton<task_config>
{
  friend class singleton<task_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const task_cfg_obj* find(const int task_id);

  ilist<int> &get_all_task_list();

  static int daily_task_done_cnt();
  static int guild_task_done_cnt();
private:
  task_config();
  task_config(const task_config &);
  task_config& operator= (const task_config &);

  task_config_impl *impl_;
};
/**
 * @class task_guide_cfg
 *
 * @brief
 */
class task_guide_cfg : public singleton<task_guide_cfg>
{
  friend class singleton<task_guide_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int get_random_task(const int type, const short lvl);
private:
  task_guide_cfg();
  task_guide_cfg(const task_guide_cfg &);
  task_guide_cfg& operator= (const task_guide_cfg &);

  task_guide_cfg_impl *impl_;
};
#endif  // TASK_CONFIG_H_
