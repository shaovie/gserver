#include "msg_queue.h"

msg_queue::msg_queue(const size_t high_water_mark) :
  mb_count_(0),
  high_water_mark_(high_water_mark),
  head_(NULL),
  tail_(NULL),
  queue_mutex_(),
  not_empty_cond_(queue_mutex_)
{ }
msg_queue::~msg_queue()
{
  guard<mtx_t> g(this->queue_mutex_);
  if (this->head_ != NULL)
  {
    mblock *mb = NULL;
    for (this->tail_ = NULL; this->head_ != NULL; )
    {
      mb = this->head_;
      this->head_ = this->head_->next();
      mb->clean();  // 
      delete mb;
    }
  }
}
int msg_queue::enqueue_tail(mblock *new_item)
{
  if (new_item == NULL) return -1;

  guard<mtx_t> g(this->queue_mutex_);
  if (this->is_full_i())
    return -1;

  this->enqueue_tail_i(new_item);
  this->not_empty_cond_.signal();
  return 0;
}
int msg_queue::enqueue_tail_i(mblock *new_item)
{
  mblock *seq_tail = new_item;

  int num = 1;
  while (seq_tail->next() != NULL)
  {
    seq_tail->next()->prev(seq_tail);
    seq_tail = seq_tail->next();
    ++num;
  }   

  if (this->tail_ == NULL)
  {
    this->head_ = new_item;
    this->tail_ = seq_tail;
    new_item->prev(NULL);
  }else
  {
    new_item->prev(this->tail_);
    this->tail_->next(new_item);
    this->tail_ = seq_tail;
  }
  this->mb_count_ += num;
  return 0;
} 
int msg_queue::enqueue_head(mblock *new_item)
{
  if (new_item == NULL) return -1;

  guard<mtx_t> g(this->queue_mutex_);
  if (this->is_full_i())
    return -1;

  this->enqueue_head_i(new_item);
  this->not_empty_cond_.signal();
  return 0;
}
int msg_queue::enqueue_head_i(mblock *new_item)
{
  mblock *seq_tail = new_item;

  int num = 1;
  while (seq_tail->next() != NULL)
  {
    seq_tail->next()->prev(seq_tail);
    seq_tail = seq_tail->next();
    ++num;
  }   

  new_item->prev(NULL);
  seq_tail->next(this->head_);
  if (this->head_ != NULL)
    this->head_->prev(seq_tail);  
  else
    this->tail_ = seq_tail;
  this->head_ = new_item;

  this->mb_count_ += num;
  return 0;
} 
int msg_queue::wait_not_empty_cond()
{
  while (this->is_empty_i())
  {
    if (this->not_empty_cond_.wait() != 0)
      return -1;
  }
  return 0;
}
int msg_queue::dequeue_head(mblock *&first_item)
{
  guard<mtx_t> g(this->queue_mutex_);
  if (this->wait_not_empty_cond() != 0)
    return -1;
  return this->dequeue_head_i(first_item);
}
int msg_queue::dequeue_head_i(mblock *&first_item)
{    
  if (this->head_ == NULL) return -1;  // check empty !!

  first_item  = this->head_;
  this->head_ = this->head_->next();
  if (this->head_ == NULL)
    this->tail_ = NULL;
  else  
    this->head_->prev(NULL);
  --this->mb_count_;

  first_item->prev(NULL);
  first_item->next(NULL);
  return 0;
}
int msg_queue::dequeue_head_n(mblock *&items, 
                              const int number/* = -1*/)
{
  if (number == 0) return 0;
  guard<mtx_t> g(this->queue_mutex_);
  if (this->wait_not_empty_cond() != 0)
    return -1;
  return this->dequeue_head_n_i(items, number);
}
int msg_queue::dequeue_head_n_i(mblock *&items, int number)
{
  if (this->head_ == NULL) return 0;
  int count = 0;
  if (number == -1)  // dequeue all
  {
    items = this->head_;
    this->head_ = this->tail_ = NULL;
    count = this->mb_count_;
    this->mb_count_ = 0;
  }else
  {
    items = this->head_;
    while (number-- > 0 && this->head_ != NULL)
    {
      this->head_ = this->head_->next();
      ++count;
    }
    if (this->head_ == NULL)
    {
      this->tail_ = NULL;
      this->mb_count_ = 0;
    }else
    {
      this->head_->prev()->next(NULL); // the items's tail 
      this->head_->prev(NULL);  // the prev pointer of the first 
      // message block must point to 0
      this->mb_count_ -= count;
    }
  }
  return count;
}
int msg_queue::dequeue_tail(mblock *&last_item)
{
  guard<mtx_t> g(this->queue_mutex_);
  if (this->wait_not_empty_cond() != 0)
    return -1;
  return this->dequeue_tail_i(last_item);
}
int msg_queue::dequeue_tail_i(mblock *&last_item)
{
  if (this->tail_ == NULL) return -1;  // check empty !!

  last_item = this->tail_;
  if (this->tail_->prev() == NULL) // only one mb
  {
    this->head_ = this->tail_ = NULL;
  }else
  {
    this->tail_->prev()->next(NULL);  // set eof
    this->tail_ = this->tail_->prev();  // 
  }

  --this->mb_count_;

  last_item->prev(NULL);
  last_item->next(NULL);
  return 0;
}
int msg_queue::dequeue_tail_n(mblock *&items, 
                              const int number/* = -1*/)
{
  if (number == 0) return 0;
  guard<mtx_t> g(this->queue_mutex_);
  if (this->wait_not_empty_cond() != 0)
    return -1;
  return this->dequeue_tail_n_i(items, number);
}
int msg_queue::dequeue_tail_n_i(mblock *&items, int number)
{
  if (this->tail_ == NULL) return 0;  // check empty !!
  int count = 0;
  if (number == -1)  // dequeue all
  {
    items = this->head_;
    this->head_ = this->tail_ = NULL;
    count = this->mb_count_;
    this->mb_count_ = 0;
  }else
  {
    while (number-- > 0 && this->tail_ != NULL)
    {
      this->tail_ = this->tail_->prev();
      ++count;
    }
    if (this->tail_ == NULL) // not enough
    {
      items = this->head_;
      this->head_ = NULL;
      count = this->mb_count_;
      this->mb_count_ = 0;
    }else
    {
      items = this->tail_->next();
      items->prev(NULL);
      this->tail_->next(NULL); // the items's tail 
      this->mb_count_ -= count;
    }
  }
  return count;
}
