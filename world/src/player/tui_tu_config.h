// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-15 11:47
 */
//========================================================================

#ifndef TUI_TU_CONFIG_H_
#define TUI_TU_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define TUI_TU_CONFIG_PATH                    "mission.json"
#define TUI_TU_TURN_CONFIG_PATH               "mission_turn.json"
#define TUI_TU_CHAPTER_CONFIG_PATH            "mission_chapter.json"
#define TUI_TU_JY_CHAPTER_CONFIG_PATH         "elite_mission_chapter.json"

#define MAX_TUI_TU_TURN_ITEM_AMOUNT           4

// Forward declarations
class tui_tu_config_impl;
class tui_tu_turn_config_impl;
class tui_tu_chapter_config_impl;
class tui_tu_jy_chapter_config_impl;

class tui_tu_cfg_obj
{
public:
  short enter_x_;
  short enter_y_;
  int scene_cid_;
  int next_scene_cid_;
  int score_;
  int tili_cost_;
  int limit_time_;
  int first_exp_award_;
  int every_exp_award_;
  int boss_cid_;
  int turn_award_;
  int free_turn_cnt_;
  int turn_all_cost_;
  item_amount_bind_t first_item_award_;
};
/**
 * @class tui_tu_config
 * 
 * @brief
 */
class tui_tu_config : public singleton<tui_tu_config>
{
  friend class singleton<tui_tu_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  tui_tu_cfg_obj *get_tui_tu_cfg_obj(const int scene_cid);
  int ti_li_cost(const int scene_cid);
private:
  tui_tu_config();
  tui_tu_config(const tui_tu_config &);
  tui_tu_config& operator= (const tui_tu_config &);

  tui_tu_config_impl *impl_;
};

class tui_tu_chapter_cfg_obj
{
public:
  ilist<int> scene_cid_list_;
};
/**
 * @class tui_tu_chapter_config
 * 
 * @brief
 */
class tui_tu_chapter_config : public singleton<tui_tu_chapter_config>
{
  friend class singleton<tui_tu_chapter_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  tui_tu_chapter_cfg_obj *get_tui_tu_chapter_cfg_obj(const int idx);
  int get_chapter_idx(const int scene_cid);
private:
  tui_tu_chapter_config();
  tui_tu_chapter_config(const tui_tu_chapter_config &);
  tui_tu_chapter_config& operator= (const tui_tu_chapter_config &);

  tui_tu_chapter_config_impl *impl_;
};

/**
 * @class tui_tu_jy_chapter_config
 * 
 * @brief 精英关卡
 */
class tui_tu_jy_chapter_config : public singleton<tui_tu_jy_chapter_config>
{
  friend class singleton<tui_tu_jy_chapter_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  tui_tu_chapter_cfg_obj *get_tui_tu_chapter_cfg_obj(const int idx);
  int get_chapter_idx(const int scene_cid);
private:
  tui_tu_jy_chapter_config();
  tui_tu_jy_chapter_config(const tui_tu_jy_chapter_config &);
  tui_tu_jy_chapter_config& operator= (const tui_tu_jy_chapter_config &);

  tui_tu_jy_chapter_config_impl *impl_;
};

class tui_tu_turn_cfg_obj
{
public:
  int scene_cid_;
  ilist<pair_t<int> > box_list_;
};
/**
 * @class tui_tu_turn_config
 * 
 * @brief
 */
class tui_tu_turn_config : public singleton<tui_tu_turn_config>
{
  friend class singleton<tui_tu_turn_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  tui_tu_turn_cfg_obj *get_tui_tu_turn_cfg_obj(const int scene_cid);
private:
  tui_tu_turn_config();
  tui_tu_turn_config(const tui_tu_turn_config &);
  tui_tu_turn_config& operator= (const tui_tu_turn_config &);

  tui_tu_turn_config_impl *impl_;
};
#endif // TUI_TU_CONFIG_H_

