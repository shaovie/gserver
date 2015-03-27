#include "buff_config.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class buff_config_impl
 * 
 * @brief implement of buff_config
 */
class buff_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, buff_cfg_obj *> buff_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, buff_cfg_obj *>::iterator buff_cfg_obj_map_iter;
  buff_config_impl() :
    buff_cfg_obj_map_(128)
  { }
  ~buff_config_impl()
  {
    for (buff_cfg_obj_map_iter itor = this->buff_cfg_obj_map_.begin();
         itor != this->buff_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BUFF_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      buff_cfg_obj *bco = new buff_cfg_obj();
      this->buff_cfg_obj_map_.insert(std::make_pair(id, bco));
      if (this->load_json(*iter, bco) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(Json::Value &root, buff_cfg_obj *bco)
  {
    bco->died_clear_ = root["died_clear"].asBool();
    bco->offline_clear_ = root["offline_clear"].asBool();
    bco->type_ = root["type"].asInt();
    bco->show_type_ = root["show_type"].asInt();
    bco->dj_cnt_ = root["dj_cnt"].asInt();
    bco->ctrl_type_ = root["control_type"].asInt();
    bco->value_ = root["value"].asInt();
    bco->last_ = root["last"].asInt();
    bco->interval_ = root["interval"].asInt();
    bco->param_1_ = root["param_1"].asInt();
    bco->param_2_ = root["param_2"].asInt();
    bco->param_3_ = root["param_3"].asInt();
    bco->mian_yi_time_ = root["mian_yi_time"].asInt();

    const char *effect = root["effect"].asCString();
    if (::strcmp(effect, "xu_ruo") == 0)
      bco->effect_id_ = BF_SK_XU_RUO;
    else if (::strcmp(effect, "xuan_yun") == 0)
      bco->effect_id_ = BF_SK_XUAN_YUN;
    else if (::strcmp(effect, "ding_shen") == 0)
      bco->effect_id_ = BF_SK_DING_SHEN;
    else if (::strcmp(effect, "jian_su") == 0)
      bco->effect_id_ = BF_SK_JIAN_SU;
    else if (::strcmp(effect, "jian_su_mst") == 0)
      bco->effect_id_ = BF_SK_JIAN_SU_MST;
    else if (::strcmp(effect, "xuan_feng") == 0)
      bco->effect_id_ = BF_SK_XUAN_FENG;
    else if (::strcmp(effect, "huo_huan") == 0)
      bco->effect_id_ = BF_SK_HUO_HUAN;
    else if (::strcmp(effect, "bian_shen") == 0)
      bco->effect_id_ = BF_SK_BIAN_SHEN;
    else if (::strcmp(effect, "ewai_shanghai") == 0)
      bco->effect_id_ = BF_SK_EWAI_SHANGHAI;
    else if (::strcmp(effect, "shandian_fengbao") == 0)
      bco->effect_id_ = BF_SK_SHANDIAN_FENGBAO;
    else if (::strcmp(effect, "huang_chong") == 0)
      bco->effect_id_ = BF_SK_HUANG_CHONG;
    else if (::strcmp(effect, "kuang_bao") == 0)
      bco->effect_id_ = BF_SK_KUANG_BAO;
    else if (::strcmp(effect, "tie_jia") == 0)
      bco->effect_id_ = BF_SK_TIE_JIA;
    else if (::strcmp(effect, "hui_chun") == 0)
      bco->effect_id_ = BF_SK_HUI_CHUN;
    else if (::strcmp(effect, "fan_tan") == 0)
      bco->effect_id_ = BF_SK_FAN_TAN;
    else if (::strcmp(effect, "hu_dun") == 0)
      bco->effect_id_ = BF_SK_HU_DUN;
    return 0;
  }
  buff_cfg_obj * get_buff_cfg_obj(const int buff_id)
  {
    buff_cfg_obj_map_iter itor = this->buff_cfg_obj_map_.find(buff_id);
    if (itor == this->buff_cfg_obj_map_.end()) return NULL;
    return itor->second; 
  }
private:
  buff_cfg_obj_map_t buff_cfg_obj_map_;
};
// ------------------------------------------------------------------
buff_config::buff_config() : impl_(new buff_config_impl()) { }
int buff_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int buff_config::reload_config(const char *cfg_root)
{
  buff_config_impl *tmp_impl = new buff_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", BUFF_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
buff_cfg_obj *buff_config::get_buff_cfg_obj(const int buff_id)
{ return this->impl_->get_buff_cfg_obj(buff_id); }
