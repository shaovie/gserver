#include "wild_boss_config.h"
#include "scene_config.h"
#include "load_json.h"
#include "sys_log.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

class wild_boss_config_impl : public load_json
{
public:
  wild_boss_config_impl()
  { }
  ~wild_boss_config_impl()
  {
    while (!this->wild_boss_list_.empty())
      delete (this->wild_boss_list_.pop_front());
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, WILD_BOSS_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      wild_boss_config::wild_boss *wb = new wild_boss_config::wild_boss();
      this->wild_boss_list_.push_back(wb);
      wb->index_ = id;
      if (this->load_json(*iter, wb) != 0)
        return -1;
    }
    return 0;
  }

  int load_json(Json::Value &v, wild_boss_config::wild_boss *wb)
  {
    wb->monster_cid_ = v["monster_cid"].asInt();
    wb->scene_cid_ = v["scene_cid"].asInt();
    wb->interval_ = v["refresh_time"].asInt();

    Json::Value &coord_v = v["coord"];
    if (coord_v.empty()) return -1;
    
    coord_t coord;
    char bf[256] = {0};
    ::strncpy(bf, coord_v.asCString(), sizeof(bf));
    char *tok_p = NULL;
    char *token = NULL;
    for(token = ::strtok_r(bf, ";", &tok_p);
        token != NULL;
        token = ::strtok_r(NULL, ";", &tok_p))
    {
      int coord_x = 0, coord_y = 0;
      if (::sscanf(token, "%d,%d", &coord_x, &coord_y) != 2)
      {
        e_log->rinfo("coord wrong");
        return -1;
      }
      if (!scene_config::instance()->can_move(wb->scene_cid_, coord_x, coord_y))
      {
        e_log->rinfo("[%d,%d] can not move", coord_x, coord_y);
        return -1;
      }
      coord.set(coord_x, coord_y);
      wb->coord_.push_back(coord);
    }

    return 0;
  }

  ilist_node<wild_boss_config::wild_boss*>* get_head()
  { return this->wild_boss_list_.head(); }
private:
  ilist<wild_boss_config::wild_boss*> wild_boss_list_;
};
// --------------------------------------------------------
wild_boss_config::wild_boss_config() :
  impl_(new wild_boss_config_impl())
{ }
int wild_boss_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int wild_boss_config::reload_config(const char *cfg_root)
{
  wild_boss_config_impl *tmp_impl = new wild_boss_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", WILD_BOSS_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
ilist_node<wild_boss_config::wild_boss*>* wild_boss_config::get_head()
{ return this->impl_->get_head(); }
