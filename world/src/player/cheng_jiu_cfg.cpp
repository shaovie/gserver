#include "cheng_jiu_cfg.h"
#include "global_macros.h"
#include "load_json.h"
#include "def.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

cheng_jiu_obj::cheng_jiu_obj() :
  show_lvl_(0),
  value_(0),
  next_cj_id_(0),
  title_(0),
  lucky_rate_(0)
{ }
/**
 * @class cheng_jiu_cfg_impl
 *
 * @brief
 */
class cheng_jiu_cfg_impl : public load_json
{
public:
  typedef std::map<int/*id*/, cheng_jiu_obj *> cheng_jiu_obj_map_t;
  typedef std::map<int/*id*/, cheng_jiu_obj *>::iterator cheng_jiu_obj_map_iter;

  cheng_jiu_cfg_impl() { }
  ~cheng_jiu_cfg_impl()
  {
    for (cheng_jiu_obj_map_iter itor = this->cheng_jiu_obj_map_.begin();
         itor != this->cheng_jiu_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, CHENG_JIU_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int cj_id = ::atoi(iter.key().asCString());
      cheng_jiu_obj *cjo = new cheng_jiu_obj();
      this->cheng_jiu_obj_map_.insert(std::make_pair(cj_id, cjo));

      cjo->show_lvl_   = (*iter)["show_lvl"].asInt();
      cjo->value_      = (*iter)["param"].asInt();
      cjo->title_      = (*iter)["title"].asInt();
      cjo->next_cj_id_ = (*iter)["next_index"].asInt();
      cjo->lucky_rate_ = (*iter)["lucky_rate"].asInt();

      if ((cj_id / 1000) != (cjo->next_cj_id_ / 1000))
        return -1;

      char bf[128] = {0};
      ::strncpy(bf, (*iter)["award"].asCString(), sizeof(bf));
      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        item_amount_bind_t v;
        int bind = 0;
        if (::sscanf(token, "%d,%d,%d",
                     &v.cid_, &v.amount_, &bind) != 3)
          return -1;
        v.bind_ = bind;
        cjo->award_list_.push_back(v);
      }

      ::memset(bf, 0, sizeof(bf));
      ::strncpy(bf, (*iter)["lucky_award"].asCString(), sizeof(bf));
      tok_p = NULL;
      token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        item_amount_bind_t v;
        int bind = 0;
        if (::sscanf(token, "%d,%d,%d",
                     &v.cid_, &v.amount_, &bind) != 3)
          return -1;
        v.bind_ = bind;
        cjo->lucky_award_.push_back(v);
      }
    }

    return 0;
  }
  cheng_jiu_obj *get_cheng_jiu_obj(const int cj_id)
  {
    cheng_jiu_obj_map_iter iter = this->cheng_jiu_obj_map_.find(cj_id);
    if (iter == this->cheng_jiu_obj_map_.end())
      return NULL;
    return iter->second;
  }
private:
  cheng_jiu_obj_map_t cheng_jiu_obj_map_;
};
cheng_jiu_cfg::cheng_jiu_cfg() : impl_(new cheng_jiu_cfg_impl()) { }
int cheng_jiu_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int cheng_jiu_cfg::reload_config(const char *cfg_root)
{
  cheng_jiu_cfg_impl *tmp_impl = new cheng_jiu_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", CHENG_JIU_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
cheng_jiu_obj *cheng_jiu_cfg::get_cheng_jiu_obj(const int cj_id)
{ return this->impl_->get_cheng_jiu_obj(cj_id); }

