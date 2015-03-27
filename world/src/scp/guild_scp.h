// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-08-05 19:07
 */
//========================================================================

#ifndef GUILD_SCP_H_
#define GUILD_SCP_H_

// Lib header

#include "scp_obj.h"
#include "sub_scp_obj.h"
#include <tr1/unordered_map>

// Forward declarations
class player_obj;

/**
 * @class guild_scp
 *
 * @brief
 */
class guild_scp : public scp_obj
{
public:
  guild_scp(const int scene_cid, const int guild_id);
  virtual ~guild_scp() { };
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
public:
  virtual int owner_id() { return this->guild_id_; }
  virtual bool do_check_nodoby() { return false; };
};
/**
 * @class guild_zhu_di_sub_scp
 *
 * @brief
 */
class guild_sub_scp : public sub_scp_obj
{
  typedef sub_scp_obj super;
public:
  typedef std::tr1::unordered_map<int/*char_id*/, int/*contrib*/> contrib_map_t;
  typedef std::tr1::unordered_map<int/*char_id*/, int/*contrib*/ >::iterator contrib_map_itor;
public:
  guild_sub_scp(const int scene_id, const int scene_cid);
  virtual ~guild_sub_scp() { };
public:
  virtual void do_something(mblock *, int *, mblock *);
  virtual int run(const int now);
  virtual void enter_sub_scp(const int /*char_id*/);
private:
  void do_init_scp(mblock *mb);
  void on_scp_reach_finish(mblock *mb);
  void if_scp_can_enter(int *res);
  void on_scp_mst_exit();
  void do_spawn_monster(const char bo_shu, const char batch);
  void do_award_yi_bo_over(const char bo_shu);
  void do_over_scp(const char result);
  void do_bro_scp_info();
  void do_notify_scp_info(const int char_id);
  int  get_player_contirb(const int char_id);
  void switch_to_status(const char status);
private:
  char difficulty_;   // 难度
  char bo_shu_;       // 波数
  char batch_;        // 每波第几批
  char hp_;           // 血量
  char scp_lvl_;      // 等级
  char scp_status_;   // 状态
  int  add_resource_; // 获取资源
  int  switch_status_time_;  //
  int  mst_amt_;      //
  contrib_map_t contrib_map_; // 获取贡献
};

#endif // GUILD_SCP_H_
