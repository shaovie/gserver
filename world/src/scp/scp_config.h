// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-06 16:21
 */
//========================================================================

#ifndef SCP_CONFIG_H_
#define SCP_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class scp_config_impl;

#define SCP_CFG_PATH                                "scp.json"

/**
 * @class sub_scp_cfg_obj
 * 
 * @brief
 */
class sub_scp_cfg_obj
{
public:
  sub_scp_cfg_obj()
    : enter_x_(0),
    enter_y_(0),
    scene_cid_(0),
    last_time_(0)
  { }

  short enter_x_;
  short enter_y_;

  int scene_cid_;
  int last_time_;
};
/**
 * @class scp_cfg_obj
 * 
 * @brief
 */
class scp_cfg_obj
{
public:
  enum
  {
    SCP_SINGLE         = 1,
    SCP_TEAM           = 2,
    SCP_FACTION        = 3,
  };
  enum
  {
    SCP_CLT_IMPL       = 1,
  };
  scp_cfg_obj()
    : is_valid_(true),
    enter_cnt_(0),
    scp_type_(0),
    control_type_(SCP_CLT_IMPL),
    scene_cid_(0)
  { }
  ~scp_cfg_obj();

  bool is_valid_;
  int enter_cnt_;

  int scp_type_;
  int control_type_;
  int scene_cid_;
  ilist<sub_scp_cfg_obj *> sub_scp_list_;
};
/**
 * @class scp_config
 * 
 * @brief
 */
class scp_config : public singleton<scp_config>
{
  friend class singleton<scp_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  scp_cfg_obj *get_scp_cfg_obj(const int scene_cid);
  coord_t get_enter_pos(const int scene_cid);
  int get_scp_enter_times(const int scene_cid);
  int control_type(const int scene_cid);
  int scp_type(const int scene_cid);
  int last_time(const int master_cid, const int scene_cid);
private:
  scp_config_impl *impl_;

  scp_config();
};

#endif // SCP_CONFIG_H_

