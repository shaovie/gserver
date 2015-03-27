#include "timer_heap.h"
#include "time_value.h"
#include "ev_handler.h"

#include <limits.h>
#include <cassert>
#include <cstring>
#include <ctype.h>
#include <cstdio>

#define HEAP_PARENT(X)   (X == 0 ? 0 :(((X) - 1) / 2))
#define HEAP_LCHILD(X)   ((X) * 2 + 1)

timer_heap::timer_heap(const int max_size, const int pre_alloc_num) :
  max_size_(max_size),
  curr_size_(0),
  timer_heap_size_(0),
  timer_ids_size_(0),
  min_timer_id_(0),
  timer_heap_(NULL),
  free_timer_nodes_(NULL),
  timer_ids_(NULL)
{
  if (max_size < 16) this->max_size_ = 16;
  int real_pre_alloc_num = pre_alloc_num;
  if (real_pre_alloc_num < 16) real_pre_alloc_num = 16;

  this->timer_heap_ = new timer_node* [real_pre_alloc_num];
  this->timer_ids_  = new int[real_pre_alloc_num];

  timer_node *itor = NULL;
  for (int i = 0; i < real_pre_alloc_num; ++i)
  {
    this->timer_heap_[i] = NULL;
    this->timer_ids_[i]  = -1;
    //
    timer_node *node = new timer_node();
    if (this->free_timer_nodes_ == NULL) 
    {
      this->free_timer_nodes_ = node;
      itor = this->free_timer_nodes_;
    }else
    {
      itor->next_ = node;
      itor = itor->next_;
    }
  }

  this->timer_heap_size_ = real_pre_alloc_num;
  this->timer_ids_size_  = real_pre_alloc_num;
  this->min_timer_id_ = 0;
}
int timer_heap::schedule(ev_handler *eh,
                         const time_value &delay_time,	
                         const time_value &interval_time)
{
  if (eh == NULL) return -1;
  if (this->curr_size_ >= this->max_size_) return -1;

  timer_node *node = this->alloc_node();
  if (node == NULL) return -1;

  int timer_id = this->alloc_timerid();
  if (timer_id == -1) 
  {
    this->free_node(node);
    return -1;
  }

  node->timer_id_      = timer_id;
  node->handler_       = eh;
  node->timer_value_   = time_value::gettimeofday() + delay_time;
  node->interval_      = interval_time;

  this->insert(node);

  eh->timer_id(timer_id);
  return timer_id;
}
int timer_heap::cancel(const int timer_id, const int dont_call_handle_close)
{
  if (timer_id < 0 || timer_id >= this->timer_ids_size_)
    return -1;

  int timer_node_slot = this->timer_ids_[timer_id];
  // Check to see if timer_id is still valid.
  if (timer_node_slot < 0)
    return 0;

  if (timer_id != this->timer_heap_[timer_node_slot]->timer_id_)
    return -1;
  else
  {
    timer_node *temp = this->remove(timer_node_slot);
    this->free_node(temp);	
    if (!dont_call_handle_close)
      temp->handler_->handle_close(-1, ev_handler::timer_mask);
  }
  return 0;
}
void timer_heap::expire()
{
  if (this->curr_size_ <= 0) return ;
  time_value current_time = time_value::gettimeofday();
  timer_node dispatched_node;
  while (this->curr_size_ > 0
         && this->dispatch_timer(current_time, dispatched_node))
  {
    if (dispatched_node.handler_->handle_timeout(current_time) < 0)
    {
      if (dispatched_node.interval_ > time_value::zero)
        this->cancel(dispatched_node.timer_id_, 0);
      else
      {
        dispatched_node.handler_->handle_close(-1,
                                               ev_handler::timer_mask);
      }
    }
  }
}
int timer_heap::dispatch_timer(const time_value &current_time,
                               timer_node &dispatched_node)
{
  timer_node *expired = NULL;
  if (this->timer_heap_[0]->timer_value_ <= current_time)
  {
    expired = this->remove(0);
    if (expired == NULL) return 0;

    dispatched_node = *expired;

    if (expired->interval_ > time_value::zero)
    {
      // Make sure that we skip past values that have already 
      // 'expired'.
      do
      {
        expired->timer_value_ += expired->interval_;
      }while (expired->timer_value_ <= current_time);
      // Since this is an interval timer, we need to reschedule it.
      this->insert(expired);
    }else
      this->free_node(expired);

    return 1;
  }
  return 0;
}
int timer_heap::alloc_timerid()
{
  for (int i = this->min_timer_id_; i < this->timer_ids_size_; ++i)
  {
    if (this->timer_ids_[i] == -1)
    {
      this->min_timer_id_ = i;
      return i;
    }
  } 
  return -1;
}
void timer_heap::release_timerid(const int id)
{
  if (id >= 0 && id < this->timer_ids_size_)
  {
    this->timer_ids_[id] = -1;
    if (id < this->min_timer_id_)
      this->min_timer_id_ = id;
  }
  return ;
}
timer_node *timer_heap::alloc_node()
{
  timer_node *node = NULL;
  if (this->free_timer_nodes_ == NULL)
    node = new timer_node();
  else
  {
    node = this->free_timer_nodes_;
    this->free_timer_nodes_ = node->next_;
    node->next_ = NULL;
  }
  return node;
}
void timer_heap::free_node(timer_node *node)
{
  if (node == NULL) return ;
  this->release_timerid(node->timer_id_);

  node->next_ = this->free_timer_nodes_;
  this->free_timer_nodes_ = node;
}
void timer_heap::insert(timer_node *new_node)
{
  if (this->curr_size_ + 2 >= this->timer_heap_size_)
    this->grow_heap();

  this->reheap_up(new_node,
                  this->curr_size_, // slot
                  HEAP_PARENT(this->curr_size_));
  ++this->curr_size_;
}
timer_node *timer_heap::remove(const int slot)
{
  timer_node *removed_node = this->timer_heap_[slot];

  --this->curr_size_;

  if (slot < this->curr_size_)
  {
    timer_node *last_node = this->timer_heap_[this->curr_size_];
    this->copy(slot, last_node);

    // If the <moved_node->time_value_> is great than or equal its
    // parent it needs be moved down the heap.
    int parent = HEAP_PARENT(slot);
    if (last_node->timer_value_
        >= this->timer_heap_[parent]->timer_value_)
      this->reheap_down(last_node,
                        slot,
                        HEAP_LCHILD(slot));
    else
      this->reheap_up(last_node, slot, parent);
  }

  return removed_node;
}
int timer_heap::grow_heap(void)
{
  int new_size = this->curr_size_ * 2 < this->max_size_ 
    ? this->curr_size_ * 2 : this->max_size_;

  timer_node **new_heap = NULL;
  new_heap = new timer_node*[new_size];
  std::memcpy(new_heap, this->timer_heap_, this->curr_size_*sizeof(timer_node *));
  delete [] this->timer_heap_;
  this->timer_heap_ = new_heap;
  this->timer_heap_size_ = new_size;

  // grow the array of timer ids.
  int *new_timer_ids = NULL;
  new_timer_ids = new int[new_size];
  std::memcpy(new_timer_ids, this->timer_ids_, this->curr_size_*sizeof(int));
  delete [] this->timer_ids_;
  this->timer_ids_ = new_timer_ids;
  this->timer_ids_size_ = new_size;

  for (int i = this->curr_size_; i < new_size; ++i)
  {
    this->timer_heap_[i] = NULL;
    this->timer_ids_[i]  = -1;
  }

  this->min_timer_id_ = this->curr_size_;
  return 0;
}
void timer_heap::reheap_up(timer_node *moved_node, int slot, int parent)
{
  // Restore the heap property after an insertion.
  while (slot > 0
         && moved_node->timer_value_ < this->timer_heap_[parent]->timer_value_)
  {
    // If the parent node is greater than the <moved_node> we need
    // to copy it down.
    this->copy(slot, this->timer_heap_[parent]);
    slot = parent;
    parent = HEAP_PARENT(slot);
  }
  // Insert the new node into its proper resting place in the heap and
  // update the corresponding slot in the parallel <timer_ids> array.
  this->copy(slot, moved_node);
}
void timer_heap::reheap_down(timer_node *moved_node, int slot, int lchild)
{
  // Restore the heap property after a deletion.
  while (lchild < this->curr_size_)
  {
    // Choose the smaller of the two lchildren.
    if (lchild + 1 < this->curr_size_
        && this->timer_heap_[lchild + 1]->timer_value_
        < this->timer_heap_[lchild]->timer_value_)
      ++lchild;
    // Perform a <copy> if the lchild has a larger timeout value than
    // the <moved_node>.
    if (this->timer_heap_[lchild]->timer_value_ < moved_node->timer_value_)
    {
      this->copy(slot, this->timer_heap_[lchild]);
      slot = lchild;
      lchild = HEAP_LCHILD(lchild);
    }else
      // We've found our location in the heap.
      break;
  }
  this->copy(slot, moved_node);
}

