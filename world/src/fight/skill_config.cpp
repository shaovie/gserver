#include "skill_config.h"
#include "buff_config.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"
#include "clsid.h"
#include "util.h"
#include "def.h"

// Lib header
#include <tr1/unordered_map>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

skill_cfg_obj::skill_cfg_obj() :
  skill_cid_(0),
  career_(0),
  hurt_delay_(500)
{
  details_ = new skill_detail*[MAX_SKILL_LEVEL];
  for (int i = 0; i < MAX_SKILL_LEVEL; ++i)
    details_[i] = NULL;
}
skill_cfg_obj::~skill_cfg_obj()
{
  for (int i = 0; i < MAX_SKILL_LEVEL; ++i)
    delete this->details_[i];
  delete []this->details_;
}
/**
 * @class skill_config_impl
 * 
 * @brief implement of skill_config
 */
class skill_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int, skill_cfg_obj *> skill_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int, skill_cfg_obj *>::iterator skill_cfg_obj_map_itor;

  skill_config_impl()
    : skill_cfg_obj_map_(256),
    career_skill_map_(CHAR_CAREER_CNT + 1)
  { }
  ~skill_config_impl()
  {
    for (skill_cfg_obj_map_itor itor = this->skill_cfg_obj_map_.begin();
         itor != this->skill_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
    for (int i = 0; i < this->career_skill_map_.size(); ++i)
    {
      ilist<int>* l = this->career_skill_map_.find(i);
      if (l != NULL) delete l;
    }
  }
  //
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SKILL_CFG_PATH, root) != 0)
      return -1;

    for (int i = CAREER_LI_LIANG; i <= CAREER_ZHI_LI; ++i)
      this->career_skill_map_.insert(i, new ilist<int>());

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      if (!clsid::is_skill(cid))
      {
        e_log->error("%d is not skill in %s!", cid, SKILL_CFG_PATH);
        return -1;
      }
      skill_cfg_obj *sco = new skill_cfg_obj();
      this->skill_cfg_obj_map_.insert(std::make_pair(cid, sco));
      if (this->load_json(*iter, sco, cid) != 0)
        return -1;

      if (clsid::is_char_skill(cid)
          && util::is_career_ok(sco->career_))
        this->career_skill_map_.find(sco->career_)->push_back(cid);
    }
    return 0;
  }
  int load_json(Json::Value &root, skill_cfg_obj *sco, const int skill_cid)
  {
    sco->skill_cid_ = skill_cid;
    sco->career_ = root["career"].asInt();
    sco->hurt_delay_ = root["hurt_delay"].asInt();
    Json::Value &detail_v = root["info"];
    if (detail_v.empty()) return -1;

    for (Json::Value::iterator iter = detail_v.begin();
         iter != detail_v.end();
         ++iter)
    {
      int i = ::atoi(iter.key().asCString());
      Json::Value &dv = *iter;
      if (!dv.empty())
      {
        if (i < 1 || i >= MAX_SKILL_LEVEL)
        {
          e_log->error("skill lvl is invalid! lvl = %d", i);
          return -1;
        }

        skill_detail *sd = new skill_detail();
        sd->cur_lvl_ = i;
        sco->details_[i] = sd;
        if (this->load_json(dv, sd) != 0)
          return -1;
      }
    }
    if (sco->get_detail(1) == NULL)
    {
      e_log->error("skill %d not found lvl 1 info!", skill_cid);
      return -1;
    }
    return 0;
  }
  int load_json(Json::Value &root, skill_detail *sd)
  {
    sd->distance_ = root["distance"].asInt();
    sd->back_dis_ = root["back_dis"].asInt();
    sd->lvl_ = root["level"].asInt();
    sd->mp_ = root["fa_li"].asInt();
    sd->cd_ = root["cd"].asInt();
    sd->hate_coe_ = root["hate_coe"].asInt();
    sd->coin_ = root["coin"].asInt();
    sd->hurt_percent_ = root["hurt_percent"].asInt();
    sd->add_fixed_hurt_ = root["add_fixed_hurt"].asInt();
    sd->param_1_ = root["param1"].asInt();
    sd->param_2_ = root["param2"].asInt();
    sd->param_3_ = root["param3"].asInt();
    Json::Value &buff_info_v = root["buf_info"];
    if (!buff_info_v.empty())
    {
      char bf[256] = {0};
      ::strncpy(bf, buff_info_v.asCString(), sizeof(bf));

      char *tok_p = NULL;
      char *token = NULL;
      for (token = ::strtok_r(bf, ";", &tok_p);
           token != NULL;
           token = ::strtok_r(NULL, ";", &tok_p))
      {
        sk_rel_buff_info srbi;
        if (::sscanf(token, "%d:%d:%d", &(srbi.id_), &(srbi.rate_), &(srbi.target_type_)) != 3)
          return -1;
        if (buff_config::instance()->get_buff_cfg_obj(srbi.id_) == NULL)
        {
          e_log->rinfo("buff %d not found in buff_config when load skill_config", srbi.id_);
          return -1;
        }
        sd->buff_info_.push_back(srbi);
      }
    }
    return 0;
  }
  const skill_detail *get_detail(const int skill_cid, const short lvl)
  {
    skill_cfg_obj_map_itor itor = this->skill_cfg_obj_map_.find(skill_cid);
    if (itor == this->skill_cfg_obj_map_.end()) return NULL; 
    return itor->second->get_detail(lvl);
  }
  const skill_cfg_obj *get_skill(const int skill_cid)
  {
    skill_cfg_obj_map_itor itor = this->skill_cfg_obj_map_.find(skill_cid);
    if (itor == this->skill_cfg_obj_map_.end()) return NULL; 
    return itor->second;
  }
  ilist<int> *get_skill_list(const int career)
  { return this->career_skill_map_.find(career); }
private:
  skill_cfg_obj_map_t skill_cfg_obj_map_;
  array_t<ilist<int>* > career_skill_map_;
};
skill_config::skill_config() : impl_(new skill_config_impl()) { }
int skill_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int skill_config::reload_config(const char *cfg_root)
{
  skill_config_impl *tmp_impl = new skill_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", SKILL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const skill_cfg_obj *skill_config::get_skill(const int skill_cid)
{ return this->impl_->get_skill(skill_cid); }
const skill_detail *skill_config::get_detail(const int skill_cid, const short lvl)
{ return this->impl_->get_detail(skill_cid, lvl); }
ilist<int> *skill_config::get_skill_list(const int career)
{ return this->impl_->get_skill_list(career); }
