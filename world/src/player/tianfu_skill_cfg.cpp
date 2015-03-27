#include "tianfu_skill_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "clsid.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

tianfu_skill_cfg_obj::tianfu_skill_cfg_obj() :
  anti_cid_(0),
  init_val_(0),
  inc_val_(0)
{ }
/**
 * @class tianfu_skill_cfg_impl
 * 
 * @brief implement of tianfu_skill_cfg
 */
class tianfu_skill_cfg_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, tianfu_skill_cfg_obj *> tianfu_skill_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, tianfu_skill_cfg_obj *>::iterator tianfu_skill_cfg_obj_map_itor;

  tianfu_skill_cfg_impl() :
    tianfu_skill_cfg_obj_map_(8)
  { }
  ~tianfu_skill_cfg_impl()
  {
    for (tianfu_skill_cfg_obj_map_itor itor = this->tianfu_skill_cfg_obj_map_.begin();
         itor != this->tianfu_skill_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TIANFU_SKILL_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int skill_cid = ::atoi(iter.key().asCString());

      tianfu_skill_cfg_obj *p = new tianfu_skill_cfg_obj();
      this->tianfu_skill_cfg_obj_map_.insert(std::make_pair(skill_cid, p));
      sk_rel_buff_info binfo;
      if (::sscanf((*iter)["buf_info"].asCString(),
                   "%d,%d,%d",
                   &binfo.id_, &binfo.rate_, &binfo.target_type_) == 3)
        p->buff_info_.push_back(binfo);
      p->anti_cid_ = (*iter)["anti_cid"].asInt();
      p->init_val_ = (*iter)["initial_val"].asInt();
      p->inc_val_  = (*iter)["inc_val"].asInt();
    }
    return 0;
  }
  const tianfu_skill_cfg_obj *get_skill(const int skill_cid)
  {
    tianfu_skill_cfg_obj_map_itor itor = this->tianfu_skill_cfg_obj_map_.find(skill_cid);
    if (itor == this->tianfu_skill_cfg_obj_map_.end()) return NULL; 
    return itor->second;
  }
  int get_tian_fu_amount()
  { return this->tianfu_skill_cfg_obj_map_.size(); }
private:
  tianfu_skill_cfg_obj_map_t tianfu_skill_cfg_obj_map_;
};
tianfu_skill_cfg::tianfu_skill_cfg() : impl_(new tianfu_skill_cfg_impl()) { }
int tianfu_skill_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int tianfu_skill_cfg::reload_config(const char *cfg_root)
{
  tianfu_skill_cfg_impl *tmp_impl = new tianfu_skill_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TIANFU_SKILL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const tianfu_skill_cfg_obj *tianfu_skill_cfg::get_skill(const int skill_cid)
{ return this->impl_->get_skill(skill_cid); }
int tianfu_skill_cfg::get_tian_fu_amount()
{ return this->impl_->get_tian_fu_amount(); }
