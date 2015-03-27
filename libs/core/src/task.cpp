#include "task.h"
#include "msg_queue.h"

// Lib header
#include <pthread.h>

task::~task()
{ delete this->msg_queue_; }
int task::activate(const int n_threads/* = 1*/, const int mq_high_water_mark/* = 1024*/)
{
  if (this->msg_queue_ == NULL)
    this->msg_queue_ = new msg_queue(mq_high_water_mark);

  for (int i = 0; i < n_threads; ++i)
  {
    pthread_t thr_id = 0;
    if (::pthread_create(&thr_id, NULL, &task::svc_run, (void*)this) != 0)
      return -1;
    ::pthread_detach(thr_id);
  }
  return 0;
}
size_t task::mq_size()
{ return this->msg_queue_->size(); }
int task::putq(mblock *mb)
{ return this->msg_queue_->enqueue_tail(mb); }
int task::putq_h(mblock *mb)
{ return this->msg_queue_->enqueue_head(mb); }
int task::getq(mblock *&mb)
{ return this->msg_queue_->dequeue_head(mb); }
int task::getq_n(mblock *&mb, const int number/* = -1*/)
{ return this->msg_queue_->dequeue_head_n(mb, number); }
void *task::svc_run(void *arg)
{
  task* t = (task *)arg;
  t->svc();
  return NULL;
}
