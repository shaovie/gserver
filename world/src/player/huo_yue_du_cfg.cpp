#include "huo_yue_du_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class huo_yue_du_obj
 *
 * @brief
 */
class huo_yue_du_obj
{
public:
  huo_yue_du_obj() :
    param_(0),
    total_score_(0),
    per_score_(0)
  { }

  int param_;
  int total_score_;
  int per_score_;
  ilist<item_amount_bind_t> award_;
};

/**
 * @class huo_yue_du_cfg_impl
 *
 * @brief
 */
class huo_yue_du_cfg_impl : public load_json
{
public:
  huo_yue_du_cfg_impl() :
    huo_yue_du_obj_map_(HYD_END)
  { }
  ~huo_yue_du_cfg_impl()
  {
    for (int i = 0; i < this->huo_yue_du_obj_map_.size(); ++i)
    {
      huo_yue_du_obj *hydo = this->huo_yue_du_obj_map_.find(i);
      if (hydo != NULL) delete hydo;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, HUO_YUE_DU_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int huo_yue_du_cid = ::atoi(iter.key().asCString());
      if (huo_yue_du_cid >= HYD_END)
        return -1;
      huo_yue_du_obj *hydo = new huo_yue_du_obj();
      this->huo_yue_du_obj_map_.insert(huo_yue_du_cid, hydo);

      hydo->param_       = (*iter)["param"].asInt();
      hydo->total_score_ = (*iter)["total_score"].asInt();
      hydo->per_score_   = (*iter)["per_score"].asInt();

      char bf[128] = {0};
      ::strncpy(bf, (*iter)["award"].asCString(), sizeof(bf));
      item_amount_bind_t item;
      int bind = 0;
      if (::sscanf(bf, "%d,%d,%d",
                   &item.cid_, &item.amount_, &bind) != 3)
        return -1;
      item.bind_ = bind;
      hydo->award_.push_back(item);
    }

    return 0;
  }
  int get_param(const int huo_yue_du_cid)
  {
    huo_yue_du_obj *hydo = this->huo_yue_du_obj_map_.find(huo_yue_du_cid);
    if (hydo == NULL)
      return 0;
    return hydo->param_;
  }
  int get_total_score(const int huo_yue_du_cid)
  {
    huo_yue_du_obj *hydo = this->huo_yue_du_obj_map_.find(huo_yue_du_cid);
    if (hydo == NULL)
      return 0;
    return hydo->total_score_;
  }
  int get_per_score(const int huo_yue_du_cid)
  {
    huo_yue_du_obj *hydo = this->huo_yue_du_obj_map_.find(huo_yue_du_cid);
    if (hydo == NULL)
      return 0;
    return hydo->per_score_;
  }
  ilist<item_amount_bind_t> *get_award(const int huo_yue_du_cid)
  {
    huo_yue_du_obj *hydo = this->huo_yue_du_obj_map_.find(huo_yue_du_cid);
    if (hydo == NULL) return NULL;
    return &hydo->award_;
  }
private:
  array_t<huo_yue_du_obj *> huo_yue_du_obj_map_;
};
huo_yue_du_cfg::huo_yue_du_cfg() : impl_(new huo_yue_du_cfg_impl()) { }
int huo_yue_du_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int huo_yue_du_cfg::reload_config(const char *cfg_root)
{
  huo_yue_du_cfg_impl *tmp_impl = new huo_yue_du_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", HUO_YUE_DU_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int huo_yue_du_cfg::get_param(const int huo_yue_du_cid)
{ return this->impl_->get_param(huo_yue_du_cid); }
int huo_yue_du_cfg::get_total_score(const int huo_yue_du_cid)
{ return this->impl_->get_total_score(huo_yue_du_cid); }
int huo_yue_du_cfg::get_per_score(const int huo_yue_du_cid)
{ return this->impl_->get_per_score(huo_yue_du_cid); }
ilist<item_amount_bind_t> *huo_yue_du_cfg::get_award(const int huo_yue_du_cid)
{ return this->impl_->get_award(huo_yue_du_cid); }

