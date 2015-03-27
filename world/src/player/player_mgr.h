// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-06 10:51
 */
//========================================================================

#ifndef PLAYER_MGR_H_
#define PLAYER_MGR_H_

#include "singleton.h"
#include "ilist.h"

// Forward declarations
class mblock;
class reactor;
class player_obj;
class player_mgr_impl;

/**
 * @class player_mgr
 *
 * @brief
 */
class player_mgr : public singleton<player_mgr>
{
  friend class singleton<player_mgr>;
public:
  int init(reactor *);
  void attach_timer(player_obj *p);
  void deattach_timer(player_obj *p);

  //= key is account
  void insert(const char *, player_obj *);
  player_obj *find(const char *);
  void remove(const char *);

  //= key is char_id
  void insert(const int, player_obj *);
  player_obj *find(const int );
  void remove(const int);
  int size();

  void post_aev_to_world(const int aev_id, mblock *mb);
  void post_aev_to_view_area(const int scene_id, const short x, const short y, const int aev_id, mblock *mb);
  void post_aev_to_scene(const int scene_id, const int aev_id, mblock *mb);
  void post_aev_to_guild(const int guild_id, const int aev_id, mblock *mb);

  void broadcast_to_world(const int msg_id, mblock *mb);
  void broadcast_to_scene(const int scene_id, const int msg_id, mblock *mb);
  void get_ol_char_list(ilist<int> &list);
private:
  player_mgr();
  player_mgr(const player_mgr &);
  player_mgr& operator= (const player_mgr &);

  player_mgr_impl *impl_;
};

#endif // PLAYER_MGR_H_

