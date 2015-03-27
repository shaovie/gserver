#include "timing_mst_config.h"
#include "scene_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

// Defines
static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

class timing_mst_config_impl : public load_json
{
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TIMING_MST_CONFIG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      const int index = ::atoi(iter.key().asCString());
      if (index <= 0 || index >= MAX_TIMING_MST_IDX)
      {
        e_log->rinfo("index[%d] error!", index);
        return -1;
      }

      timing_mst_cfg_obj *obj = new timing_mst_cfg_obj();
      this->obj_list_.push_back(obj);

      obj->index_ = index;
      obj->scene_cid_ = (*iter)["scene_cid"].asInt();
      obj->mst_cid_   = (*iter)["monster_cid"].asInt();
      obj->coord_x_   = (*iter)["coord_x"].asInt();
      obj->coord_y_   = (*iter)["coord_y"].asInt();
      obj->range_     = (*iter)["range"].asInt();
      if (!scene_config::instance()->can_move(obj->scene_cid_,
                                              obj->coord_x_,
                                              obj->coord_y_))
      {
        e_log->rinfo("%s [%d,%d] can not move",
                     TIMING_MST_CONFIG_PATH,
                     obj->coord_x_, obj->coord_y_);
        return -1;
      }

      char *tok_p = NULL;
      char *token = NULL;
      char bf[256] = {0};
      ::strncpy(bf, (*iter)["month"].asCString(), sizeof(bf));
      for(token = ::strtok_r(bf, ",", &tok_p);
          token != NULL;
          token = ::strtok_r(NULL, ",", &tok_p))
        obj->month_list_.push_back(::atoi(token));

      ::strncpy(bf, (*iter)["week"].asCString(), sizeof(bf));
      for(token = ::strtok_r(bf, ",", &tok_p);
          token != NULL;
          token = ::strtok_r(NULL, ",", &tok_p))
        obj->wday_list_.push_back(::atoi(token));

      ::strncpy(bf, (*iter)["day"].asCString(), sizeof(bf));
      for(token = ::strtok_r(bf, ",", &tok_p);
          token != NULL;
          token = ::strtok_r(NULL, ",", &tok_p))
        obj->mday_list_.push_back(::atoi(token));

      ::strncpy(bf, (*iter)["refresh_time"].asCString(), sizeof(bf));
      for(token = ::strtok_r(bf, ",", &tok_p);
          token != NULL;
          token = ::strtok_r(NULL, ",", &tok_p))
      {
        int hour = 0;
        int min = 0;
        ::sscanf(token, "%d:%d", &hour, &min);
        obj->time_list_.push_back(pair_t<char>(hour, min));
      }
    }
    return 0;
  }
  ilist<timing_mst_cfg_obj *> *get_timing_mst_list(void)
  { return &this->obj_list_; }
private:
  ilist<timing_mst_cfg_obj *> obj_list_;
};
timing_mst_config::timing_mst_config() :
  impl_(new timing_mst_config_impl())
{ }
int timing_mst_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int timing_mst_config::reload_config(const char *cfg_root)
{
  timing_mst_config_impl *tmp_impl = new timing_mst_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TIMING_MST_CONFIG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist<timing_mst_cfg_obj *> *timing_mst_config::get_timing_mst_list(void)
{ return this->impl_->get_timing_mst_list(); }
