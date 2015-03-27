#include "lucky_goods_draw_cfg.h"
#include "load_json.h"
#include "item_config.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header

#define MAX_LUCKY_GOODS_DRAW_CNT     20

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class lucky_goods_draw_cfg_impl
 *
 * @brief
 */
class lucky_goods_draw_cfg_impl : public load_json
{
public:
  lucky_goods_draw_cfg_impl() : draw_item_obj_map_(MAX_LUCKY_GOODS_DRAW_CNT) { }
  ~lucky_goods_draw_cfg_impl()
  {
    for (int i = 0; i < this->draw_item_obj_map_.size(); ++i)
    {
      ilist<lucky_goods_draw_cfg_obj *> *p = this->draw_item_obj_map_.find(i);
      if (p != NULL)
        while (!p->empty())
          delete p->pop_front();
      delete p;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LUCKY_GOODS_DRAW_CFG_PATH, root) != 0)
      return -1;
    
    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cnt = ::atoi(iter.key().asCString());
      if (cnt >= MAX_LUCKY_GOODS_DRAW_CNT) return -1;
      ilist<lucky_goods_draw_cfg_obj *> *p = new ilist<lucky_goods_draw_cfg_obj *>();
      this->draw_item_obj_map_.insert(cnt, p);
      Json::Value &v = *iter;
      for (Json::Value::iterator i_iter = v.begin();
           i_iter != v.end();
           ++i_iter)
      {
        Json::Value &item_v = *i_iter;
        lucky_goods_draw_cfg_obj *pi = new lucky_goods_draw_cfg_obj();
        p->push_back(pi);

        pi->item_cid_ = item_v["item_cid"].asInt();
        if (!item_config::instance()->find(pi->item_cid_))
        {
          e_log->error("item %d not found!", pi->item_cid_);
          return -1;
        }
        pi->min_cnt_ = item_v["min_item_cnt"].asInt();
        pi->max_cnt_ = item_v["max_item_cnt"].asInt();
        if (pi->max_cnt_ < pi->min_cnt_)
        {
          e_log->rinfo("%d max cnt %d:%d is invalid!", pi->item_cid_,
                       pi->max_cnt_, pi->min_cnt_); 
          return -1;
        }
        pi->bind_type_ = item_v["bind"].asInt();
        pi->rate_ = item_v["p"].asInt();
      }
    }
    return 0;
  }
  ilist<lucky_goods_draw_cfg_obj *> *get_cfg_obj(const int cnt)
  { return this->draw_item_obj_map_.find(cnt); }
private:
  array_t<ilist<lucky_goods_draw_cfg_obj *> *> draw_item_obj_map_;
};
lucky_goods_draw_cfg::lucky_goods_draw_cfg() : impl_(new lucky_goods_draw_cfg_impl()) { }
int lucky_goods_draw_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int lucky_goods_draw_cfg::reload_config(const char *cfg_root)
{
  lucky_goods_draw_cfg_impl *tmp_impl = new lucky_goods_draw_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LUCKY_GOODS_DRAW_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<lucky_goods_draw_cfg_obj *> *lucky_goods_draw_cfg::get_cfg_obj(const int cnt)
{ return this->impl_->get_cfg_obj(cnt); }
