// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-01 16:18
 */
//========================================================================

#ifndef XSZC_SCP_H_
#define XSZC_SCP_H_

#include "scp_obj.h"
#include "sub_scp_obj.h"
#include "ilist.h"

/**
 * @class xszc_scp_obj
 *
 * @brief
 */
class xszc_scp_obj : public scp_obj
{
public:
  xszc_scp_obj(const int scene_cid);
  ~xszc_scp_obj() { }

  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
  virtual bool do_check_nodoby() { return false; }
};

enum
{
  S_XSZC_READY    = 1L << 0,
  S_XSZC_RUNNING  = 1L << 1,
  S_XSZC_END      = 1L << 2
};

/**
 * @class xszc_player_info
 *
 * @brief
 */
class xszc_player_info
{
public:
  xszc_player_info() :
    char_id_(0),
    kill_count_(0),
    honor_(0)
  { }

  int char_id_;
  int kill_count_;
  int honor_;
};
/**
 * @class xszc_sub_scp_obj
 *
 * @brief
 */
class xszc_sub_scp_obj : public sub_scp_obj
{
  typedef sub_scp_obj super;
public:
  xszc_sub_scp_obj(const int scene_id, const int scene_cid);
  ~xszc_sub_scp_obj();

  virtual int open(scp_obj *);
  virtual int run(const int now);

  virtual void do_something(mblock *, int *, mblock *);

  virtual void enter_sub_scp(const int char_id);
  virtual void exit_sub_scp(const int char_id);
private:
  void do_spawn_monster();
  void xszc_end(const int lose_group, const int char_id);
  void add_char_honor(const int char_id, const short honor);
  void add_kill_count(const int char_id);
  void end_award(const int lose_group);
  void fight_battle_notice(const int type, const int killer_id, const int killed_id = 0);

private:
  int status_;
  int last_time_;
  int last_spawn_mst_time_;
  int jinwei_kill_count_;
  int tianzai_kill_count_;
  ilist<int> dead_barrack_list_;
  ilist<xszc_player_info *> xszc_player_info_list_;
};

#endif  // XSZC_SCP_H_
