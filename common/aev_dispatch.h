// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-12 16:30
 */
//========================================================================

#ifndef AEV_DISPATCH_H_
#define AEV_DISPATCH_H_

#include "aev.h"
#include "ilist.h"

// Forward declarations
class mblock;

/**
 * @class ev_node
 *
 * @brief
 */
class aev_node
{
public:
  aev_node() : id_(0), mb_(NULL) { }
  aev_node(const int id, mblock *mb) : id_(id), mb_(mb) { }

  int     id_;
  mblock *mb_;
};
/**
 * @class aev_dispatch
 *
 * @brief asynch event dispatch
 */
class aev_dispatch
{
public:
  void post_aev(const int aev_id, mblock *mb);

protected:
  aev_dispatch();

  virtual ~aev_dispatch();

  void handle_aev_queue();

  virtual void dispatch_aev(const int ev_id, mblock *) = 0;
private:
  ilist<aev_node> aev_list_;
};

#endif // AEV_DISPATCH_H_

