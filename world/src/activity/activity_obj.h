// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-24 10:46
 */
//========================================================================

#ifndef ACTIVITY_OBJ_H_
#define ACTIVITY_OBJ_H_

// Lib header
#include "mblock.h"

#include "ilist.h"

// Forward declarations
class player_obj;

/**
 * @class activity_obj
 * 
 * @brief
 */
class activity_obj
{
public:
  enum
  {
    ST_OPENED   = 111,
    ST_CLOSED   = 112,
  };
  activity_obj(const int act_id);
  virtual ~activity_obj();

  virtual int init()  { return 0; }
  virtual int open()  { return 0; }
  virtual int close() { return 0; }

  virtual void run(const int );
  virtual int do_ai(const int ) { return 0; }
  virtual bool is_opened() { return this->status_ == ST_OPENED; } 
  virtual int can_enter(player_obj *) { return 0; }

  virtual int enter(player_obj *, const int) { return 0; }
  virtual int exit(player_obj *) { return 0; }
  virtual int left_time() { return this->left_time_; }
  virtual int begin_time() { return this->begin_time_; }
  virtual void do_something(mblock *, int *, mblock *) { return ; }

  int act_id() const { return this->act_id_; }
protected:
  virtual void on_activity_opened();
  virtual void on_activity_closed();
  virtual bool do_check_if_opened(const int now);
protected:
  int status_;
  int act_id_;
  int begin_time_;
  int left_time_;
};

#endif // ACTIVITY_OBJ_H_

