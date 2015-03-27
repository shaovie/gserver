// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-04 11:39
 */
//========================================================================

#ifndef LUE_DUO_SCP_H_
#define LUE_DUO_SCP_H_

#include "scp_obj.h"
#include "sub_scp_obj.h"

// Forward declarations

/**
 * @class lue_duo_scp
 * 
 * @brief
 */
class lue_duo_scp : public scp_obj
{
public:
  lue_duo_scp(const int scene_cid, const int char_id);
  virtual ~lue_duo_scp();

  static void on_char_dead(player_obj *player);
public:
  virtual int  load_config() { return 0; }
  virtual int  owner_id() { return this->char_id_; }
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
public:
};
/**
 * @class lue_duo_sub_scp
 *
 * @brief
 */
class lue_duo_sub_scp : public sub_scp_obj
{
  typedef sub_scp_obj super;
public:
  lue_duo_sub_scp(const int scene_id, const int scene_cid);
  virtual ~lue_duo_sub_scp();
public:
  virtual int open(scp_obj *);
  virtual void do_something(mblock *, int *, mblock *);
  virtual int run(const int now);
  virtual void exit_sub_scp(const int char_id);
private:
  void lue_duo_end(const int);
private:
  bool end_;
  bool has_fanpai_;
  int monster_id_;
  int last_time_;
};

#endif // LUE_DUO_SCP_H_

