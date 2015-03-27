#include "fighting_monster_obj.h"
#include "monster_cfg.h"
#include "scene_config.h"
#include "spawn_monster.h"
#include "fighter_mgr.h"
#include "sys_log.h"
#include "util.h"

// Lib header
#include "singleton.h"
#include "obj_pool.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

/**
 * @class ai_info_pool
 * 
 * @brief
 */
class ai_info_pool : public singleton<ai_info_pool>
{
  friend class singleton<ai_info_pool>;
public:
  ai_info *alloc() { return this->pool_.alloc(); }

  void release(ai_info *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  ai_info_pool() { }

  obj_pool<ai_info, obj_pool_std_allocator<ai_info> > pool_;
};
static void destroy_ai_list_i(ilist<ai_info *> *&ai_list)
{
  if (ai_list != NULL)
  {
    while (!ai_list->empty())
    {
      ai_info *info = ai_list->pop_front();
      ai_info_pool::instance()->release(info);
    }
    delete ai_list;
    ai_list = NULL;
  }
}
void fighting_monster_obj::destroy_ai_list()
{
  destroy_ai_list_i(this->birth_ai_list_);
  destroy_ai_list_i(this->attack_ai_list_);
  destroy_ai_list_i(this->back_ai_list_);
  destroy_ai_list_i(this->die_ai_list_);
}
static void load_ai(ilist<ai_info *> &cfg_ai_list, ilist<ai_info *> *&ai_list)
{
  if (!cfg_ai_list.empty())
  {
    ai_list = new ilist<ai_info *>();
    for (ilist_node<ai_info *> *ai_p = cfg_ai_list.head();
         ai_p != NULL;
         ai_p = ai_p->next_)
    {
      ai_info *info = ai_p->value_;
      ai_info *p = ai_info_pool::instance()->alloc();
      ::memcpy((char *)p, info, sizeof(ai_info));
      ai_list->push_back(p);
    }
  }
}
void fighting_monster_obj::do_load_ai(monster_cfg_obj *cfg)
{
  this->destroy_ai_list();

  load_ai(cfg->birth_ai_list_, this->birth_ai_list_);
  load_ai(cfg->attack_ai_list_, this->attack_ai_list_);
  load_ai(cfg->back_ai_list_, this->back_ai_list_);
  load_ai(cfg->die_ai_list_, this->die_ai_list_);
  return ;
}
void fighting_monster_obj::do_ai(ilist<ai_info *> *ai_list,
                                 const time_value &now,
                                 const bool direct_do)
{
  if (ai_list == NULL) return ;
  if (!direct_do)
  {
    if (now < this->next_check_ai_time_) return ;
    this->next_check_ai_time_ = now + time_value(0, 600*1000);
  }

  int lsize = ai_list->size();
  for (int i = 0; i < lsize; ++i)
  {
    ai_info *info = ai_list->pop_front();
    if (this->dispatch_ai_if(info, now) != 0)
      ai_info_pool::instance()->release(info);
    else
      ai_list->push_back(info);
  }
  return ;
}
int fighting_monster_obj::dispatch_ai_if(ai_info *info, const time_value &now)
{
  switch (info->if_type_)
  {
  case IF_HP_LT:
    if (this->hp() * 100 / this->total_hp() < info->if_arg_)
      return this->dispatch_ai_do(info, now);
    return 0;
  case IF_TRUE:
    return this->dispatch_ai_do(info, now);
  case IF_TIME_AT:
    if (time_util::now - this->first_be_attacked_time_ > info->if_arg_)
      return this->dispatch_ai_do(info, now);
    return 0;
  default:
    e_log->wning("not support ai if type %d", info->if_type_);
    break;
  }
  return -1;
}
int fighting_monster_obj::dispatch_ai_do(ai_info *info, const time_value &now)
{
  switch (info->do_type_)
  {
  case DO_USE_SKILL:
    return this->ai_do_use_skill(info, now);
  case DO_CALL_MST:
    return this->ai_do_call_mst(info, now);
  case DO_LEARN_SKILL:
    return this->ai_do_learn_skill(info, now);
  case DO_RECOVER_SM:
    return this->ai_do_recover_hp(info, now);
  default:
    e_log->wning("not support ai do type %d", info->do_type_);
    break;
  }
  return -1;
}
int fighting_monster_obj::ai_do_use_skill(ai_info *info, const time_value &now)
{
  const skill_cfg_obj *sco = skill_config::instance()->get_skill(info->do_arg_[0]);
  if (sco == NULL) return -1;
  const skill_detail *sd = sco->get_detail(info->do_arg_[1]);
  if (sd == NULL) return -1;

  // do_attack 中已经检查过是否能使用技能
  if (this->do_use_skill(this->att_obj_id_,
                         now,
                         this->coord_x_,
                         this->coord_y_,
                         info->do_arg_[0],
                         sco->hurt_delay_,
                         sd) == 0)
    this->on_use_skill_ok(NULL, sd, sco->hurt_delay_, now);

  return -1;
}
int fighting_monster_obj::ai_do_call_mst(ai_info *info, const time_value &)
{
  for (int i = 0; i < info->do_arg_[1]; ++i)
  {
    coord_t pos = scene_config::instance()->get_random_pos(this->scene_cid_,
                                                           this->coord_x_,
                                                           this->coord_y_,
                                                           info->do_arg_[2]);
    spawn_monster::spawn_one(info->do_arg_[0],
                             500,
                             this->scene_id_,
                             this->scene_cid_,
                             DIR_XX,
                             pos.x_,
                             pos.y_);
  }
  return -1;
}
int fighting_monster_obj::ai_do_learn_skill(ai_info *info, const time_value &)
{
  for (ilist_node<skill_info *> *itor = this->skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == info->do_arg_[0])
      return -1;
  }
  skill_info *si   = skill_info_pool::instance()->alloc();
  si->cid_         = info->do_arg_[0];
  si->lvl_         = info->do_arg_[1];
  si->rate_        = info->do_arg_[2];
  this->skill_list_.push_back(si);
  return -1;
}
int fighting_monster_obj::ai_do_recover_hp(ai_info *info, const time_value &)
{
  if (this->do_add_hp((int)((double)this->obj_attr_.total_hp() * (double)info->do_arg_[0] / 100.0)) != 0)
    this->broadcast_hp_mp();
  return -1;
}
