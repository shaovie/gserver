#include "basic_attr_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class basic_attr_cfg_impl
 * 
 * @brief implement of basic_attr_cfg
 */
class basic_attr_cfg_impl : public load_json
{
public:
  basic_attr_cfg_impl()
    : basic_attr_cfg_info_(NULL)
  { }
  ~basic_attr_cfg_impl()
  {
    if (this->basic_attr_cfg_info_ != NULL)
      delete []this->basic_attr_cfg_info_;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BASIC_ATTR_CFG_PATH, root) != 0)
      return -1;

    this->basic_attr_cfg_info_ = new basic_attr_cfg_info[CHAR_CAREER_CNT + 1]();
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int career = ::atoi(iter.key().asCString());
      if (!util::is_career_ok(career)) return -1;

      basic_attr_cfg_info &baci = this->basic_attr_cfg_info_[career];
      baci.attr_val_[ATTR_T_HP]          = ((*iter)["sheng_ming"]).asInt();
      baci.attr_val_[ATTR_T_MP]          = ((*iter)["fa_li"]).asInt();
      baci.attr_val_[ATTR_T_GONG_JI]     = ((*iter)["gong_ji"]).asInt();
      baci.attr_val_[ATTR_T_FANG_YU]     = ((*iter)["fang_yu"]).asInt();
      baci.attr_val_[ATTR_T_MING_ZHONG]  = ((*iter)["ming_zhong"]).asInt();
      baci.attr_val_[ATTR_T_SHAN_BI]     = ((*iter)["shan_bi"]).asInt();
      baci.attr_val_[ATTR_T_BAO_JI]      = ((*iter)["bao_ji"]).asInt();
      baci.attr_val_[ATTR_T_KANG_BAO]    = ((*iter)["kang_bao"]).asInt();
      baci.attr_val_[ATTR_T_SHANG_MIAN]  = ((*iter)["shang_mian"]).asInt();
    }
    return 0;
  }
  basic_attr_cfg_info *basic_attr(const char career)
  {
    if (!util::is_career_ok(career)) return NULL;
    return &(this->basic_attr_cfg_info_[(int)career]);
  }
  basic_attr_cfg_info *basic_attr_cfg_info_;
};
basic_attr_cfg::basic_attr_cfg()
: impl_(new basic_attr_cfg_impl())
{ }
int basic_attr_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int basic_attr_cfg::reload_config(const char *cfg_root)
{
  basic_attr_cfg_impl *tmp_impl = new basic_attr_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", BASIC_ATTR_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
basic_attr_cfg_info *basic_attr_cfg::basic_attr(const char career)
{ return this->impl_->basic_attr(career); }
