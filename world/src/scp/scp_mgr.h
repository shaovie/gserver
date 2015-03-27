// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-08 22:34
 */
//========================================================================

#ifndef SCP_MGR_H_
#define SCP_MGR_H_

// Lib header
#include "singleton.h"

// Forward declarations
class scp_obj;
class mblock;
class sub_scp_obj;
class scp_mgr_impl;

/**
 * @class scp_mgr
 *
 * @brief
 */
class scp_mgr : public singleton<scp_mgr>
{
  friend class singleton<scp_mgr>;
public:
  void run(const int now);

  // 分配副本scene_id
  int assign_scp_id();
  void release_scp_id(int scp_id);
  int size();

  void insert_scp(scp_obj *);
  void remove_scp(scp_obj *);

  sub_scp_obj *get_sub_scp_obj(const int scene_id);
  int insert_sub_scp(const int scene_id, sub_scp_obj *);
  void remove_sub_scp(const int scene_id);

  void bind_scp_char_owner(const int char_id, scp_obj *so);
  scp_obj *get_scp_by_char_owner(const int char_id);
  void unbind_scp_char_owner(const int char_id);
  void bind_scp_team_owner(const int team_id, scp_obj *so);
  scp_obj *get_scp_by_team_owner(const int team_id);
  void unbind_scp_team_owner(const int team_id);
  void bind_scp_guild_owner(const int guild_id, scp_obj *so);
  scp_obj *get_scp_by_guild_owner(const int guild_id);
  void unbind_scp_guild_owner(const int guild_id);

  int exit_scp(const int scene_id, const int char_id, const int r);

  void do_something(const int scene_id, mblock *, int *ret, mblock *result);
private:
  scp_mgr();

  scp_mgr_impl *impl_;
};

#endif // SCP_MGR_H_

