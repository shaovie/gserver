#include "title_cfg.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header
#include <map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

title_cfg_obj::title_cfg_obj()
{
  ::memset(attr_, 0, sizeof(attr_));
}
/**
 * @class title_cfg_impl
 *
 * @brief
 */
class title_cfg_impl : public load_json
{
public:
  typedef std::map<int/*id*/, title_cfg_obj *> title_cfg_obj_map_t;
  typedef std::map<int/*id*/, title_cfg_obj *>::iterator title_cfg_obj_map_iter;

  title_cfg_impl() { }
  ~title_cfg_impl()
  {
    for (title_cfg_obj_map_iter itor = this->title_cfg_obj_map_.begin();
         itor != this->title_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TITLE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int title_cid = ::atoi(iter.key().asCString());
      title_cfg_obj *to = new title_cfg_obj();
      this->title_cfg_obj_map_.insert(std::make_pair(title_cid, to));

      to->attr_[ATTR_T_HP]         = (*iter)["sheng_ming"].asInt();
      to->attr_[ATTR_T_GONG_JI]    = (*iter)["gong_ji"].asInt();
      to->attr_[ATTR_T_FANG_YU]    = (*iter)["fang_yu"].asInt();
      to->attr_[ATTR_T_MING_ZHONG] = (*iter)["ming_zhong"].asInt();
      to->attr_[ATTR_T_SHAN_BI]    = (*iter)["shan_bi"].asInt();
      to->attr_[ATTR_T_BAO_JI]     = (*iter)["bao_ji"].asInt();
      to->attr_[ATTR_T_KANG_BAO]   = (*iter)["kang_bao"].asInt();
      to->attr_[ATTR_T_SHANG_MIAN] = (*iter)["shang_mian"].asInt();
    }

    return 0;
  }
  title_cfg_obj *get_title_cfg_obj(const int title_cid)
  {
    title_cfg_obj_map_iter iter = this->title_cfg_obj_map_.find(title_cid);
    if (iter == this->title_cfg_obj_map_.end())
      return NULL;
    return iter->second;
  }
private:
  title_cfg_obj_map_t title_cfg_obj_map_;
};
title_cfg::title_cfg() : impl_(new title_cfg_impl()) { }
int title_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int title_cfg::reload_config(const char *cfg_root)
{
  title_cfg_impl *tmp_impl = new title_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TITLE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
title_cfg_obj *title_cfg::get_title_cfg_obj(const int title_cid)
{ return this->impl_->get_title_cfg_obj(title_cid); }

