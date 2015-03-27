// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-27 15:25
 */
//========================================================================

#ifndef TIMER_HEAP_H_
#define TIMER_HEAP_H_

#include "time_value.h"

// Forward declarations
class ev_handler;

/**
 * @class timer_node
 *
 * @brief 
 */
class timer_node
{
public:
  timer_node() :
    timer_id_(-1),
    handler_(NULL),
    prev_(NULL),
    next_(NULL),
    timer_value_(0, 0),
    interval_(0, 0)
  { }

  timer_node & operator = (const timer_node &tn)
  {
    if (this == &tn) return *this;
    this->timer_id_     = tn.timer_id_;
    this->handler_      = tn.handler_;
    this->prev_	        = tn.prev_;
    this->next_	        = tn.next_;
    this->timer_value_  = tn.timer_value_;
    this->interval_     = tn.interval_;
    return *this;
  }
public:
  int timer_id_;

  ev_handler *handler_;

  timer_node *prev_;
  timer_node *next_;

  time_value timer_value_;
  time_value interval_;
};
/**
 * @class timer_heap
 * 
 * @brief 
 */
class timer_heap
{
public:
  timer_heap(const int max_size, const int pre_alloc_num);

  // return the number of timer node working now..
  inline int curr_size() const { return this->curr_size_; }

  // eh->timer_id(xx) will be called on ok.
  int schedule(ev_handler *eh,
               const time_value &delay_time,	
               const time_value &interval_time);

  int cancel(const int timer_id, const int dont_call_handle_close);

  inline const time_value* calculate_timeout()
  {
    if (this->curr_size_ == 0) return NULL;

    const time_value cur_time = time_value::gettimeofday();
    if (this->timer_heap_[0]->timer_value_ > cur_time)
    {
      this->timeout_ = this->timer_heap_[0]->timer_value_ - cur_time;
      return &this->timeout_;
    }
    return &time_value::zero;
  }

  void expire();
private:
  int grow_heap(void);
  
  void insert(timer_node *new_node);
  timer_node *remove(const int slot);

  timer_node *alloc_node();
  void free_node(timer_node *node);
  int alloc_timerid();
  void release_timerid(const int id);
  
  int dispatch_timer(const time_value &current_time, timer_node &dispatched_node);

  void reheap_up(timer_node *moved_node, int slot, int parent);
  void reheap_down(timer_node *moved_node, int slot, int lchild);
  inline void copy(const int slot, timer_node *moved_node)
  {
    this->timer_heap_[slot] = moved_node;
    this->timer_ids_[moved_node->timer_id_] = slot;
  }

  // 
  timer_heap(const timer_heap &);
  void operator = (const timer_heap &); 
private:
  int max_size_;
  int curr_size_;
  int timer_heap_size_;
  int timer_ids_size_;
  int min_timer_id_;

  timer_node **timer_heap_;
  timer_node *free_timer_nodes_;

  int *timer_ids_;

  time_value timeout_;
};

#endif // TIMER_HEAP_H_

