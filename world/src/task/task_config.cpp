#include "task_config.h"
#include "load_json.h"
#include "global_macros.h"
#include "item_config.h"
#include "monster_cfg.h"
#include "task_info.h"
#include "array_t.h"
#include "sys_log.h"
#include "util.h"
#include "sys.h"

// Lib header
#include <tr1/unordered_map>
#include <map>

// task config log
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

static int s_guild_task_done_cnt = 0;
static int s_daily_task_done_cnt = 0;

task_cfg_obj::task_cfg_obj(int task_cid) :
  task_cid_(task_cid),
  pre_task_cid_(0),
  accept_npc_cid_(0),
  sub_npc_cid_(0),
  accept_if_ai_(NULL),
  accept_do_ai_(NULL),
  target_to_ai_(NULL),
  complete_do_ai_(NULL),
  award_do_ai_(NULL)
{ }
static void releast_ai_list(task_ai_list_t *p_ai_list)
{
  while (p_ai_list != NULL && !p_ai_list->empty())
  {
    task_ai_info *tai = p_ai_list->pop_front();
    delete tai;
  }
  if (p_ai_list != NULL) delete p_ai_list;
}
task_cfg_obj::~task_cfg_obj()
{
  releast_ai_list(this->accept_if_ai_);
  releast_ai_list(this->accept_do_ai_);
  releast_ai_list(this->target_to_ai_);
  releast_ai_list(this->complete_do_ai_);
  releast_ai_list(this->award_do_ai_);
}
/**
 * @class task_config_impl
 *
 * @brief
 */
class task_config_impl : public load_json
{
public:
  typedef std::tr1::unordered_map<int/*task_cid*/, task_cfg_obj *> task_cfg_obj_map_t;
  typedef std::tr1::unordered_map<int/*task_cid*/, task_cfg_obj *>::iterator task_cfg_obj_map_iter;

  task_config_impl() : task_cfg_obj_map_(1024) { }
  ~task_config_impl()
  {
    for (task_cfg_obj_map_iter itor = this->task_cfg_obj_map_.begin();
         itor != this->task_cfg_obj_map_.end();
         ++itor)
      delete itor->second;
  }
  int load_config(const char *cfg_root)
  {
    if (this->load_task_config(cfg_root) != 0)
      return -1;

    return 0;
  }
  int load_task_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TASK_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int task_cid = atoi(iter.key().asCString());
      if (TASK_TYPE(task_cid) == TRUNK_TASK
          || TASK_TYPE(task_cid) == BRANCH_TASK)
      {
        if (TASK_BIT_IDX(task_cid) >= HALF_TASK_NUM)
        {
          e_log->error("task_cid too large %d", task_cid);
          return -1;
        }
      }
      task_cfg_obj *tco = new task_cfg_obj(task_cid);
      this->task_cfg_obj_map_.insert(std::make_pair(task_cid, tco));
      if (this->load_task_cfg_obj_json(*iter, tco) != 0)
        return -1;
      this->id_list_.push_back(task_cid);
    }
    if (this->id_list_.size() >= MAX_TASK_NUM)
    {
      e_log->error("task too much %d", this->id_list_.size());
      return -1;
    }
    return 0;
  }
  int load_task_cfg_obj_json(Json::Value &root, task_cfg_obj *tco)
  {
    tco->pre_task_cid_ = root["pre_task_cid"].asInt();

    Json::Value &pre_v = root["pre_accept_ai"];
    if (!pre_v.empty())
    {
      tco->accept_if_ai_ = new task_ai_list_t();
      if (this->parse_if_ai_list(pre_v.asString(),
                                 tco->accept_if_ai_,
                                 tco->task_cid_) != 0)
        return -1;
    }

    tco->accept_npc_cid_ = root["accept_npc"].asInt();

    Json::Value &accept_ai_v = root["accept_ai"];
    if (!accept_ai_v.empty())
    {
      tco->accept_do_ai_ = new task_ai_list_t();
      if (this->parse_do_ai_list(accept_ai_v.asString(),
                                 tco->accept_do_ai_,
                                 tco->task_cid_) != 0)
        return -1;
    }

    Json::Value &target_ai_v = root["target"];
    if (!target_ai_v.empty())
    {
      tco->target_to_ai_ = new task_ai_list_t();
      if (this->parse_to_ai_list(target_ai_v.asString(),
                                 tco->target_to_ai_,
                                 tco->task_cid_) != 0)
        return -1;
    }

    tco->sub_npc_cid_ = root["complete_npc"].asInt();

    Json::Value &complete_ai_v = root["complete_ai"];
    if (!complete_ai_v.empty())
    {
      tco->complete_do_ai_ = new task_ai_list_t();
      if (this->parse_do_ai_list(complete_ai_v.asString(),
                                 tco->complete_do_ai_,
                                 tco->task_cid_) != 0)
        return -1;
    }
    Json::Value &award_ai_v = root["award"];
    if (!award_ai_v.empty())
    {
      tco->award_do_ai_ = new task_ai_list_t();
      if (this->parse_do_ai_list(award_ai_v.asString(),
                                 tco->award_do_ai_,
                                 tco->task_cid_) != 0)
        return -1;
    }
    return 0;
  }
  int parse_if_ai_list(const std::string &s_ai, task_ai_list_t *p_ai_list, const int task_cid)
  {
    char bf[1024] = {0};
    if (s_ai.length() >= sizeof(bf))
    {
      e_log->rinfo("task ai [%s] too large!", s_ai.c_str());
      return -1;
    }
    util::strstrip_all(s_ai.c_str(), bf, sizeof(bf));
    char *tok_p = NULL;
    char *token = NULL;
    for (token = ::strtok_r(bf, ";", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ";", &tok_p))
    {
      task_ai_info info;
      if (this->parse_if_ai_info(token, info, task_cid) != 0)
        return -1;
      task_ai_info *ai_p = new task_ai_info();
      ::memcpy(ai_p, &info, sizeof(task_ai_info));
      p_ai_list->push_back(ai_p);
    }
    return 0;
  }
  int parse_if_ai_info(const char *token, task_ai_info &info, const int task_cid)
  {
    char *if_p = NULL;
    if ((if_p = ::strstr((char*)token, "if_in_level")) != NULL)
    {
      char *if_arg_p = if_p + sizeof("if_in_level") - 1;
      if (::sscanf(if_arg_p, "(%d,%d)", &(info.args_[0]), &(info.args_[1])) != 2)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = IF_IN_LEVEL;
    }else if ((if_p = ::strstr((char*)token, "if_in_guild")) != NULL)
    {
      info.type_ = IF_IN_GUILD;
    }else if ((if_p = ::strstr((char*)token, "if_daily_task_done_cnt_lt")) != NULL)
    {
      char *if_arg_p = if_p + sizeof("if_daily_task_done_cnt_lt") - 1;
      if (::sscanf(if_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = IF_DAILY_TASK_DONE_CNT_LT;
      s_daily_task_done_cnt = info.args_[0];
    }else if ((if_p = ::strstr((char*)token, "if_guild_task_done_cnt_lt")) != NULL)
    {
      char *if_arg_p = if_p + sizeof("if_guild_task_done_cnt_lt") - 1;
      if (::sscanf(if_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = IF_GUILD_TASK_DONE_CNT_LT;
      s_guild_task_done_cnt = info.args_[0];
    }else if ((if_p = ::strstr((char*)token, "if_no_task")) != NULL)
    {
      char *if_arg_p = if_p + sizeof("if_no_task") - 1;
      if (::sscanf(if_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = IF_NO_TASK;
    }else
    {
      e_log->rinfo("unknow ai [%s]!", token);
      return -1;
    }
    return 0;
  }
  int parse_do_ai_list(const std::string &s_ai, task_ai_list_t *p_ai_list, const int task_cid)
  {
    char bf[1024] = {0};
    if (s_ai.length() >= sizeof(bf))
    {
      e_log->rinfo("task ai [%s] too large!", s_ai.c_str());
      return -1;
    }
    util::strstrip_all(s_ai.c_str(), bf, sizeof(bf));
    char *tok_p = NULL;
    char *token = NULL;
    for (token = ::strtok_r(bf, ";", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ";", &tok_p))
    {
      task_ai_info info;
      if (this->parse_do_ai_info(token, info, task_cid) != 0)
        return -1;
      task_ai_info *ai_p = new task_ai_info();
      ::memcpy(ai_p, &info, sizeof(task_ai_info));
      p_ai_list->push_back(ai_p);
    }
    return 0;
  }
  int parse_do_ai_info(const char *token, task_ai_info &info, const int task_cid)
  {
    char *do_p = NULL;
    if ((do_p = ::strstr((char*)token, "do_give_item")) != NULL)
    {
      char *args_p = do_p + sizeof("do_give_item") - 1;
      if (::sscanf(args_p, "(%d,%d,%d)", &(info.args_[0]), &(info.args_[1]), &(info.args_[2])) != 3)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      if (!item_config::instance()->find(info.args_[0]))
      {
        e_log->error("[%s] item not found!", token);
        return -1;
      }
      info.type_ = DO_GIVE_ITEM;
    }else if ((do_p = ::strstr((char*)token, "do_give_exp")) != NULL)
    {
      char *args_p = do_p + sizeof("do_give_exp") - 1;
      if (::sscanf(args_p, "(%d)", &(info.args_[0])) != 1
          || info.args_[0] < 0)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = DO_GIVE_EXP;
    }else if ((do_p = ::strstr((char*)token, "do_give_money")) != NULL)
    {
      char *args_p = do_p + sizeof("do_give_money") - 1;
      if (::sscanf(args_p, "(%d,%d)", &(info.args_[0]), &(info.args_[1])) != 2)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = DO_GIVE_MONEY;
    }else if ((do_p = ::strstr((char*)token, "do_give_guild_contrib")) != NULL)
    {
      char *args_p = do_p + sizeof("do_give_guild_contrib") - 1;
      if (::sscanf(args_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = DO_GIVE_GUILD_CONTRIB;
    }else if ((do_p = ::strstr((char*)token, "do_give_guild_money")) != NULL)
    {
      char *args_p = do_p + sizeof("do_give_guild_money") - 1;
      if (::sscanf(args_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = DO_GIVE_GUILD_MONEY;
    }else if ((do_p = ::strstr((char*)token, "do_auto_accept_task")) != NULL)
    {
      char *args_p = do_p + sizeof("do_auto_accept_task") - 1;
      if (::sscanf(args_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      if (TASK_TYPE(info.args_[0]) == TRUNK_TASK)
        return -1;
      info.type_ = DO_AUTO_ACCEPT_TASK;
    }else if ((do_p = ::strstr((char*)token, "do_random_accept_task")) != NULL)
    {
      char *args_p = do_p + sizeof("do_random_accept_task") - 1;
      if (::sscanf(args_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      if (info.args_[0] == TRUNK_TASK)
        return -1;
      info.type_ = DO_RANDOM_ACCEPT_TASK;
    }else
    {
      e_log->rinfo("unknow ai [%s]!", token);
      return -1;
    }
    return 0;
  }
  int parse_to_ai_list(const std::string &s_ai, task_ai_list_t *p_ai_list, const int task_cid)
  {
    char bf[1024] = {0};
    if (s_ai.length() >= sizeof(bf))
    {
      e_log->rinfo("task ai [%s] too large!", s_ai.c_str());
      return -1;
    }
    util::strstrip_all(s_ai.c_str(), bf, sizeof(bf));
    char *tok_p = NULL;
    char *token = NULL;
    int idx = 0;
    for (token = ::strtok_r(bf, ";", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ";", &tok_p))
    {
      task_ai_info info;
      if (this->parse_to_ai_info(token, info, task_cid) != 0)
        return -1;
      info.idx_ = idx++;
      if (info.idx_ >= MAX_TASK_VALUE)
      {
        e_log->rinfo("task %d [%s] too muck todo list!", task_cid, s_ai.c_str());
        return -1;
      }
      task_ai_info *ai_p = new task_ai_info();
      ::memcpy(ai_p, &info, sizeof(task_ai_info));
      p_ai_list->push_back(ai_p);
    }
    return 0;
  }
  int parse_to_ai_info(const char *token, task_ai_info &info, const int task_cid)
  {
    char *to_p = NULL;
    if ((to_p = ::strstr((char *)token, "to_kill_mst")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_kill_mst") - 1;
      if (::sscanf(to_arg_p, "(%d,%d)", &(info.args_[0]), &(info.args_[1])) != 2)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      if (!monster_cfg::instance()->get_monster_cfg_obj(info.args_[0]))
      {
        e_log->error("[%s] monster not found!", token);
        return -1;
      }
      info.type_ = TO_KILL_MST;
    }else if ((to_p = ::strstr((char *)token, "to_finish_mission")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_finish_mission") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_FINISH_MISSION;
      info.args_[1] = 1;
    }else if ((to_p = ::strstr((char *)token, "to_finish_star_mission")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_finish_star_mission") - 1;
      if (::sscanf(to_arg_p, "(%d,%d)", &(info.args_[0]), &(info.args_[1])) != 2)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_FINISH_STAR_MISSION;
      info.args_[2] = 1;
    }else if ((to_p = ::strstr((char *)token, "to_finish_guide")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_finish_guide") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_FINISH_GUIDE;
      info.args_[1] = 1;
    }else if ((to_p = ::strstr((char *)token, "to_up_level")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_up_level") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_UP_LEVEL;
    }else if ((to_p = ::strstr((char *)token, "to_kill_boss")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_kill_boss") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_KILL_BOSS;
    }else if ((to_p = ::strstr((char *)token, "to_kill_char")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_kill_char") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_KILL_CHAR;
    }else if ((to_p = ::strstr((char *)token, "to_equip_strengthen")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_equip_strengthen") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_EQUIP_STRENGTHEN;
    }else if ((to_p = ::strstr((char *)token, "to_upgrade_pskill")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_upgrade_pskill") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_UPGRADE_PSKILL;
    }else if ((to_p = ::strstr((char *)token, "to_guan_gai")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_guan_gai") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_GUAN_GAI;
    }else if ((to_p = ::strstr((char *)token, "to_mo_bai")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_mo_bai") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_MO_BAI;
    }else if ((to_p = ::strstr((char *)token, "to_jing_ji")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_jing_ji") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_JING_JI;
    }else if ((to_p = ::strstr((char *)token, "to_add_friends")) != NULL)
    {
      char *to_arg_p = to_p + sizeof("to_add_friends") - 1;
      if (::sscanf(to_arg_p, "(%d)", &(info.args_[0])) != 1)
      {
        e_log->error("task %d [%s] arg error!", task_cid, token);
        return -1;
      }
      info.type_ = TO_ADD_FRIENDS;
    }else
    {
      e_log->rinfo("unknow ai [%s]!", token);
      return -1;
    }
    return 0;
  }
  const task_cfg_obj *find(const int task_cid)
  {
    task_cfg_obj_map_iter itor = this->task_cfg_obj_map_.find(task_cid);
    if (itor == this->task_cfg_obj_map_.end()) return NULL;
    return itor->second;
  }
  ilist<int> &get_all_task_list()
  { return this->id_list_; }
private:
  task_cfg_obj_map_t task_cfg_obj_map_;
  ilist<int> id_list_;
};
task_config::task_config() : impl_(new task_config_impl()) { }
int task_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int task_config::reload_config(const char *cfg_root)
{
  task_config_impl *tmp_impl = new task_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TASK_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const task_cfg_obj *task_config::find(const int task_cid)
{ return this->impl_->find(task_cid); }
ilist<int> &task_config::get_all_task_list()
{ return this->impl_->get_all_task_list(); }
int task_config::daily_task_done_cnt()
{ return s_daily_task_done_cnt; }
int task_config::guild_task_done_cnt()
{ return s_guild_task_done_cnt; }
// -----------------------------------------------------------------------
/**
 * @class task_guide_cfg_impl
 *
 * @brief
 */
class task_guide_cfg_impl : public load_json
{
public:
  typedef std::map<int/*lvl*/, std::vector<int/*task_cid*/> > lvl_ids_map_t;
  typedef std::map<int/*lvl*/, std::vector<int/*task_cid*/> >::iterator lvl_ids_map_itor;

  task_guide_cfg_impl(): random_task_pool_(TASK_TYPE_CNT) { }
  ~task_guide_cfg_impl()
  {
    for (int i = 0; i < this->random_task_pool_.size(); ++i)
    {
      lvl_ids_map_t *v = this->random_task_pool_.find(i);
      if (v != NULL) delete v;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, TASK_GUIDE_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int task_type = atoi(iter.key().asCString());

      lvl_ids_map_t *p = new lvl_ids_map_t();
      this->random_task_pool_.insert(task_type, p);

      Json::Value &t_root = *iter;
      for (Json::Value::iterator t_itor = t_root.begin();
           t_itor != t_root.end();
           ++t_itor)
      {
        Json::Value &v = *t_itor;
        int lvl = v["lvl"].asInt();
        lvl_ids_map_itor l_itor = p->find(lvl);
        if (l_itor == p->end())
        {
          std::vector<int> tv;
          tv.reserve(16);
          p->insert(std::make_pair(lvl, tv));
          l_itor = p->find(lvl);
        }
        char bf[512] = {0};
        ::strncpy(bf, v["ids"].asCString(), sizeof(bf) - 1);
        char *tok_p = NULL;
        char *token = NULL;
        for (token = ::strtok_r(bf, ",", &tok_p);
             token != NULL;
             token = ::strtok_r(NULL, ",", &tok_p))
        {
          int cid = ::atoi(token);
          if (task_config::instance()->find(cid) == NULL) return -1;
          l_itor->second.push_back(cid);
        }
      }
    }
    return 0;
  }
  int get_random_task(const int type, const short p_lvl)
  {
    lvl_ids_map_t *m = this->random_task_pool_.find(type);
    if (m == NULL) return 0;
    int lvl = p_lvl / 10 * 10;
    lvl_ids_map_itor l_itor = m->find(lvl);
    if (l_itor == m->end() || l_itor->second.empty()) return 0;

    return l_itor->second[rand() % l_itor->second.size()];
  }
private:
  array_t<lvl_ids_map_t *> random_task_pool_;
};
task_guide_cfg::task_guide_cfg() : impl_(new task_guide_cfg_impl()) { }
int task_guide_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int task_guide_cfg::reload_config(const char *cfg_root)
{
  task_guide_cfg_impl *tmp_impl = new task_guide_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", TASK_GUIDE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int task_guide_cfg::get_random_task(const int type, const short p_lvl)
{ return this->impl_->get_random_task(type, p_lvl); }
