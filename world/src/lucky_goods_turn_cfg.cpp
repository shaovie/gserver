#include "lucky_goods_turn_cfg.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class lucky_goods_turn_cfg_impl
 *
 * @brief
 */
class lucky_goods_turn_cfg_impl : public load_json
{
public:
  lucky_goods_turn_cfg_impl()
  { }
  ~lucky_goods_turn_cfg_impl()
  {
    while (!this->lucky_goods_turn_cfg_list_.empty())
      delete this->lucky_goods_turn_cfg_list_.pop_front();
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LUCKY_GOODS_TURN_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      lucky_goods_turn_cfg_obj *p = new lucky_goods_turn_cfg_obj();
      this->lucky_goods_turn_cfg_list_.push_back(p);

      p->notify_ = (*iter)["notify"].asBool();
      p->rate_ = (*iter)["item_rate"].asInt();
      p->award_item_.cid_ = (*iter)["item_cid"].asInt();
      p->award_item_.bind_ = (*iter)["bind"].asInt();
      p->award_item_.amount_ = (*iter)["item_cnt"].asInt();
      if (!item_config::instance()->find(p->award_item_.cid_))
        return -1;
    }
    return 0;
  }

  ilist<lucky_goods_turn_cfg_obj *> *get_cfg_list()
  { return &(this->lucky_goods_turn_cfg_list_); }
private:
  ilist<lucky_goods_turn_cfg_obj *> lucky_goods_turn_cfg_list_;
};
lucky_goods_turn_cfg::lucky_goods_turn_cfg() : impl_(new lucky_goods_turn_cfg_impl()) { }
int lucky_goods_turn_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int lucky_goods_turn_cfg::reload_config(const char *cfg_root)
{
  lucky_goods_turn_cfg_impl *tmp_impl = new lucky_goods_turn_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LUCKY_GOODS_TURN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<lucky_goods_turn_cfg_obj *> *lucky_goods_turn_cfg::get_cfg_list()
{ return this->impl_->get_cfg_list(); }
