#include "exit_transfer_cfg.h"
#include "scene_config.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"
#include "ilist.h"

#define MAX_EXIT_TRANSFER_ID    128

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

class exit_cfg_info
{
public:
  int load_json(Json::Value &root)
  {
    this->from_scene_coord_.cid_ = root["scene_cid"].asInt();
    if (::sscanf(root["pos_info"].asCString(),
                 "%hd,%hd",
                 &(this->from_scene_coord_.coord_.x_),
                 &(this->from_scene_coord_.coord_.y_)) != 2)
      return -1;

    char bf[256] = {0};
    ::strncpy(bf, root["to_info"].asCString(), sizeof(bf) - 1);

    char *tok_p = NULL;
    char *token = NULL;
    for (token = ::strtok_r(bf, ";", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ";", &tok_p))
    {
      scene_coord_t v;
      if (::sscanf(token,
                   "%d,%hd,%hd",
                   &(v.cid_),
                   &(v.coord_.x_),
                   &(v.coord_.y_)) != 3)
        return -1;
      if (!scene_config::instance()->can_move(v.cid_,
                                              v.coord_.x_,
                                              v.coord_.y_))
      {
        e_log->rinfo("exit trasfer to_pos %s is not movable!", token);
        return -1;
      }
      this->to_scene_coord_list_.push_back(v);
    }
    if (this->to_scene_coord_list_.empty())
      return -1;
    return 0;
  }
  scene_coord_t from_scene_coord_;
  ilist<scene_coord_t> to_scene_coord_list_;
};

/**
 * @class exit_transfer_cfg_impl
 * 
 * @brief implement of exit_transfer_cfg 
 */
class exit_transfer_cfg_impl : public load_json
{
public:
  exit_transfer_cfg_impl() :
    exit_cfg_info_map_(MAX_EXIT_TRANSFER_ID)
  { }
  ~exit_transfer_cfg_impl()
  { 
    for (int i = 0; i < this->exit_cfg_info_map_.size(); ++i)
    {
      exit_cfg_info *p = this->exit_cfg_info_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SCENE_TRANSFER_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (id < 0 || id >= MAX_EXIT_TRANSFER_ID) return -1;
      exit_cfg_info *p = new exit_cfg_info();
      this->exit_cfg_info_map_.insert(id, p);
      if (p->load_json(*iter) != 0) return -1;
    }
    return 0;
  }
  scene_coord_t get_target_coord(const int id, const int scene_cid)
  {
    exit_cfg_info *p = this->exit_cfg_info_map_.find(id);
    if (p == NULL
        || p->from_scene_coord_.cid_ != scene_cid)
      return scene_coord_t();
    int r = rand() % p->to_scene_coord_list_.size();
    ilist_node<scene_coord_t> *itor = p->to_scene_coord_list_.head();
    for (int i = 0; itor != NULL; ++i, itor = itor->next_)
    {
      if (i == r)
        return itor->value_;
    }
    return scene_coord_t();
  }
private:
  array_t<exit_cfg_info *> exit_cfg_info_map_;
};
// ---------------------------------------------------------------------------
exit_transfer_cfg::exit_transfer_cfg() : impl_(new exit_transfer_cfg_impl()) { }
int exit_transfer_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int exit_transfer_cfg::reload_config(const char *cfg_root)
{
  exit_transfer_cfg_impl *tmp_impl = new exit_transfer_cfg_impl();
  if (tmp_impl->load_config(cfg_root) != 0)
  {
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
scene_coord_t exit_transfer_cfg::get_target_coord(const int id, const int scene_cid)
{ return this->impl_->get_target_coord(id, scene_cid); }
