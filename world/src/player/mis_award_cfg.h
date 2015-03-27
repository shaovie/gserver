// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-15 18:03
 */
//========================================================================

#ifndef MIS_AWARD_CFG_H_
#define MIS_AWARD_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define ONLINE_AWARD_CFG_PATH                        "online_award.json"
#define LOGIN_AWARD_CFG_PATH                         "login_award.json"
#define SEVEN_DAY_LOGIN_CFG_PATH                     "seven_day_login.json"

// Forward declarations
class online_award_cfg_impl;
class login_award_cfg_impl;
class seven_day_login_cfg_impl;

/**
 * @class online_award_cfg
 *
 * @brief
 */
class online_award_cfg : public singleton<online_award_cfg>
{
  friend class singleton<online_award_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  int get_award_min_time(const short lvl, const char get_idx);
  ilist<item_amount_bind_t> *award_list(const short lvl, const char get_idx);
private:
  online_award_cfg();
  online_award_cfg_impl *impl_;
};
/**
 * @class login_award_cfg
 *
 * @brief
 */
class login_award_cfg : public singleton<login_award_cfg>
{
  friend class singleton<login_award_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  ilist<item_amount_bind_t> *award_list(const char wday);
private:
  login_award_cfg();
  login_award_cfg_impl *impl_;
};
/**
 * @class seven_day_login_cfg
 *
 * @brief
 */
class seven_day_login_cfg : public singleton<seven_day_login_cfg>
{
  friend class singleton<seven_day_login_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  ilist<item_amount_bind_t> *award_list(const char day);
private:
  seven_day_login_cfg();
  seven_day_login_cfg_impl *impl_;
};
#endif // MIS_AWARD_CFG_H_
