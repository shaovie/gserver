// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-01-22 18:34
 */
//========================================================================

#ifndef IU_CONFIG_H_
#define IU_CONFIG_H_

#include "def.h"
#include "ilist.h"
#include "array_t.h"

// Lib header
#include "singleton.h"

#define EQUIP_STRENGTHEN_CFG_PATH                 "equip_strengthen.json"
#define EQUIP_FEN_JIE_CFG_PATH                    "equip_break.json"
#define EQUIP_RONG_HE_CFG_PATH                    "equip_fusion.json"
#define EQUIP_RONG_HE_CFG_PATH                    "equip_fusion.json"
#define EQUIP_XI_LIAN_CFG_PATH                    "equip_wash.json"
#define XI_LIAN_RULE_CFG_PATH                     "wash_rule.json"
#define XI_LIAN_VALUE_CFG_PATH                    "wash_val.json"
#define EQUIP_JIN_JIE_CFG_PATH                    "equip_promote.json"

#define MAX_XI_LIAN_STAR                          10

// Forward declarations
class equip_strengthen_cfg_impl;
class equip_fen_jie_cfg_impl;
class equip_rong_he_cfg_impl;
class equip_xi_lian_config_impl;
class xi_lian_rule_config_impl;
class xi_lian_value_config_impl;
class equip_jin_jie_cfg_impl;

class equip_strengthen_cfg_obj
{
public:
  int cost_;
  int item_cid_;
  int item_cnt_;
  int probability_;
  int min_strengthen_cnt_;
  int min_bless_val_;
  int max_bless_val_;
  int total_bless_val_;
  int lucky_rate_;
};
/**
 * @class equip_strengthen_cfg
 * 
 * @brief
 */
class equip_strengthen_cfg : public singleton<equip_strengthen_cfg>
{
  friend class singleton<equip_strengthen_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const equip_strengthen_cfg_obj *get_strengthen_info(const short lvl);
private:
  equip_strengthen_cfg();
  equip_strengthen_cfg(const equip_strengthen_cfg &);
  equip_strengthen_cfg& operator= (const equip_strengthen_cfg &);

  equip_strengthen_cfg_impl *impl_;
};

class equip_fen_jie_cfg_obj
{
public:
  int cost_;
  int item_cid_;
  int item_cnt_;
};
/**
 * @class equip_fen_jie_cfg
 * 
 * @brief
 */
class equip_fen_jie_cfg : public singleton<equip_fen_jie_cfg>
{
  friend class singleton<equip_fen_jie_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const equip_fen_jie_cfg_obj *get_fen_jie_info(const int color);
private:
  equip_fen_jie_cfg();
  equip_fen_jie_cfg(const equip_fen_jie_cfg &);
  equip_fen_jie_cfg& operator= (const equip_fen_jie_cfg &);

  equip_fen_jie_cfg_impl *impl_;
};

class equip_rong_he_cfg_obj
{
public:
  int cost_;
  int item_cid_;
  int item_cnt_;
};
/**
 * @class equip_rong_he_cfg
 * 
 * @brief
 */
class equip_rong_he_cfg : public singleton<equip_rong_he_cfg>
{
  friend class singleton<equip_rong_he_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const equip_rong_he_cfg_obj *get_rong_he_info(const int color);
private:
  equip_rong_he_cfg();
  equip_rong_he_cfg(const equip_rong_he_cfg &);
  equip_rong_he_cfg& operator= (const equip_rong_he_cfg &);

  equip_rong_he_cfg_impl *impl_;
};

/**
 * @class equip_xi_lian_obj
 *
 * @brief
 */
class equip_xi_lian_obj
{
public:
  equip_xi_lian_obj()
    : item_cid_(0),
    item_cnt_(0),
    cost_(0),
    min_num_(0),
    max_num_(0)
  { }

  int item_cid_;
  int item_cnt_;
  int cost_;
  int min_num_;
  int max_num_;
};
/**
 * @class equip_xi_lian_config
 *
 * @brief
 */
class equip_xi_lian_config : public singleton<equip_xi_lian_config>
{
  friend class singleton<equip_xi_lian_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  equip_xi_lian_obj *get_equip_xi_lian_obj(const int color);
private:
  equip_xi_lian_config();

  equip_xi_lian_config_impl *impl_;
};
/**
 * @class xi_lian_rule_config
 *
 * @brief
 */
class xi_lian_rule_config : public singleton<xi_lian_rule_config>
{
  friend class singleton<xi_lian_rule_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int xl_cnt(const int star);
  int rate(const int star);
private:
  xi_lian_rule_config();

  xi_lian_rule_config_impl *impl_;
};

/**
 * @class xi_lian_value_obj
 *
 * @brief
 */
class xi_lian_value_obj
{
public:
  xi_lian_value_obj();
  ~xi_lian_value_obj();

  class _s_value
  {
  public:
    _s_value();

    int value_[MAX_XI_LIAN_STAR + 1];
  };

  array_t<_s_value *> s_value_info_;
};
/**
 * @class xi_lian_value_config
 *
 * @brief
 */
class xi_lian_value_config : public singleton<xi_lian_value_config>
{
  friend class singleton<xi_lian_value_config>;
public:
  int load_config(const char *cfg_root);

  int reload_config(const char *cfg_root);

  xi_lian_value_obj *get_xi_lian_value_obj(const int suit_cid);
private:
  xi_lian_value_config();

  xi_lian_value_config_impl *impl_;
};

class equip_jin_jie_cfg_obj
{
public:
  int cost_;
  int to_equip_cid_;
  ilist<pair_t<int> > cai_liao_info_; // first:cid second:amount
};
/**
 * @class equip_jin_jie_cfg
 * 
 * @brief
 */
class equip_jin_jie_cfg : public singleton<equip_jin_jie_cfg>
{
  friend class singleton<equip_jin_jie_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  equip_jin_jie_cfg_obj *get_jin_jie_info(const int equip_cid);
private:
  equip_jin_jie_cfg();
  equip_jin_jie_cfg(const equip_jin_jie_cfg &);
  equip_jin_jie_cfg& operator= (const equip_jin_jie_cfg &);

  equip_jin_jie_cfg_impl *impl_;
};
#endif // IU_CONFIG_H_

