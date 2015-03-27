// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2013-12-13 17:30
 */
//========================================================================

#ifndef TEAM_MGR_H_
#define TEAM_MGR_H_

// Lib header
#include <string.h>
#include "singleton.h"

#define MAX_TEAM_MEMBER 4

// Forward declarations
class team_mgr_impl;

/**
 * @class team_info
 * 
 * @brief
 */
class team_info
{
public:
  enum
  {
    ADD_MEMBER    = 1,
    DEL_MEMBER    = 2,
    LEAVE_TEAM    = 3,
    CHANGE_LEADER = 4,
    MEMBER_OFFLINE= 5,
    MEMBER_ONLINE = 6
  };

  team_info(const int team_id, const int char_id)
    : team_id_(team_id), leader_id_(char_id)
  {
    ::memset(this->mem_id_, 0, sizeof(this->mem_id_));
    this->mem_id_[0] = char_id;
  }
  team_info(): team_id_(0), leader_id_(0)
  { ::memset(this->mem_id_, 0, sizeof(this->mem_id_)); }

  int leader() const { return this->leader_id_; }
  void set_leader(int leader) { this->leader_id_ = leader; }

  int team_id() const { return this->team_id_; }

  void add_member(const int mem_id);
  void del_member(const int mem_id);

  bool have_player(const int mem_id);
  int member_count();
public:
  int team_id_;
  int leader_id_;
  int mem_id_[MAX_TEAM_MEMBER];
};

/**
 * @class team_mgr
 * 
 * @brief
 */
class team_mgr : public singleton<team_mgr>
{
  friend class singleton<team_mgr>;
public:
  int assign_id();

  void insert(const int, team_info*);
  void remove(const int);
  team_info* find(const int);
  team_info* find_by_char(const int);

  bool is_leader(const int team_id, const int char_id);
private:
  team_mgr();

  team_mgr_impl *impl_;
};

#endif // TEAM_MGR_H_
