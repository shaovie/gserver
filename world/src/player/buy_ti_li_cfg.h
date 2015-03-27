// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-08-18 16:30
 */
//========================================================================

#ifndef BUY_TI_LI_CFG_H_
#define BUY_TI_LI_CFG_H_

// Lib header
#include "singleton.h"

#define TI_LI_PURCHASE_CFG_PATH                  "ti_li_purchase.json"

// Forward declarations
class buy_ti_li_cfg_impl;

/**
 * @class buy_ti_li_cfg
 * 
 * @brief
 */
class buy_ti_li_cfg : public singleton<buy_ti_li_cfg>
{
public:
  friend class singleton<buy_ti_li_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int buy_ti_li(const int cnt, int &diamond, int &ti_li_cnt);
private:
  buy_ti_li_cfg();
  buy_ti_li_cfg_impl *impl_;
};

#endif // BUY_TI_LI_CFG_H_

