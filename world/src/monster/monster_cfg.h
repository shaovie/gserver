// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-10-08 14:30
 */
//========================================================================

#ifndef MONSTER_CONFIG_H_
#define MONSTER_CONFIG_H_

// Lib header
#include <string.h>
#include "obj_pool.h"
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define MONSTER_CFG_PATH                   "monster.json"

enum MONSTER_TYPE_T
{
  MST_COMMON                 = 0,  // 普通怪
  MST_ZHU_ZAI_JING_XIANG     = 1,  // 主宰镜像
  MST_GHZ_WANG_ZUO           = 2,  // 公会战冰封王座
  MST_GHZ_SHOU_WEI           = 3,  // 公会战通灵塔
  MST_GUILD_SHEN_SHOU        = 4,  // 公会神兽
  MST_GUILD_SCP_NO_HATE_MST  = 5,  // 公会副本没有仇恨怪
  MST_GUILD_SCP_HAD_HATE_MST = 6,  // 公会副本有仇恨怪
  MST_TU_TENG_TO_PLAYER      = 7,  // 给角色加血的图腾
  MST_XSZC_MAIN              = 8,  // 雄狮战场主建筑
  MST_XSZC_BARRACK           = 9,  // 雄狮战场兵营
  MST_XSZC_DEFENDER          = 10, // 雄狮战场防御塔
  MST_XSZC_ARM               = 11, // 雄狮战场小兵
  MST_END
};
enum mst_angle_t
{
  MST_ANGLE_GLOBAL        = 1,
  MST_ANGLE_1_4           = 2,
};
enum monster_sort_t
{
  MST_SORT_COMMON         = 0,
  MST_SORT_BOSS           = 1,
};
enum ai_if_xx_t
{
  IF_HP_LT                = 1, // if_sm_lt(number)
  IF_TRUE                 = 2, // if_true()
  IF_TIME_AT              = 3, // if_time_at(number)

  AI_IF_XX_CNT
};
enum ai_do_xx_t
{
  DO_USE_SKILL = 0,           // do_use_skill(cid, lvl)
  DO_LEARN_SKILL,             // do_learn_skill(cid,lvl,rate)
  DO_CALL_MST,                // do_call_mst(cid,area_radius)
  DO_RECOVER_SM,              // do_recover_sm(percent)

  AI_DO_XX_CNT
};
class ai_info
{
public:
  ai_info() { reset(); }
  void reset()
  {
    this->done_    = false;
    this->if_type_ = AI_IF_XX_CNT;
    this->do_type_ = -1;
    this->if_arg_  = 0;
    ::memset(this->do_arg_, 0, sizeof(this->do_arg_));
  }
  bool done_;
  char if_type_;
  char do_type_;
  int  if_arg_;
  int  do_arg_[3];
};

// Forward declarations
class monster_cfg_impl;

/**
 * @class monster_cfg_obj
 *
 * @brief
 */
class monster_cfg_obj
{
public:
  monster_cfg_obj();

  ~monster_cfg_obj();
public:
  bool  change_target_;
  char  career_;
  char  block_radius_;
  char  att_type_;
  char  sort_;
  short lvl_;
  short mst_type_;
  short patrol_radius_;
  short patrol_speed_;
  short eye_angle_;
  short eye_radius_;
  short chase_radius_;
  short chase_speed_;
  int  cid_;
  int  base_skill_;
  int  exp_;
  int  common_cd_;
  int  fight_back_delay_;
  int  patrol_interval_;
  int  refresh_time_;
  int  stiff_my_time_;
  int  live_time_;
  int  hp_;
  int  gong_ji_;
  int  fang_yu_;
  int  param_;

  ilist<ai_info *> birth_ai_list_;
  ilist<ai_info *> attack_ai_list_;
  ilist<ai_info *> back_ai_list_;
  ilist<ai_info *> die_ai_list_;
};

/**
 * @class monster_cfg
 *
 * @brief
 */
class monster_cfg : public singleton<monster_cfg>
{
  friend class singleton<monster_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  monster_cfg_obj *get_monster_cfg_obj(const int cid);
private:
  monster_cfg();
  monster_cfg(const monster_cfg &);
  monster_cfg& operator= (const monster_cfg &);

  monster_cfg_impl *impl_;
};

#endif  // MONSTER_CONFIG_H_
