#include "passive_skill_cfg.h"
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

passive_skill_cfg_obj::passive_skill_cfg_obj() :
  career_(0),
  effect_id_(0),
  learn_lvl_(0),
  attr_val_(0)
{ }
/**
 * @class passive_skill_cfg_impl
 * 
 * @brief implement of passive_skill_cfg
 */
class passive_skill_cfg_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*cid*/, passive_skill_cfg_obj *> passive_skill_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*cid*/, passive_skill_cfg_obj *>::iterator passive_skill_cfg_obj_map_itor;

  passive_skill_cfg_impl() :
    passive_skill_cfg_obj_map_(64),
    career_skill_map_(CHAR_CAREER_CNT + 1)
  { }
  ~passive_skill_cfg_impl()
  {
    for (passive_skill_cfg_obj_map_itor itor = this->passive_skill_cfg_obj_map_.begin();
         itor != this->passive_skill_cfg_obj_map_.end();
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
    if (this->load_json_cfg(cfg_root, PASSIVE_SKILL_CFG_PATH, root) != 0)
      return -1;

    for (int i = CAREER_LI_LIANG; i <= CAREER_ZHI_LI; ++i)
      this->career_skill_map_.insert(i, new ilist<int>());

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int career = ::atoi(iter.key().asCString());
      if (!util::is_career_ok(career))
        return -1;

      for (Json::Value::iterator iter2 = (*iter).begin(); 
           iter2 != (*iter).end();
           ++iter2)
      {
        int cid = ::atoi(iter2.key().asCString());
        int effect = (*iter2)["effect"].asInt();
        int initial_val = (*iter2)["initial_val"].asInt();
        int lvl = (*iter2)["lvl"].asInt();
        if (!clsid::is_skill(cid)
            || (effect < ATTR_T_HP
                || effect >= ATTR_T_ITEM_CNT)
            || initial_val < 0)
          return -1;

        passive_skill_cfg_obj *p = new passive_skill_cfg_obj();
        p->career_ = career;
        p->learn_lvl_ = lvl;
        p->effect_id_ = effect;
        p->attr_val_ = initial_val;
        this->passive_skill_cfg_obj_map_.insert(std::make_pair(cid, p));
        this->career_skill_map_.find(career)->push_back(cid);
      }
    }
    return 0;
  }
  const passive_skill_cfg_obj *get_skill(const int skill_cid)
  {
    passive_skill_cfg_obj_map_itor itor = this->passive_skill_cfg_obj_map_.find(skill_cid);
    if (itor == this->passive_skill_cfg_obj_map_.end()) return NULL; 
    return itor->second;
  }
  ilist<int> *get_skill_list(const int career)
  { return this->career_skill_map_.find(career); }
private:
  passive_skill_cfg_obj_map_t passive_skill_cfg_obj_map_;
  array_t<ilist<int>* > career_skill_map_;
};
passive_skill_cfg::passive_skill_cfg() : impl_(new passive_skill_cfg_impl()) { }
int passive_skill_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int passive_skill_cfg::reload_config(const char *cfg_root)
{
  passive_skill_cfg_impl *tmp_impl = new passive_skill_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", PASSIVE_SKILL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const passive_skill_cfg_obj *passive_skill_cfg::get_skill(const int skill_cid)
{ return this->impl_->get_skill(skill_cid); }
ilist<int> *passive_skill_cfg::get_skill_list(const int career)
{ return this->impl_->get_skill_list(career); }
