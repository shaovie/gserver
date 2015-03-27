// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-11 20:39
 */
//========================================================================

#ifndef JING_JI_SCP_H_
#define JING_JI_SCP_H_

#include "scp_obj.h"
#include "sub_scp_obj.h"

// Forward declarations

/**
 * @class jing_ji_scp
 * 
 * @brief
 */
class jing_ji_scp : public scp_obj
{
public:
  jing_ji_scp(const int scene_cid, const int char_id);
  virtual ~jing_ji_scp();

public:
  virtual int  load_config() { return 0; }
  virtual int  owner_id() { return this->char_id_; }
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
public:
};
/**
 * @class jing_ji_sub_scp
 *
 * @brief
 */
class jing_ji_sub_scp : public sub_scp_obj
{
  typedef sub_scp_obj super;
public:
  jing_ji_sub_scp(const int scene_id, const int scene_cid);
  virtual ~jing_ji_sub_scp();
public:
  virtual int open(scp_obj *);
  virtual void do_something(mblock *, int *, mblock *);
  virtual int run(const int now);
private:
  void jing_ji_end(const int);
private:
  bool end_;
  int monster_id_;
  int last_time_;
};

#endif // JING_JI_SCP_H_

