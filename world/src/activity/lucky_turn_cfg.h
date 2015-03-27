// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-23 18:21
 */
//========================================================================

#ifndef LUCKY_TURN_CFG_H_
#define LUCKY_TURN_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class lucky_turn_cfg_impl;
class lucky_turn_rank_award_cfg_impl;

#define LUCKY_TURN_CFG_PATH                "lucky_turn.json"
#define LUCKY_TURN_RANK_AWARD_CFG_PATH     "lucky_turn_rank_award.json"

class lucky_turn_cfg_obj
{
public:
  char notify_;
  int  rate_;
  item_amount_bind_t award_item_;
};
/**
 * @class lucky_turn_cfg
 *
 * @brief
 */
class lucky_turn_cfg : public singleton<lucky_turn_cfg>
{
  friend class singleton<lucky_turn_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<lucky_turn_cfg_obj *> * get_cfg_list();
private:
  lucky_turn_cfg();

  lucky_turn_cfg_impl *impl_;
};

class lucky_turn_rank_award
{
public:
  int  rank_min_;
  int  rank_max_;
  ilist<item_amount_bind_t> award_item_list_;
};
/**
 * @class lucky_turn_rank_award_cfg
 *
 * @brief
 */
class lucky_turn_rank_award_cfg : public singleton<lucky_turn_rank_award_cfg>
{
  friend class singleton<lucky_turn_rank_award_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  ilist<item_amount_bind_t> *get_award_list(const int rank);
private:
  lucky_turn_rank_award_cfg();

  lucky_turn_rank_award_cfg_impl *impl_;
};
#endif // LUCKY_TURN_CFG_H_

