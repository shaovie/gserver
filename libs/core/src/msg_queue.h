// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 09:44
 */
//========================================================================

#ifndef MSG_QUEUE_H_
#define MSG_QUEUE_H_

#include "guard.h"
#include "mblock.h"
#include "condition.h"
#include "thread_mutex.h"

/**
 * @class msg_queue
 *
 * @brief
 */
class msg_queue
{
public:
  /**
   * <high_water_mark> is the max number of the mblock in the 
   * msg_queue in concurrent time, If <mb_count_> is greater than
   * <high_water_mark>, then enqueue_* operation will be failed.
   */
  msg_queue(const size_t high_water_mark);

  ~msg_queue();

  int enqueue_tail(mblock *new_item);

  int enqueue_head(mblock *new_item);

  int dequeue_head(mblock *&first_item);
  int dequeue_head_n(mblock *&items,
                     const int number = -1);

  int dequeue_tail(mblock *&last_item);
  int dequeue_tail_n(mblock *&items,
                     const int number = -1);

  inline size_t size(void)
  {
    guard<mtx_t> g(this->queue_mutex_);
    return this->mb_count_;
  }
  inline bool is_empty()
  {
    guard<mtx_t> g(this->queue_mutex_);
    return this->is_empty_i();
  }
  inline bool is_full()
  {
    guard<mtx_t> g(this->queue_mutex_);
    return this->is_full_i();
  }
protected:
  //
  inline bool is_empty_i()
  { return this->tail_ == NULL; }

  inline bool is_full_i()
  { return this->mb_count_ >= this->high_water_mark_; }

  int wait_not_empty_cond();

  int enqueue_tail_i(mblock *new_item);
  int enqueue_head_i(mblock *new_item);

  int dequeue_head_i(mblock *&first_item);
  int dequeue_head_n_i(mblock *&items, int number);

  int dequeue_tail_i(mblock *&last_item);
  int dequeue_tail_n_i(mblock *&items, int number);
private:
  size_t	  mb_count_;
  size_t    high_water_mark_;

  mblock *head_;
  mblock *tail_;

  typedef thread_mutex mtx_t;
  mtx_t     queue_mutex_;

  condition<thread_mutex> not_empty_cond_;
};

#endif // MSG_QUEUE_H_

