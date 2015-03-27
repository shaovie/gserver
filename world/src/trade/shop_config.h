// -*- C++ -*-

//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-03-04 17:52
 */
//========================================================================

#ifndef SHOP_CONFIG_H_
#define SHOP_CONFIG_H_

#include "singleton.h"

#define SHOP_CONFIG_PATH                            "shop.json"

// Forward declarations
class shop_config_impl;

/**
 * @class goods_obj
 *
 * @brief
 */
class goods_obj
{
public:
  goods_obj() :
    bind_type_(0),
    price_type_(0),
    item_cid_(0),
    price_(0),
    limit_cnt_(0)
  { }

public:
  char bind_type_;
  char price_type_;
  int  item_cid_;
  int  item_cnt_;
  int  price_;
  int  limit_cnt_;
};
/**
 * @class shop_config
 *
 * @brief
 */
class shop_config: public singleton<shop_config>
{
  friend class singleton<shop_config>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const goods_obj *get_goods(const int npc_cid, const int group_id);
  int price_type(const int npc_cid, const int group_id);
private:
  shop_config();
  shop_config_impl *impl_;
};
#endif // SHOP_CONFIG_H_

