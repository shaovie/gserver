// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-04-09 16:33
 */
//========================================================================

#ifndef GUILD_ZHU_DI_SCP_H_
#define GUILD_ZHU_DI_SCP_H_

#include "scp_obj.h"
#include "sub_scp_obj.h"

// Forward declarations
class player_obj;

/**
 * @class guild_zhu_di_scp
 *
 * @brief
 */
class guild_zhu_di_scp : public scp_obj
{
public:
  guild_zhu_di_scp(const int scene_cid, const int guild_id);
  virtual ~guild_zhu_di_scp();
public:
  virtual int  load_config() { return 0; }
  virtual int  owner_id() { return this->guild_id_; }
  virtual bool do_check_nodoby() { return false; }
  virtual void do_something(mblock *, int *, mblock *);
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
public:
  static void on_new_guild(const int guild_id);
  static void on_remove_guild(const int zhudi_scp_id);
  static void on_exit_guild(player_obj *player, const int guild_id);
  static int  do_summon_tld_boss(player_obj *player, const int boss_cid);
  static int  do_enter_zhu_di(player_obj *player);
};
/**
 * @class guild_zhu_di_sub_scp
 *
 * @brief
 */
class guild_zhu_di_sub_scp : public sub_scp_obj
{
  typedef sub_scp_obj super;
public:
  guild_zhu_di_sub_scp(const int scene_id, const int scene_cid);
  virtual ~guild_zhu_di_sub_scp();
public:
  virtual void do_something(mblock *, int *, mblock *);
  virtual int run(const int now);
public:
  void on_remove_guild(mblock *mb);
  void do_summon_tld_boss(mblock *mb);
private:
  int last_create_tld_boss_time_;
  int tld_boss_id_;
};
#endif // GUILD_ZHU_DI_SCP_H_
