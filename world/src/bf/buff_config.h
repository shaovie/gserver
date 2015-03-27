// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-01-08 13:56
 */
//========================================================================

#ifndef BUFF_CONFIG_H_
#define BUFF_CONFIG_H_

#include "def.h"
#include "buff_effect_id.h"

// Lib header
#include "singleton.h"

// Forward declarations
class buff_config_impl;

#define BUFF_CFG_PATH                              "buff.json"

class buff_cfg_obj
{
public:
  buff_cfg_obj() :
    died_clear_(false),
    offline_clear_(true),
    type_(0),
    show_type_(0),
    dj_cnt_(0),
    ctrl_type_(0),
    effect_id_(BF_SK_NULL),
    value_(0),
    last_(0),
    interval_(0),
    param_1_(0),
    param_2_(0),
    mian_yi_time_(0)
  { }

  bool died_clear_;
  bool offline_clear_;
  char type_;
  char show_type_;
  char dj_cnt_;
  char ctrl_type_;
  buff_effect_id_t  effect_id_;
  int  value_;
  int  last_;
  int  interval_;
  int  param_1_;
  int  param_2_;
  int  param_3_;
  int  mian_yi_time_;
};
/**
 * @class buff_config
 * 
 * @brief
 */
class buff_config : public singleton<buff_config>
{
  friend class singleton<buff_config>;
public:
  enum
  {
    TYPE_BUFF       = 1,
    TYPE_DEBUFF     = 2,
  };
  enum
  {
    SHOW_TO_HIDE    = 1,
    SHOW_TO_SELF    = 2,
    SHOW_TO_ALL     = 3,
  };
  enum
  {
    BF_CTRL_ATTR    = 1,
    BF_CTRL_MOVE    = 2,
  };
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  buff_cfg_obj *get_buff_cfg_obj(const int buff_id);
private:
  buff_config();
  buff_config(const buff_config &);
  buff_config& operator= (const buff_config &);

  buff_config_impl *impl_;
};

#endif // BUFF_CONFIG_H_

