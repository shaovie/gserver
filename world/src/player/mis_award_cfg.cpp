#include "mis_award_cfg.h"
#include "item_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"

// Lib header
#include "array_t.h"
#include "ilist.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

#define MAX_ONLINE_AWARD_LVL_IDX  10
#define MAX_ONLINE_AWARD_TIME_IDX 6
#define MAX_LOGIN_AWARD_IDX       7

/**
 * @class online_award_cfg
 *
 * @brief
 */
class online_award_cfg_impl : public load_json
{
public:
  class time_award_cfg_obj
  {
  public:
    time_award_cfg_obj() :
      min_time_(0)
    { }
  public:
    int min_time_;
    ilist<item_amount_bind_t> award_list_;
  };
public:
  online_award_cfg_impl() :
    obj_map_(MAX_ONLINE_AWARD_LVL_IDX + 1)
  { }
  ~online_award_cfg_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      array_t<time_award_cfg_obj *> *obj = this->obj_map_.find(i);
      if (obj == NULL) continue;
      for (int j = 0; j < obj->size(); ++j)
      {
        time_award_cfg_obj *tac_obj = obj->find(j);
        if (tac_obj != NULL) delete tac_obj;
      }
      delete obj;
    }
  }
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, ONLINE_AWARD_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int idx = ::atoi(itor.key().asCString());
      if (idx > MAX_ONLINE_AWARD_LVL_IDX)
      {
        e_log->error("overflow idx %d", idx);
        return -1;
      }
      array_t<time_award_cfg_obj *> *array = new array_t<time_award_cfg_obj *>(MAX_ONLINE_AWARD_TIME_IDX + 1);
      this->obj_map_.insert(idx, array);

      int gift_idx = 0;
      for(Json::Value::iterator itor2 = (*itor).begin();
          itor2 != (*itor).end() && gift_idx <= MAX_ONLINE_AWARD_TIME_IDX;
          ++itor2)
      {
        time_award_cfg_obj *obj = new time_award_cfg_obj();
        array->insert(++gift_idx, obj);
        obj->min_time_ = (*itor2)["time"].asInt();

        Json::Value &award_v = (*itor2)["award"];
        if (award_v.empty()) continue;
        char *tok_p = NULL;
        char *token = NULL;
        char bf[256] = {0};
        ::strncpy(bf, award_v.asCString(), sizeof(bf) - 1);
        for (token = ::strtok_r(bf, ";", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ";", &tok_p))
        {
          item_amount_bind_t item;
          int bind = 0;
          ::sscanf(token, "%d,%d,%d", &item.cid_, &item.amount_, &bind);
          item.bind_ = bind;
          obj->award_list_.push_back(item);
          if (!item_config::instance()->find(item.cid_))
            return -1;
        }
      }
    }
    return 0;
  }
  int get_award_min_time(const short lvl, const char get_idx)
  {
    const int lvl_idx = lvl / 10;
    array_t<time_award_cfg_obj *> *lvl_obj = this->obj_map_.find(lvl_idx);
    if (lvl_obj == NULL) return MAX_INVALID_INT;

    time_award_cfg_obj *time_obj = lvl_obj->find(get_idx);
    if (time_obj == NULL) return MAX_INVALID_INT;
    return time_obj->min_time_;
  }
  ilist<item_amount_bind_t> *award_list(const short lvl, const char get_idx)
  {
    const int lvl_idx = lvl / 10;
    array_t<time_award_cfg_obj *> *lvl_obj = this->obj_map_.find(lvl_idx);
    if (lvl_obj == NULL) return NULL;

    time_award_cfg_obj *time_obj = lvl_obj->find(get_idx);
    if (time_obj == NULL) return NULL;
    return &time_obj->award_list_;
  }
private:
  array_t<array_t<time_award_cfg_obj *> *> obj_map_;
};
online_award_cfg::online_award_cfg() : impl_(new online_award_cfg_impl()) { }
int online_award_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int online_award_cfg::reload_config(const char *cfg_root)
{
  online_award_cfg_impl *tmp_impl = new online_award_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", ONLINE_AWARD_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int online_award_cfg::get_award_min_time(const short lvl, const char get_idx)
{ return this->impl_->get_award_min_time(lvl, get_idx); }
ilist<item_amount_bind_t> *online_award_cfg::award_list(const short lvl, const char get_idx)
{ return this->impl_->award_list(lvl, get_idx); }

/**
 * @class login_award_cfg
 *
 * @brief
 */
class login_award_cfg_impl : public load_json
{
public:
  login_award_cfg_impl() :
    obj_map_(MAX_LOGIN_AWARD_IDX + 1)
  { }
  ~login_award_cfg_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      ilist<item_amount_bind_t> *obj = obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, LOGIN_AWARD_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int idx = ::atoi(itor.key().asCString());
      if (idx > MAX_LOGIN_AWARD_IDX)
      {
        e_log->error("overflow idx %d", idx);
        return -1;
      }
      ilist<item_amount_bind_t> *list = new ilist<item_amount_bind_t>;
      this->obj_map_.insert(idx, list);

      for(Json::Value::iterator itor2 = (*itor).begin();
          itor2 != (*itor).end();
          ++itor2)
      {
        item_amount_bind_t item;
        item.cid_    = (*itor2)["item_cid"].asInt();
        item.bind_   = (*itor2)["bind"].asInt();
        item.amount_ = (*itor2)["item_cnt"].asInt();
        list->push_back(item);
        if (!item_config::instance()->find(item.cid_))
          return -1;
      }
    }
    return 0;
  }
  ilist<item_amount_bind_t> *award_list(const char wday)
  { return this->obj_map_.find(wday); }
private:
  array_t<ilist<item_amount_bind_t> *> obj_map_;
};
login_award_cfg::login_award_cfg() : impl_(new login_award_cfg_impl()) { }
int login_award_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int login_award_cfg::reload_config(const char *cfg_root)
{
  login_award_cfg_impl *tmp_impl = new login_award_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", LOGIN_AWARD_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<item_amount_bind_t> *login_award_cfg::award_list(const char wday)
{ return this->impl_->award_list(wday); }

/**
 * @class seven_day_login_cfg
 *
 * @brief
 */
class seven_day_login_cfg_impl : public load_json
{
public:
  seven_day_login_cfg_impl() :
    obj_map_(8)
  { }
  ~seven_day_login_cfg_impl()
  {
    for (int i = 0; i < this->obj_map_.size(); ++i)
    {
      ilist<item_amount_bind_t> *obj = obj_map_.find(i);
      if (obj != NULL) delete obj;
    }
  }
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SEVEN_DAY_LOGIN_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      int idx = ::atoi(itor.key().asCString());
      if (idx > 7)
      {
        e_log->error("overflow day %d", idx);
        return -1;
      }
      ilist<item_amount_bind_t> *list = new ilist<item_amount_bind_t>;
      this->obj_map_.insert(idx, list);

      for(Json::Value::iterator itor2 = (*itor).begin();
          itor2 != (*itor).end();
          ++itor2)
      {
        item_amount_bind_t item;
        item.cid_    = (*itor2)["item_cid"].asInt();
        item.bind_   = (*itor2)["bind"].asInt();
        item.amount_ = (*itor2)["item_cnt"].asInt();
        list->push_back(item);
        if (!item_config::instance()->find(item.cid_))
          return -1;
      }
    }
    return 0;
  }
  ilist<item_amount_bind_t> *award_list(const char day)
  { return this->obj_map_.find(day); }
private:
  array_t<ilist<item_amount_bind_t> *> obj_map_;
};
seven_day_login_cfg::seven_day_login_cfg() : impl_(new seven_day_login_cfg_impl()) { }
int seven_day_login_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int seven_day_login_cfg::reload_config(const char *cfg_root)
{
  seven_day_login_cfg_impl *tmp_impl = new seven_day_login_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SEVEN_DAY_LOGIN_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<item_amount_bind_t> *seven_day_login_cfg::award_list(const char wday)
{ return this->impl_->award_list(wday); }
