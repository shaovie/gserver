// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-06-30 16:48
 */
//========================================================================

#ifndef PRODUCT_CFG_H_
#define PRODUCT_CFG_H_

#include "singleton.h"
#include "ilist.h"
#include "def.h"

#define PRODUCT_CFG_PATH              "produce.json"

// Forward declarations
class product_cfg_impl;

class product
{
public:
  product()
   : notify_(false), bind_(0), expend_(0)
  { }
public:
  bool notify_;                 //是否公告
  char bind_;
  int  expend_;                 //生产一次的消费
  ilist<pair_t<int> > material_;
};

/**
 * @class product_cfg
 * 
 * @brief
 */
class product_cfg : public singleton<product_cfg>
{
  friend class singleton<product_cfg>;

public:
  // return 0 on success otherwise -1.
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const product* get_product(const int item_cid);
private:
  product_cfg();

  product_cfg_impl *impl_;
};

#endif // PRODUCT_CFG_H_

