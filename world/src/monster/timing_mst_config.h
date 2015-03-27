// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-12 16:38
 */
//========================================================================

#ifndef TIMING_MST_CONFIG_H_
#define TIMING_MST_CONFIG_H_

#include "def.h"

// Lib header
#include "ilist.h"
#include "singleton.h"

#define MAX_TIMING_MST_IDX            64

#define TIMING_MST_CONFIG_PATH        "timing_boss.json"

// Forward declarations
class timing_mst_cfg_obj
{
public:
  timing_mst_cfg_obj() :
    index_(0),
    coord_x_(0),
    coord_y_(0),
    range_(0),
    scene_cid_(0),
    mst_cid_(0)
  { }
public:
  char index_;
  short coord_x_;
  short coord_y_;
  short range_;
  int   scene_cid_;
  int   mst_cid_;
  ilist<int> month_list_;
  ilist<int> wday_list_;
  ilist<int> mday_list_;
  ilist<pair_t<char> > time_list_; // hour && min
};
class timing_mst_config_impl;
/**
 * @class timing_mst_config
 *
 * @brief
 */
class timing_mst_config : public singleton<timing_mst_config>
{
public:
  friend class singleton<timing_mst_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
public:
  ilist<timing_mst_cfg_obj *> *get_timing_mst_list(void);
private:
  timing_mst_config();
  timing_mst_config_impl *impl_;
};
#endif // TIMING_MST_CONFIG_H_
