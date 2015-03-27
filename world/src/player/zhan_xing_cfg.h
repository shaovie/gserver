// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-17 16:42
 */
//========================================================================

#ifndef ZHAN_XING_CFG_H_
#define ZHAN_XING_CFG_H_

// Lib header
#include "singleton.h"

#define ZHAN_XING_CFG_PATH                "zhan_xing.json"
#define ZHAN_XING_TEN_CFG_PATH            "zx_shi.json"

#define ZHAN_XING_ONCE_COUNT              10

// Forward declarations
class zhan_xing_cfg_impl;
class zhan_xing_ten_cfg_impl;

enum
{
  ZHAN_XING_PRIMARY    = 0,
  ZHAN_XING_SENIOR,

  ZHAN_XING_TYPE_CNT
};

/**
 * @class zx_obj
 *
 * @brief
 */
class zx_obj
{
public:
  zx_obj() :
    bind_(0),
    notify_(0),
    item_cid_(0),
    item_cnt_(0),
    rate_(0)
  { }

  char bind_;
  char notify_;
  int item_cid_;
  int item_cnt_;
  int rate_;
};

/**
 * @class zhan_xing_cfg
 *
 * @brief
 */
class zhan_xing_cfg : public singleton<zhan_xing_cfg>
{
  friend class singleton<zhan_xing_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int get_zx_cost(const int type);
  zx_obj *get_random_zx_obj(const int type);
private:
  zhan_xing_cfg();

  zhan_xing_cfg_impl *impl_;
};

/**
 * @class zhan_xing_ten_cfg
 *
 * @brief
 */
class zhan_xing_ten_cfg : public singleton<zhan_xing_ten_cfg>
{
  friend class singleton<zhan_xing_ten_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  zx_obj *get_random_zx_obj(const int number);
private:
  zhan_xing_ten_cfg();

  zhan_xing_ten_cfg_impl *impl_;
};

#endif  //ZHAN_XING_CFG_H_
