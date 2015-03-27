#include "lucky_turn_cfg.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class lucky_turn_cfg_impl
 *
 * @brief
 */
class lucky_turn_cfg_impl : public load_json
{
public:
  lucky_turn_cfg_impl()
  { }
  ~lucky_turn_cfg_impl()
  {
    while (!this->lucky_turn_cfg_list_.empty())
      delete this->lucky_turn_cfg_list_.pop_front();
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LUCKY_TURN_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      lucky_turn_cfg_obj *p = new lucky_turn_cfg_obj();
      this->lucky_turn_cfg_list_.push_back(p);

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

  ilist<lucky_turn_cfg_obj *> *get_cfg_list()
  { return &(this->lucky_turn_cfg_list_); }
private:
  ilist<lucky_turn_cfg_obj *> lucky_turn_cfg_list_;
};
lucky_turn_cfg::lucky_turn_cfg() : impl_(new lucky_turn_cfg_impl()) { }
int lucky_turn_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int lucky_turn_cfg::reload_config(const char *cfg_root)
{
  lucky_turn_cfg_impl *tmp_impl = new lucky_turn_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LUCKY_TURN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<lucky_turn_cfg_obj *> *lucky_turn_cfg::get_cfg_list()
{ return this->impl_->get_cfg_list(); }
// -----------------------------------------------------------------------------
/**
 * @class lucky_turn_rank_award_cfg_impl
 *
 * @brief
 */
class lucky_turn_rank_award_cfg_impl : public load_json
{
public:
  lucky_turn_rank_award_cfg_impl()
  { }
  ~lucky_turn_rank_award_cfg_impl()
  {
    while (!this->rank_award_cfg_list_.empty())
      delete this->rank_award_cfg_list_.pop_front();
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LUCKY_TURN_RANK_AWARD_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      lucky_turn_rank_award *p = new lucky_turn_rank_award();
      this->rank_award_cfg_list_.push_back(p);
      p->rank_min_ = (*iter)["rank_min"].asInt();
      p->rank_max_ = (*iter)["rank_max"].asInt();

      Json::Value &item_v = (*iter)["info"];
      for (Json::Value::iterator iter2 = item_v.begin();
           iter2 != item_v.end();
           ++iter2)
      {
        item_amount_bind_t v;
        v.cid_ = (*iter2)["item_cid"].asInt();
        v.bind_ = (*iter2)["bind"].asInt();
        v.amount_ = (*iter2)["item_cnt"].asInt();
        if (!item_config::instance()->find(v.cid_))
          return -1;
        p->award_item_list_.push_back(v);
      }
    }
    return 0;
  }

  ilist<item_amount_bind_t> *get_award_list(const int rank)
  {
    for (ilist_node<lucky_turn_rank_award *> *itor = this->rank_award_cfg_list_.head();
         itor != NULL;
         itor = itor->next_)
    {
      if (rank >= itor->value_->rank_min_
          && rank <= itor->value_->rank_max_)
        return &(itor->value_->award_item_list_);
    }
    return NULL;
  }
private:
  ilist<lucky_turn_rank_award *> rank_award_cfg_list_;
};
lucky_turn_rank_award_cfg::lucky_turn_rank_award_cfg() : impl_(new lucky_turn_rank_award_cfg_impl()) { }
int lucky_turn_rank_award_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int lucky_turn_rank_award_cfg::reload_config(const char *cfg_root)
{
  lucky_turn_rank_award_cfg_impl *tmp_impl = new lucky_turn_rank_award_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LUCKY_TURN_RANK_AWARD_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<item_amount_bind_t> *lucky_turn_rank_award_cfg::get_award_list(const int rank)
{ return this->impl_->get_award_list(rank); }
