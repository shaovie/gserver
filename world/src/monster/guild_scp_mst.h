// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-08-05 15:01
 */
//========================================================================

#ifndef GUILD_SCP_MST_H_
#define GUILD_SCP_MST_H_

// Lib header

#include "fighting_monster_obj.h"

// Forward declarations

/**
 * @class guild_scp_had_hate_mst
 *
 * @brief
 */
class guild_scp_had_hate_mst : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  guild_scp_had_hate_mst();
  virtual ~guild_scp_had_hate_mst() { }
protected:
  virtual void do_patrol(const time_value &);
  virtual void to_back(const time_value &);
  virtual int  do_exit_scene();
  virtual int  do_action_in_scene(const time_value &);
protected:
  bool if_reach_target();
  void on_reach_target(const time_value &);
private:
  short target_x_;
  short target_y_;
  short range_;
  time_value reach_time_;
};
/**
 * @class guild_scp_no_hate_mst
 *
 * @brief
 */
class guild_scp_no_hate_mst : public guild_scp_had_hate_mst
{
  typedef guild_scp_had_hate_mst super;
public:
  guild_scp_no_hate_mst() { }
  virtual ~guild_scp_no_hate_mst() { }
protected:
  virtual void do_fight_back(const int ) { }
};

#endif // GUILD_SCP_MST_H_
