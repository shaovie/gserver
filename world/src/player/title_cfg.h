// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-04-21 15:27
 */
//========================================================================

#ifndef TITLE_CFG_H_
#define TITLE_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define TITLE_CFG_PATH                "title.json"

class title_cfg_impl;
class item_amount_bind_t;

/**
 * @class title_cfg_obj
 *
 * @brief
 */
class title_cfg_obj
{
public:
  title_cfg_obj();
  int attr_[ATTR_T_ITEM_CNT];
};

/**
 * @class title_cfg
 *
 * @brief
 */
class title_cfg : public singleton<title_cfg>
{
  friend class singleton<title_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  title_cfg_obj *get_title_cfg_obj(const int title_cid);
public:
  title_cfg();

  title_cfg_impl *impl_;
};

#endif  // TITLE_CFG_H_
