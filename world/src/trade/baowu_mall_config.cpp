#include "baowu_mall_config.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"
#include "ilist.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

#define MAX_LVL_SECTION       32   

class baowu_info
{
public:
  baowu_info() :
    min_lvl_(0),
    max_lvl_(0),
    rand_range_(0)
  { }

  int load_json(Json::Value &root)
  {
    this->min_lvl_ = root["min_lvl"].asInt();
    this->max_lvl_ = root["max_lvl"].asInt();

    Json::Value &info = root["info"];
    if (info.empty()) return -1;

    for(Json::Value::iterator iter = info.begin();
        iter != info.end();
        ++iter)
    {
      pair_t<int> index_rate;
      index_rate.first_  = (*iter)["item_index"].asInt();
      index_rate.second_ = (*iter)["p"].asInt();

      this->rand_range_ += index_rate.second_;
      this->item_index_list_.push_back(index_rate);
    }
    if (this->rand_range_ == 0) return -1;

    return 0;
  }
  int get_item_index()
  {
    if (this->rand_range_ == 0) return 0;
    int r = rand() % this->rand_range_ + 1;
    for (ilist_node<pair_t<int> > *iter = this->item_index_list_.head();
         iter != NULL;
         iter = iter->next_)
    {
      if (r <= iter->value_.second_)
        return iter->value_.first_;

      r -= iter->value_.second_;
    }
    return 0;
  }
public:
  int min_lvl_;
  int max_lvl_;
  int rand_range_;
  ilist<pair_t<int> > item_index_list_;
};

/**
 * @class baowu_mall_config_impl
 *
 * @brief
 */
class baowu_mall_config_impl : public load_json
{
public:
  baowu_mall_config_impl() :
    baowu_info_map_(MAX_LVL_SECTION)
  { }
  ~baowu_mall_config_impl()
  {
    for (int i = 0; i < this->baowu_info_map_.size(); ++i)
    {
      baowu_info *bi = this->baowu_info_map_.find(i);
      if (bi != NULL) delete bi;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BAOWU_MALL_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int n = ::atoi(iter.key().asCString());
      if (n >= MAX_LVL_SECTION)
        return -1;

      baowu_info *bi = new baowu_info();
      this->baowu_info_map_.insert(n, bi);
      if (bi->load_json(*iter) != 0)
        return -1;
    }
    return 0;
  }
  int get_item_index(const int lvl)
  {
    for (int i = 0; i < this->baowu_info_map_.size(); ++i)
    {
      baowu_info *bi = this->baowu_info_map_.find(i);
      if (bi == NULL) continue;
      if (bi->min_lvl_ <= lvl && bi->max_lvl_ >= lvl)
        return bi->get_item_index();
    }

    return 0;
  }
private:
  array_t<baowu_info *> baowu_info_map_;
};
baowu_mall_config::baowu_mall_config() : impl_(new baowu_mall_config_impl()) { }
int baowu_mall_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int baowu_mall_config::reload_config(const char *cfg_root)
{
  baowu_mall_config_impl *tmp_impl = new baowu_mall_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", BAOWU_MALL_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int baowu_mall_config::get_item_index(const int lvl)
{ return this->impl_->get_item_index(lvl); }
