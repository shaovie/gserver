// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-22 12:43
 */
//========================================================================

#ifndef GIFT_CONFIG_H_
#define GIFT_CONFIG_H_

// Lib header
#include "singleton.h"

#include "ilist.h"

#define GIFT_CFG_PATH                            "gift.json"
#define GIFT_BYVIP_CFG_PATH                      "box_bykey.json"

// Forward declarations
class gift_config_impl;
class gift_byvip_config_impl;

/**
 * @class gift_info
 * 
 * @brief
 */
class gift_info
{
public:
  gift_info() :
    bind_(0),
    p_(0),
    item_cid_(0),
    min_amt_(0),
    max_amt_(0)
  { }
public:
  char bind_;
  short p_;            // 概率
  int item_cid_;
  int min_amt_;
  int max_amt_;
};
/**
 * @class gift_obj 
 *
 * @brief
 */
class gift_obj
{
public:
  ~gift_obj()
  {
    while (!this->gift_list_.empty())
      delete this->gift_list_.pop_front();
  }
public:
  ilist<gift_info *> gift_list_;
};
/**
 * @class gift_config
 * 
 * @brief
 */
class gift_config : public singleton<gift_config>
{
  friend class singleton<gift_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
  
  gift_obj *get_gift(const int gift_cid); 
private:
  gift_config();

  gift_config_impl *impl_;
};

/**
 * @class gift_byvip_config
 * 
 * @brief
 */
class gift_byvip_config : public singleton<gift_byvip_config>
{
  friend class singleton<gift_byvip_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);
  
  gift_obj *get_gift(const int gift_cid, const int vip); 
private:
  gift_byvip_config();

  gift_byvip_config_impl *impl_;
};
#endif // GIFT_CONFIG_H_
