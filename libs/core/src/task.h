// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-11 16:26
 */
//========================================================================

#ifndef TASK_H_
#define TASK_H_

#include <cstddef>

// Forward declarations
class mblock;
class msg_queue;
class time_value;

/**
 * @class task
 * 
 * @brief
 */
class task
{
public:
  task() : msg_queue_(NULL) { }

  virtual ~task();

  int activate(const int n_threads = 1, const int mq_high_water_mark = 1024);
protected:
  virtual int svc() = 0;

public:
  size_t mq_size();

  int putq(mblock *mb);
  int putq_h(mblock *mb);

  int getq(mblock *&mb);
  int getq_n(mblock *&mb, const int number = -1);

private:
  static void *svc_run(void *arg);

protected:
  msg_queue *msg_queue_;
};

#endif // TASK_H_

