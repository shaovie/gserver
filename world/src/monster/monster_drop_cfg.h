// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-10-12 14:30
 */
//========================================================================

#ifndef MONSTER_DROP_CFG_H_
#define MONSTER_DROP_CFG_H_

// Lib header
#include "singleton.h"

#include "def.h"
#include "ilist.h"

#define MONSTER_DROP_CFG_PATH               "drop.json"
#define DROP_BOX_CFG_PATH                   "box.json"

enum
{
  RANDOM_CALC_RATE     = 1,
  CAKE_CALC_RATE       = 2,
};

// Forward declarations
class monster_drop_cfg_impl;
class box_item_cfg_impl;

/**
 * @class monster_drop_obj
 *
 * @brief
 */
class monster_drop_obj
{
public:
  class _drop_box
  {
  public:
    _drop_box() : box_cid_(0), box_rate_(0) { }

    int box_cid_;
    int box_rate_;
    ilist<pair_t<int> > limit_time_list_;
  };

  monster_drop_obj() { }
  ~monster_drop_obj()
  {
    while(!this->drop_box_list_.empty())
      delete this->drop_box_list_.pop_front();
  }

  ilist<_drop_box *> drop_box_list_;
};

/**
 * @class monster_drop_cfg
 *
 * @brief
 */
class monster_drop_cfg : public singleton<monster_drop_cfg>
{
  friend class singleton<monster_drop_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const monster_drop_obj *get_monster_drop_obj(const int cid);
private:
  monster_drop_cfg();

  monster_drop_cfg_impl *impl_;
};

/**
 * @class box_item_obj
 *
 * @brief
 */
class box_item_obj
{
public:
  class _drop_item
  {
  public:
    _drop_item() :
      pick_notice_(false),
      bind_type_(0),
      box_type_(0),
      drop_rate_(0),
      min_cnt_(0),
      max_cnt_(0),
      item_cid_(0)
    { }

    bool pick_notice_;
    char bind_type_;
    char box_type_;
    short drop_rate_;
    int  min_cnt_;
    int  max_cnt_;
    int  item_cid_;
  };

  box_item_obj() : box_type_(0) { }
  ~box_item_obj()
  {
    while (!this->drop_item_list_.empty())
      delete this->drop_item_list_.pop_front();
  }

  char box_type_;
  ilist<_drop_item *> drop_item_list_;
};

/**
 * @class box_item_cfg
 *
 * @brief
 */
class box_item_cfg : public singleton<box_item_cfg>
{
  friend class singleton<box_item_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  const box_item_obj *get_box_item_obj(const int cid);
private:
  box_item_cfg();

  box_item_cfg_impl *impl_;
};

#endif  // MONSTER_DROP_CONFIG_H_
