// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-10 15:09
 */
//========================================================================

#ifndef FA_BAO_CONFIG_H_
#define FA_BAO_CONFIG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"
#include "global_macros.h"

#define FA_BAO_DENG_JIE_CFG_PATH                   "fabao_dengjie.json"
#define FA_BAO_HE_CHENG_CFG_PATH                   "fabao_produce.json"

// Forward declarations
class fa_bao_dj_cfg_impl;
class fa_bao_he_cheng_cfg_impl;

/**
 * @class fa_bao_dj_cfg_obj 
 *
 * @brief
 */
class fa_bao_dj_cfg_obj
{
public:
  int hp_recover_;
  int mp_recover_;
  int add_extra_hurt_;
  int reduce_hurt_;
  int attr_rate_[ATTR_T_ITEM_CNT];
  int part_fa_bao_cid_[FA_BAO_PART_CNT + 1];
};
/**
 * @class fa_bao_dj_cfg
 *
 * @brief
 */
class fa_bao_dj_cfg : public singleton<fa_bao_dj_cfg>
{
  friend class singleton<fa_bao_dj_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  fa_bao_dj_cfg_obj *get_dj_info(const int dj);
private:
  fa_bao_dj_cfg();
  fa_bao_dj_cfg(const fa_bao_dj_cfg &);
  fa_bao_dj_cfg& operator= (const fa_bao_dj_cfg &);

  fa_bao_dj_cfg_impl *impl_;
};

class fa_bao_he_cheng_cfg_obj
{
public:
  int  cost_;
  ilist<pair_t<int> > material_;
};
/**
 * @class fa_bao_he_cheng_cfg
 *
 * @brief
 */
class fa_bao_he_cheng_cfg : public singleton<fa_bao_he_cheng_cfg>
{
  friend class singleton<fa_bao_he_cheng_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const fa_bao_he_cheng_cfg_obj *get_he_cheng_info(const int cid);
private:
  fa_bao_he_cheng_cfg();
  fa_bao_he_cheng_cfg(const fa_bao_he_cheng_cfg &);
  fa_bao_he_cheng_cfg& operator= (const fa_bao_he_cheng_cfg &);

  fa_bao_he_cheng_cfg_impl *impl_;
};

#endif // FA_BAO_CONFIG_H_

