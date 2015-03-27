// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-08 23:10
 */
//========================================================================

#ifndef SCP_OBJ_H_
#define SCP_OBJ_H_

#include "def.h"
#include "ilist.h"

// Lib header
#include "mblock.h"

// Forward declarations
class player_obj;
class sub_scp_obj;

/**
 * @class scp_obj
 *
 * @brief
 */
class scp_obj
{
public:
  enum
  {
    S_SCP_INIT            = 1L << 0,
    S_SCP_RUNNING         = 1L << 1,
    S_SCP_TO_CLOSE        = 1L << 2
  };
  enum
  {
    CHAR_INITIATIVE_EXIT  = 13, // 玩家主动离开退出
    CHAR_PASSIVE_EXIT     = 14, // 玩家被动离开副本
  };
  scp_obj(const int scene_cid,
          const int char_id,
          const int team_id,
          const int guild_id);
  virtual ~scp_obj();

  virtual int init();
  virtual int open();
  virtual int close();
  virtual int run(const int );

  virtual int load_config();
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);

  virtual int can_enter_scp(player_obj *);
  virtual int enter_scp(const int char_id, const int scene_id);
  virtual int exit_scp(const int char_id, const int scene_id, const int reason);
  virtual void on_exit_scp(const int char_id, const int reason);
  virtual bool do_check_nodoby();

  virtual int owner_id() { return this->char_id_; }
  virtual void do_something(mblock *, int *, mblock *) { return ; }

  int scp_cid() { return this->scene_cid_; }
  int first_scene_id();
  ilist<int/*char id*/> *enter_char_list() { return &this->enter_char_list_; }
  void put_aev_to_all_char(const int aev, mblock *ev_mb);
protected:
  void insert_new_sub_scp(sub_scp_obj *sub_scp);
protected:
  int scp_status_;
  int char_id_;                 // 副本归属
  int team_id_;
  int guild_id_;
  int scene_cid_;               // 副本CID(就是第一张图的CID)

  int open_time_;               // 副本开启时间
  ilist<int/*char id*/> enter_char_list_;
  ilist<pair_t<int> >   exit_char_list_; // first:char id ; second:exit reason
  ilist<sub_scp_obj *>  sub_scp_list_;
};

#endif // SCP_OBJ_H_

