#include "monster_cfg.h"
#include "skill_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "util.h"

// Lib header
#include <tr1/unordered_map>

// monster config log
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

monster_cfg_obj::monster_cfg_obj() :
  cid_(0)
{ }
monster_cfg_obj::~monster_cfg_obj()
{
  while (!this->birth_ai_list_.empty())
  {
    ai_info *info = this->birth_ai_list_.pop_front();
    delete info;
  }
  while (!this->attack_ai_list_.empty())
  {
    ai_info *info = this->attack_ai_list_.pop_front();
    delete info;
  }
  while (!this->die_ai_list_.empty())
  {
    ai_info *info = this->die_ai_list_.pop_front();
    delete info;
  }
}
/**
 * @class monster_cfg_impl
 *
 * @brief
 */
class monster_cfg_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int, monster_cfg_obj *> mst_obj_map_t;
  typedef std::tr1::unordered_map<int, monster_cfg_obj *>::iterator mst_obj_map_itor;

  monster_cfg_impl() : mst_obj_map_(2048) { }
  ~monster_cfg_impl()
  {
    for (mst_obj_map_itor itor = this->mst_obj_map_.begin();
         itor != this->mst_obj_map_.end();
         ++itor)
      delete itor->second;
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, MONSTER_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int cid = ::atoi(iter.key().asCString());
      monster_cfg_obj *mco = new monster_cfg_obj();
      this->mst_obj_map_.insert(std::make_pair(cid, mco));
      if (this->load_json(mco, *iter, cid) != 0)
        return -1;
    }
    return 0;
  }

  int load_json(monster_cfg_obj *mco, Json::Value &root, const int cid)
  {
    mco->cid_ = cid;
    mco->career_ = root["career"].asInt();
    mco->block_radius_ = root["block_radius"].asInt();
    mco->att_type_ = root["attack_type"].asInt();
    mco->mst_type_ = root["monster_type"].asInt();
    mco->sort_ = root["monster_sort"].asInt();
    mco->lvl_ = root["lvl"].asInt();
    mco->base_skill_ = root["base_skill"].asInt();
    if (skill_config::instance()->get_detail(mco->base_skill_, 1) == NULL)
    {
      e_log->rinfo("monster %d base skill is error!", cid);
      return -1;
    }
    mco->exp_ = root["exp"].asInt();
    mco->patrol_radius_ = root["patrol_radius"].asInt();
    if (mco->patrol_radius_ > 15)
      e_log->wning("monster %d patrol_radius %d too big!", cid, mco->patrol_radius_);
    mco->patrol_speed_ = root["patrol_speed"].asInt();
    mco->patrol_interval_ = root["patrol_interval"].asInt();
    mco->common_cd_ = root["common_cd"].asInt();
    mco->fight_back_delay_ = root["fight_back_delay"].asInt();
    mco->eye_angle_ = root["eye_angle"].asInt();
    mco->eye_radius_ = root["eye_radius"].asInt();
    mco->chase_radius_ = root["chase_radius"].asInt();
    mco->chase_speed_ = root["chase_speed"].asInt();
    mco->refresh_time_ = root["refresh_time"].asInt();
    mco->stiff_my_time_ = root["stiff_my_time"].asInt();
    mco->live_time_ = root["live_time"].asInt();
    mco->change_target_ = root["change_target"].asBool();
    mco->hp_ = root["sheng_ming"].asInt();
    mco->gong_ji_ = root["gong_ji"].asInt();
    mco->fang_yu_ = root["fang_yu"].asInt();
    mco->param_ = root["param"].asInt();

    Json::Value &birth_ai_v = root["birth_ai"];
    if (!birth_ai_v.empty())
    {
      if (this->load_ai(mco->birth_ai_list_, cid, birth_ai_v.asString()) != 0)
        return -1;
    }
    Json::Value &attack_ai_v = root["attack_ai"];
    if (!attack_ai_v.empty())
    {
      if (this->load_ai(mco->attack_ai_list_, cid, attack_ai_v.asString()) != 0)
        return -1;
    }
    Json::Value &back_ai_v = root["back_ai"];
    if (!back_ai_v.empty())
    {
      if (this->load_ai(mco->back_ai_list_, cid, back_ai_v.asString()) != 0)
        return -1;
    }
    Json::Value &die_ai_v = root["die_ai"];
    if (!die_ai_v.empty())
    {
      if (this->load_ai(mco->die_ai_list_, cid, die_ai_v.asString()) != 0)
        return -1;
    }
    return 0;
  }
  int load_ai(ilist<ai_info *> &ai_list, const int cid, const std::string &s_ai)
  {
    if (s_ai.length() > 1024)
    {
      e_log->wning("%d ai string is too long [%s]!", cid, s_ai.c_str());
      return -1;
    }
    char bf[1024] = {0};
    util::strstrip_all(s_ai.c_str(), bf, sizeof(bf));

    char *tok_p = NULL;
    char *token = NULL;
    for(token = ::strtok_r(bf, ";", &tok_p);
        token != NULL;
        token = ::strtok_r(NULL, ";", &tok_p))
    {
      ai_info info;
      if (this->load_if_ai(token, info, cid) != 0) return -1;
      if (this->load_do_ai(token, info, cid) != 0) return -1;
      if (info.do_type_ != -1)
      {
        ai_info *ai_p = new ai_info();
        ::memcpy(ai_p, &info, sizeof(ai_info));
        if (info.if_type_ == AI_IF_XX_CNT)
          ai_p->if_type_ = IF_TRUE;
        ai_list.push_back(ai_p);
      }
    }
    return 0;
  }
  int load_if_ai(char *token, ai_info &info, const int cid)
  {
    char *if_p = NULL;
    if ((if_p = ::strstr(token, "if_sm_lt")) != NULL)
    {
      info.if_type_ = IF_HP_LT;
      char *if_arg_p = if_p + sizeof("if_sm_lt") - 1;
      if (::sscanf(if_arg_p, "(%d)", &(info.if_arg_)) != 1)
      {
        e_log->error("monster %d ai[%s] error!", cid, token);
        return -1;
      }
    }else if ((if_p = ::strstr(token, "if_time_at")) != NULL)
    {
      info.if_type_ = IF_TIME_AT;
      char *if_arg_p = if_p + sizeof("if_time_at") - 1;
      if (::sscanf(if_arg_p, "(%d)", &(info.if_arg_)) != 1)
      {
        e_log->error("monster %d ai[%s] error!", cid, token);
        return -1;
      }
    }
    return 0;
  }
  int load_do_ai(char *token, ai_info &info, const int cid)
  {
    char *do_p = NULL;
    if ((do_p = ::strstr(token, "do_use_skill")) != NULL)
    {
      info.do_type_  = DO_USE_SKILL;
      char *do_arg_p = do_p + sizeof("do_use_skill") - 1;
      if (::sscanf(do_arg_p, "(%d,%d)", &(info.do_arg_[0]), &(info.do_arg_[1])) != 2)
      {
        e_log->error("monster %d ai:do_use_skill arg error!", cid, token);
        return -1;
      }
      const skill_detail *sd = skill_config::instance()->get_detail(info.do_arg_[0],
                                                                    info.do_arg_[1]);
      if (sd == NULL || sd->distance_ > 0)
      {
        e_log->error("monster %d ai:do_use_skill skill is invalid!", cid);
        return -1;
      }
    }else if ((do_p = ::strstr(token, "do_call_mst")) != NULL)
    {
      info.do_type_  = DO_CALL_MST;
      char *do_arg_p = do_p + sizeof("do_call_mst") - 1;
      if (::sscanf(do_arg_p, "(%d,%d)",
                   &(info.do_arg_[0]), &(info.do_arg_[1])) != 2)
      {
        e_log->error("monster %d ai:do_call_mst arg error!", cid);
        return -1;
      }
    }else if ((do_p = ::strstr(token, "do_recover_sm")) != NULL)
    {
      info.do_type_  = DO_RECOVER_SM;
      char *do_arg_p = do_p + sizeof("do_recover_sm") - 1;
      if (::sscanf(do_arg_p, "(%d)", &(info.do_arg_[0])) != 1)
      {
        e_log->error("monster %d ai:do_recover_sm arg error!", cid);
        return -1;
      }
    }else if ((do_p = ::strstr(token, "do_learn_skill")) != NULL)
    {
      info.do_type_  = DO_LEARN_SKILL;
      char *do_arg_p = do_p + sizeof("do_learn_skill") - 1;
      if (::sscanf(do_arg_p, "(%d,%d,%d)",
                   &(info.do_arg_[0]), &(info.do_arg_[1]), &(info.do_arg_[2])) != 3)
      {
        e_log->error("monster %d ai:do_learn_skill arg error!", cid);
        return -1;
      }
      if (skill_config::instance()->get_detail(info.do_arg_[0], info.do_arg_[1]) == NULL)
      {
        e_log->error("monster %d ai:do_learn_skill skill is invalid!", cid);
        return -1;
      }
    }else
    {
      e_log->error("monster %d ai:%s is invalid!", token);
      return -1;
    }
    return 0;
  }
  monster_cfg_obj *get_monster_cfg_obj(const int cid)
  {
    mst_obj_map_itor itor = this->mst_obj_map_.find(cid);
    if (itor == this->mst_obj_map_.end()) return NULL;
    return itor->second;
  }
private:
  mst_obj_map_t mst_obj_map_;
};
// ------------------------------- monster config ------------------------------
monster_cfg::monster_cfg() : impl_(new monster_cfg_impl()) { }
int monster_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 
int monster_cfg::reload_config(const char *cfg_root)
{
  monster_cfg_impl *tmp_impl = new monster_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", MONSTER_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
monster_cfg_obj* monster_cfg::get_monster_cfg_obj(const int cid)
{ return this->impl_->get_monster_cfg_obj(cid); }
