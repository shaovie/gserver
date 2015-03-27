#include "scene_monster_cfg.h"
#include "load_json.h"
#include "monster_cfg.h"
#include "global_macros.h"
#include "array_t.h"
#include "sys_log.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class monster_info
 *
 * @brief
 */
class monster_info
{
public:
  class _mst_spawn_info
  {
  public:
    int idx_;
    int mst_cid_;
    int dir_;
  };
  monster_info() :
    mst_spawn_info_map_(MAX_LAYOUT_ID)
  { }
  ~monster_info()
  {
    for (int i = 0; i < this->mst_spawn_info_map_.size(); ++i)
    {
      monster_info::_mst_spawn_info *p = this->mst_spawn_info_map_.find(i);
      if (p != NULL) delete p;
    }
  }
  int load_json(Json::Value &root)
  {
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      _mst_spawn_info *p = new _mst_spawn_info();
      p->idx_ = id;
      p->mst_cid_ = (*iter)["monster_cid"].asInt();
      p->dir_ = (*iter)["toward"].asInt();
      this->mst_spawn_info_map_.insert(p->idx_, p);
    }
    return 0;
  }
public:
  array_t<monster_info::_mst_spawn_info *> mst_spawn_info_map_;
};
/**
 * @class scene_monster_cfg_impl
 *
 * @brief
 */
class scene_monster_cfg_impl : public load_json
{
public:
  scene_monster_cfg_impl() :
    scene_mst_map_(MAX_SCENE_CID)
  { }
  ~scene_monster_cfg_impl()
  {
    for (int i = 0; i < this->scene_mst_map_.size(); ++i)
    {
      monster_info *mi = this->scene_mst_map_.find(i);
      if (mi != NULL) delete mi;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SCENE_MONSTER_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      monster_info *mi = new monster_info();
      this->scene_mst_map_.insert(scene_cid, mi);
      if (mi->load_json(*iter) != 0)
        return -1;
    }
    return 0;
  }

  bool have_monster(const int scene_cid)
  { return this->scene_mst_map_.find(scene_cid) != NULL; }
  int get_monster_info(const int scene_cid,
                       const int idx,
                       int &mst_cid,
                       int &dir)
  { 
    monster_info *mi = this->scene_mst_map_.find(scene_cid);
    if (mi == NULL) return -1;
    monster_info::_mst_spawn_info *p = mi->mst_spawn_info_map_.find(idx);
    if (p == NULL) return -1;
    mst_cid = p->mst_cid_;
    dir = p->dir_;
    return 0;
  }
private:
  array_t<monster_info *> scene_mst_map_;
};

scene_monster_cfg::scene_monster_cfg() : impl_(new scene_monster_cfg_impl()) { }
int scene_monster_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int scene_monster_cfg::reload_config(const char *cfg_root)
{
  scene_monster_cfg_impl *tmp_impl = new scene_monster_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SCENE_MONSTER_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int scene_monster_cfg::get_monster_info(const int scene_cid,
                                        const int idx,
                                        int &mst_cid,
                                        int &dir)
{ return this->impl_->get_monster_info(scene_cid, idx, mst_cid, dir); }
bool scene_monster_cfg::have_monster(const int scene_cid)
{ return this->impl_->have_monster(scene_cid); }
