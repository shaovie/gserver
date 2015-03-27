// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-27 12:13
 */
//========================================================================

#ifndef GUILD_CONFIG_H_
#define GUILD_CONFIG_H_

// Lib header
#include "singleton.h"

#define GUILD_LVL_CFG_PATH                        "gh_hall.json"
#define GUILD_POS_CFG_PATH                        "gh_position.json"
#define GUILD_SKILL_CFG_PATH                      "gh_skill.json"
#define GUILD_SCP_CFG_PATH                        "guild_fb.json"

#include "def.h"
#include "ilist.h"

// Forward declarations
class guild_lvl_config_impl;
class guild_pos_config_impl;
class guild_skill_config_impl;
class guild_scp_config_impl;

enum scp_difficulty
{
  SCP_DFF_PU_TONG    = 1,
  SCP_DFF_JING_YING  = 2,
  SCP_DFF_YING_XIONG = 3,
  SCP_DFF_END,
};

/**
 * @class guild_lvl_config
 *
 * @brief
 */
class guild_lvl_config : public singleton<guild_lvl_config>
{
  friend class singleton<guild_lvl_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  int  mem_capacity(const char lvl);
  int  dt_lvl_up_cost(const char lvl);
  int  zq_lvl_up_cost(const char lvl);
  int  tld_lvl_up_cost(const char lvl);
  int  jbd_lvl_up_cost(const char lvl);
  char free_day_jb_cnt(const char lvl);
  char max_day_jb_cnt(const char lvl);
  int  ju_bao_award_contrib(const char lvl);
  int  ju_bao_award_item(const char lvl);
  int  tu_long_dian_boss(const char lvl);
  int  summon_boss_cost(const char lvl);
  int  max_day_learn_skill_cnt(const char lvl);
  bool is_lvl_valid(const char lvl);
  int  max_skill_lvl(const char lvl);
  int  scp_lvl_up_cost(const char lvl);
  int  scp_award_per(const char lvl);
private:
  guild_lvl_config();
  guild_lvl_config_impl *impl_;
};
/**
 * @class guild_pos_config
 *
 * @brief
 */
class guild_pos_config : public singleton<guild_pos_config>
{
  friend class singleton<guild_pos_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  bool had_right_to_agree_join(const char pos);
  bool had_right_to_set_pos(const char tar_pos, const char pos);
  bool had_right_to_expel_mem(const char tar_pos, const char pos);
  bool had_right_to_set_apply(const char pos);
  bool had_right_to_set_purpose(const char pos);
  bool pos_full(const char pos, const char amt);
  bool had_right_to_summon_boss(const char pos);
  bool had_right_to_up_buliding_lvl(const char pos);
  bool had_right_to_open_scp(const char pos);
  bool had_right_to_summon_mem(const char pos);
private:
  guild_pos_config();
  guild_pos_config_impl *impl_;
};
/**
 * @class guild_skill_config
 *
 * @brief
 */
class guild_skill_config : public singleton<guild_skill_config>
{
  friend class singleton<guild_skill_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  int effect_id(const int skill_cid);
  int effect_val(const int skill_cid, const short skill_lvl);
  int min_zhan_qi_lvl(const int skill_cid);
  int learn_cost(const int skill_cid, const short skill_lvl);
private:
  guild_skill_config();
  guild_skill_config_impl *impl_;
};
/**
 * @class guild_scp_config
 *
 * @brief
 */
class guild_scp_config : public singleton<guild_scp_config>
{
  friend class singleton<guild_scp_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  ilist<coord_t> *mst_refresh_pos(const char dff, const char bo_shu);
  coord_t &target_pos();
  int over_all_time(const char dff, const char bo_shu);
  int over_all_time(const char dff, const char bo_shu, const char batch);
  int award_resource(const char dff, const char bo_shu);
  int award_contrib(const char dff, const char bo_shu);
  int refresh_mst_cid(const char dff, const char bo_shu, const char batch);
  int refresh_mst_amt(const char dff, const char bo_shu, const char batch);
private:
  guild_scp_config();
  guild_scp_config_impl *impl_;
};
#endif // GUILD_CONFIG_H_
