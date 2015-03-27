#include "zhan_xing_cfg.h"
#include "sys_log.h"
#include "load_json.h"
#include "ilist.h"
#include "array_t.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class zx_item_list
 *
 * @brief
 */
class zx_item_list
{
public:
  zx_item_list() :
    all_rate_(0)
  { }
  ~zx_item_list()
  {
    while (!this->zx_obj_list_.empty())
    {
      zx_obj *zo = this->zx_obj_list_.pop_front();
      delete zo;
    }
  }

  zx_obj *get_random_zx_obj()
  {
    int rate = rand() % this->all_rate_ + 1;
    for (ilist_node<zx_obj *> *itor = this->zx_obj_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      if (rate <= itor->value_->rate_)
        return itor->value_;
      rate -= itor->value_->rate_;
    }
    return NULL;
  }

  int all_rate_;
  int cost_;
  ilist<zx_obj *> zx_obj_list_;
};

/**
 * @class zhan_xing_cfg_impl
 *
 * @brief
 */
class zhan_xing_cfg_impl : public load_json
{
public:
  zhan_xing_cfg_impl() :
    zx_type_map_(ZHAN_XING_TYPE_CNT)
  { }
  ~zhan_xing_cfg_impl()
  {
    for (int i = ZHAN_XING_PRIMARY; i < this->zx_type_map_.size(); ++i)
    {
      zx_item_list *zil = this->zx_type_map_.find(i);
      if (zil != NULL) delete zil;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, ZHAN_XING_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int type = ::atoi(iter.key().asCString());
      if (type >= ZHAN_XING_TYPE_CNT)
        return -1;
      Json::Value &info = (*iter)["info"];
      if (info.empty()) continue;

      zx_item_list *zil = new zx_item_list();
      this->zx_type_map_.insert(type, zil);
      zil->cost_ = (*iter)["count"].asInt();

      if (this->load_zx_item_list(zil, info) != 0)
        return -1;
    }
    return 0;
  }

  int load_zx_item_list(zx_item_list *zil, Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      zx_obj *zo = new zx_obj();
      zil->zx_obj_list_.push_back(zo);

      zo->item_cid_ = (*iter)["item_cid"].asInt();
      zo->item_cnt_ = (*iter)["item_cnt"].asInt();
      zo->bind_     = (*iter)["bind"].asInt();
      zo->rate_     = (*iter)["item_rate"].asInt();
      zo->notify_   = (*iter)["notify"].asInt();

      zil->all_rate_ += zo->rate_;
    }

    if (zil->all_rate_ <= 0)
      return -1;

    return 0;
  }

  int get_zx_cost(const int type)
  {
    zx_item_list *zil = this->zx_type_map_.find(type);
    if (zil == NULL) return 0;
    return zil->cost_;
  }
  zx_obj *get_random_zx_obj(const int type)
  {
    zx_item_list *zil = this->zx_type_map_.find(type);
    if (zil == NULL) return NULL;
    return zil->get_random_zx_obj();
  }
private:
  array_t<zx_item_list *> zx_type_map_;
};
zhan_xing_cfg::zhan_xing_cfg() : impl_(new zhan_xing_cfg_impl()) { }
int zhan_xing_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int zhan_xing_cfg::reload_config(const char *cfg_root)
{
  zhan_xing_cfg_impl *tmp_impl = new zhan_xing_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", ZHAN_XING_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int zhan_xing_cfg::get_zx_cost(const int type)
{ return this->impl_->get_zx_cost(type); }
zx_obj* zhan_xing_cfg::get_random_zx_obj(const int type)
{ return this->impl_->get_random_zx_obj(type); }
//=====================================================================
/**
 * @class zhan_xing_ten_cfg_impl
 *
 * @brief
 */
class zhan_xing_ten_cfg_impl : public load_json
{
public:
  zhan_xing_ten_cfg_impl() :
    zx_ten_map_(ZHAN_XING_ONCE_COUNT + 1)
  { }
  ~zhan_xing_ten_cfg_impl()
  {
    for (int i = 0; i < this->zx_ten_map_.size(); ++i)
    {
      zx_item_list *zil = this->zx_ten_map_.find(i);
      if (zil != NULL) delete zil;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, ZHAN_XING_TEN_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int number = ::atoi(iter.key().asCString());
      if (number > ZHAN_XING_ONCE_COUNT)
        return -1;

      zx_item_list *zil = new zx_item_list();
      this->zx_ten_map_.insert(number, zil);

      if (this->load_zx_item_list(zil, *iter) != 0)
        return -1;
    }
    return 0;
  }

  int load_zx_item_list(zx_item_list *zil, Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      zx_obj *zo = new zx_obj();
      zil->zx_obj_list_.push_back(zo);

      zo->item_cid_ = (*iter)["item_cid"].asInt();
      zo->item_cnt_ = (*iter)["item_cnt"].asInt();
      zo->bind_     = (*iter)["bind"].asInt();
      zo->rate_     = (*iter)["item_rate"].asInt();
      zo->notify_   = (*iter)["notify"].asInt();

      zil->all_rate_ += zo->rate_;
    }

    if (zil->all_rate_ <= 0)
      return -1;

    return 0;
  }

  zx_obj *get_random_zx_obj(const int number)
  {
    zx_item_list *zil = this->zx_ten_map_.find(number);
    if (zil == NULL) return NULL;
    return zil->get_random_zx_obj();
  }
private:
  array_t<zx_item_list *> zx_ten_map_;
};
zhan_xing_ten_cfg::zhan_xing_ten_cfg() : impl_(new zhan_xing_ten_cfg_impl()) { }
int zhan_xing_ten_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int zhan_xing_ten_cfg::reload_config(const char *cfg_root)
{
  zhan_xing_ten_cfg_impl *tmp_impl = new zhan_xing_ten_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", ZHAN_XING_TEN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
zx_obj* zhan_xing_ten_cfg::get_random_zx_obj(const int number)
{ return this->impl_->get_random_zx_obj(number); }
