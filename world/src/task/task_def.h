// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-03-01 10:11
 */
//========================================================================

#ifndef TASK_DEF_H_
#define TASK_DEF_H_

#include "ilist.h"
#include "obj_pool.h"
#include "task_info.h"
#include "task_bit_array.h"
#include "daily_task_info.h"

#include "singleton.h"
#include <map>

using namespace __gnu_cxx;

class task_todo_info
{
public:
  task_todo_info() { this->reset(); }
  void reset()
  {
    this->type_     = 0;
    this->task_cid_ = 0;
    this->arg_0_    = 0;
    this->arg_1_    = 0;
  }

  int type_;
  int task_cid_;
  int arg_0_;
  int arg_1_;
};
class task_todo_info_pool : public singleton<task_todo_info_pool>
{
  friend class singleton<task_todo_info_pool>;
public:
  task_todo_info* alloc() { return this->pool_.alloc(); }
  void release(task_todo_info *p) { p->reset(); this->pool_.release(p); }

private:
  task_todo_info_pool() { }
  obj_pool<task_todo_info, obj_pool_std_allocator<task_todo_info> > pool_;
};

typedef ilist<task_todo_info*> task_todo_list_t;
typedef ilist_node<task_todo_info* >* task_todo_list_itor;

typedef std::map<int/*task_cid*/, task_info*> task_info_map_t;
typedef std::map<int/*task_cid*/, task_info*>::iterator task_info_map_itor;

class task_data
{
public:
  task_data() { }
  ~task_data()
  {
    for (task_todo_list_itor l_itor = this->task_todo_list_.head();
         l_itor != NULL;
         l_itor = l_itor->next_)
      task_todo_info_pool::instance()->release(l_itor->value_);
    
    task_info_map_itor m_itor = this->task_info_map_.begin();
    for (; m_itor != this->task_info_map_.end(); ++m_itor)
      task_info_pool::instance()->release(m_itor->second);

    while (!this->daily_task_list_.empty())
      delete this->daily_task_list_.pop_front();
  }

  task_bit_array          bit_array_;
  task_todo_list_t        task_todo_list_;            // 监听
  task_info_map_t         task_info_map_;             // 已接列表
  ilist<int>              task_acceptable_list_;      // 可接任务列表
  ilist<daily_task_info *> daily_task_list_;
};

#endif // TASK_DEF_H_
