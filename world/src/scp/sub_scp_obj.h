// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-08 23:10
 */
//========================================================================

#ifndef SUB_SCP_OBJ_H_
#define SUB_SCP_OBJ_H_

// Lib header
#include "mblock.h"

// Forward declarations
class scp_obj;

/**
 * @class sub_scp_obj
 * 
 * @brief
 */
class sub_scp_obj
{
public:
  sub_scp_obj(const int scene_id, const int scene_cid);
  virtual ~sub_scp_obj();

  virtual int open(scp_obj *);
  virtual int close();

  //= method
  virtual int run(const int );
  virtual void do_clear();

  virtual void enter_sub_scp(const int /*char_id*/) { }
  virtual void exit_sub_scp(const int /*char_id*/) { }

  virtual void do_something(mblock *, int *, mblock *) { return ; }

  int scene_id()  const  { return this->scene_id_;  }
  int scene_cid() const  { return this->scene_cid_; }
  scp_obj *master_scp()  { return this->master_scp_;}

protected:
  int  scene_id_;                // 副本ID
  int  scene_cid_;               // 副本CID == scene_cid
  int  open_time_;               // 副本开启时间

  scp_obj *master_scp_;
};

#endif // SUB_SCP_OBJ_H_

