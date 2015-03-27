#include "lue_duo_config.h"
#include "sys_log.h"
#include "load_json.h"
#include "ilist.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class lue_duo_cfg_impl
 *
 * @brief
 */
class lue_duo_cfg_impl : public load_json
{
public:
  lue_duo_cfg_impl() :
    all_rate_(0)
  { }
  ~lue_duo_cfg_impl()
  {
    while (!this->ld_item_list_.empty())
      delete this->ld_item_list_.pop_front();
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LUE_DUO_CONFIG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      ld_obj *ldo = new ld_obj();
      this->ld_item_list_.push_back(ldo);

      if (this->load_ld_obj(ldo, *iter) != 0)
        return -1;
    }
    return 0;
  }

  int load_ld_obj(ld_obj *ldo, Json::Value &root)
  {
    ldo->item_cid_ = root["item_cid"].asInt();
    ldo->item_cnt_ = root["item_count"].asInt();
    ldo->rate_     = root["item_rate"].asInt();
    this->all_rate_ += ldo->rate_;

    if (ldo->item_cnt_ <= 0 || this->all_rate_ <= 0)
      return -1;
    return 0;
  }

  ld_obj *get_random_ld_obj()
  {
    int rate = rand() % this->all_rate_ + 1;
    for (ilist_node<ld_obj *> *itor = this->ld_item_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      if (rate <= itor->value_->rate_)
        return itor->value_;
      rate -= itor->value_->rate_;
    }
    return NULL;
  }
private:
  int all_rate_;
  ilist<ld_obj *> ld_item_list_;
};
lue_duo_config::lue_duo_config() : impl_(new lue_duo_cfg_impl()) { }
int lue_duo_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int lue_duo_config::reload_config(const char *cfg_root)
{
  lue_duo_cfg_impl *tmp_impl = new lue_duo_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LUE_DUO_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ld_obj* lue_duo_config::get_random_ld_obj()
{ return this->impl_->get_random_ld_obj(); }
