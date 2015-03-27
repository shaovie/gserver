#include "zhan_li_calc_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class zhan_li_calc_cfg_impl
 * 
 * @brief implement of zhan_li_calc_cfg
 */
class zhan_li_calc_cfg_impl : public load_json
{
public:
  zhan_li_calc_cfg_impl() : zhan_li_calc_coe_(NULL) { }
  ~zhan_li_calc_cfg_impl()
  {
    if (this->zhan_li_calc_coe_ != NULL)
      delete []this->zhan_li_calc_coe_;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, ZHAN_LI_CALC_CFG_PATH, root) != 0)
      return -1;

    this->zhan_li_calc_coe_ = new zhan_li_calc_coe[CHAR_CAREER_CNT + 1]();
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int career = ::atoi(iter.key().asCString());
      if (!util::is_career_ok(career)) return -1;

      zhan_li_calc_coe &zlcc = this->zhan_li_calc_coe_[career];
      zlcc.attr_coe_[ATTR_T_HP]          = ((*iter)["sheng_ming_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_MP]          = ((*iter)["fa_li_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_GONG_JI]     = ((*iter)["gong_ji_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_FANG_YU]     = ((*iter)["fang_yu_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_MING_ZHONG]  = ((*iter)["ming_zhong_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_SHAN_BI]     = ((*iter)["shan_bi_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_BAO_JI]      = ((*iter)["bao_ji_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_KANG_BAO]    = ((*iter)["kang_bao_coe"]).asDouble();
      zlcc.attr_coe_[ATTR_T_SHANG_MIAN]  = ((*iter)["shang_mian_coe"]).asDouble();
    }
    return 0;
  }
  zhan_li_calc_coe *get_zhan_li_calc_coe(const char career)
  {
    if (!util::is_career_ok(career)) return NULL;
    return &(this->zhan_li_calc_coe_[(int)career]);
  }
  zhan_li_calc_coe *zhan_li_calc_coe_;
};
zhan_li_calc_cfg::zhan_li_calc_cfg() : impl_(new zhan_li_calc_cfg_impl()) { }
int zhan_li_calc_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int zhan_li_calc_cfg::reload_config(const char *cfg_root)
{
  zhan_li_calc_cfg_impl *tmp_impl = new zhan_li_calc_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", ZHAN_LI_CALC_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
zhan_li_calc_coe *zhan_li_calc_cfg::get_zhan_li_calc_coe(const char career)
{ return this->impl_->get_zhan_li_calc_coe(career); }
