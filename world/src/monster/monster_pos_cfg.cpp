#include "monster_pos_cfg.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class monster_pos_cfg_impl
 *
 * @brief
 */
class monster_pos_cfg_impl : public load_json
{
public:
  typedef std::map<int, mst_spawn_list *> mst_spawn_list_map_t;
  typedef std::map<int, mst_spawn_list *>::iterator mst_spawn_list_map_iter;

  monster_pos_cfg_impl() { }
  ~monster_pos_cfg_impl()
  {
    for (mst_spawn_list_map_iter iter = this->mst_spawn_list_map_.begin();
         iter != this->mst_spawn_list_map_.end();
         ++iter)
      delete iter->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, MONSTAR_POS_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      mst_spawn_list *msl = new mst_spawn_list();
      this->mst_spawn_list_map_.insert(std::make_pair(scene_cid, msl));
      if (this->load_json(msl, *iter) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(mst_spawn_list *msl, Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      mst_spawn_list::_info *info = new mst_spawn_list::_info();
      msl->spawn_list_.push_back(info);

      info->monster_cid_ = ::atoi(iter.key().asCString());
      info->monster_cnt_ = (*iter)["mst_cnt"].asInt();
      info->param_       = (*iter)["param"].asInt();

      char bf[1024] = {0};
      ::strncpy(bf, (*iter)["tar_coord"].asCString(), sizeof(bf) - 1);
      char *tok_p = NULL;
      char *token = NULL;
      for(token = ::strtok_r(bf, ";", &tok_p);
          token != NULL;
          token = ::strtok_r(NULL, ";", &tok_p))
      {
        coord_t coord;
        if (::sscanf(token, "%hd,%hd", &coord.x_, &coord.y_) != 2)
          return -1;
        info->tar_coord_.push_back(coord);
      }
    }
    return 0;
  }
  mst_spawn_list *get_mst_spawn_list(const int scene_cid)
  {
    mst_spawn_list_map_iter iter = this->mst_spawn_list_map_.find(scene_cid);
    if (iter == this->mst_spawn_list_map_.end())
      return NULL;
    return iter->second;
  }
  mst_spawn_list::_info *get_mst_spawn_info(const int scene_cid, const int monster_cid)
  {
    mst_spawn_list_map_iter iter = this->mst_spawn_list_map_.find(scene_cid);
    if (iter == this->mst_spawn_list_map_.end())
      return NULL;
    mst_spawn_list *msl = iter->second;
    for (ilist_node<mst_spawn_list::_info *> *info_iter = msl->spawn_list_.head();
         info_iter != NULL;
         info_iter = info_iter->next_)
    {
      if (info_iter->value_->monster_cid_ == monster_cid)
        return info_iter->value_;
    }
    return NULL;
  }
private:
  mst_spawn_list_map_t mst_spawn_list_map_;
};

monster_pos_cfg::monster_pos_cfg() :
  impl_(new monster_pos_cfg_impl())
{ }
int monster_pos_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int monster_pos_cfg::reload_config(const char *cfg_root)
{
  monster_pos_cfg_impl *tmp_impl = new monster_pos_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", MONSTAR_POS_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
mst_spawn_list *monster_pos_cfg::get_mst_spawn_list(const int scene_cid)
{ return this->impl_->get_mst_spawn_list(scene_cid); }
mst_spawn_list::_info *monster_pos_cfg::get_mst_spawn_info(const int scene_cid,
                                                           const int monster_cid)
{ return this->impl_->get_mst_spawn_info(scene_cid, monster_cid); }

