#include "equip_score_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"
#include "def.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class equip_score_cfg_impl
 * 
 * @brief implement of equip_score_cfg
 */
class equip_score_cfg_impl : public load_json
{
public:
  equip_score_cfg_impl() : equip_score_coe_(NULL) { }
  ~equip_score_cfg_impl()
  {
    if (this->equip_score_coe_ != NULL)
      delete []this->equip_score_coe_;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, EQUIP_SCORE_CFG_PATH, root) != 0)
      return -1;

    this->equip_score_coe_ = new equip_score_coe[CHAR_CAREER_CNT + 1]();
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int career = ::atoi(iter.key().asCString());
      if (!util::is_career_ok(career)) return -1;

      equip_score_coe &zlcc = this->equip_score_coe_[career];
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
  equip_score_coe *get_equip_score_coe(const char career)
  {
    if (!util::is_career_ok(career)) return NULL;
    return &(this->equip_score_coe_[(int)career]);
  }
  equip_score_coe *equip_score_coe_;
};
equip_score_cfg::equip_score_cfg() : impl_(new equip_score_cfg_impl()) { }
int equip_score_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int equip_score_cfg::reload_config(const char *cfg_root)
{
  equip_score_cfg_impl *tmp_impl = new equip_score_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", EQUIP_SCORE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
equip_score_coe *equip_score_cfg::get_equip_score_coe(const char career)
{ return this->impl_->get_equip_score_coe(career); }
