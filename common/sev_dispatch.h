// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-12 16:44
 */
//========================================================================

#ifndef SEV_DISPATCH_H_
#define SEV_DISPATCH_H_

#include "sev.h"
#include "ilist.h"

#include "singleton.h"

// Forward declarations
class mblock;

/**
 * @class sev_dispatch
 * 
 * @brief 同步事件分派器，通过attack方法挂载相关的ev_id，当ev_id触发时会
 *        循环调用所有挂载的事件处理方法
 */
class sev_dispatch
{
  friend class sev_notify;
protected:
  virtual void dispatch_sev(const int ev_id, mblock *) = 0;

  virtual ~sev_dispatch() { }
};
/**
 * @class sev_notify
 * 
 * @brief or boost::bind + boost::function
 */
class sev_notify : public singleton<sev_notify>
{
  friend class singleton<sev_notify>;
public:
  void notify(const int ev_id, mblock *mb);

  void attach(const int ev_id, sev_dispatch *);

  // don't deattach when in notify method!
  void deattach(const int ev_id, sev_dispatch *);
  void deattach(sev_dispatch *);
private:
  sev_notify();
  sev_notify(const sev_notify &);
  sev_notify& operator= (const sev_notify&);

  int min_ev_id_;
  ilist<sev_dispatch *> *sev_dispatch_map_;
};

#endif // SEV_DISPATCH_H_

