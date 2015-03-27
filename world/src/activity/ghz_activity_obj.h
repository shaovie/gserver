// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-04 15:16
 */
//========================================================================

#ifndef GHZ_ACTIVITY_OBJ_H_
#define GHZ_ACTIVITY_OBJ_H_

#include "def.h"
#include "ilist.h"
#include "activity_obj.h"

// Forward declarations

/**
 * @class ghz_activity_obj
 * 
 * @brief
 */
class ghz_activity_obj : public activity_obj
{
  enum
  {
    IN_PREPARING  = 11,
    IN_FIGHTING   = 12,
  };
  typedef activity_obj super;
public:
  ghz_activity_obj();
  virtual ~ghz_activity_obj() { }

  virtual int init();
  virtual int open();
  virtual int close();
  virtual int do_ai(const int );
  virtual void do_something(mblock *, int *, mblock *);
  virtual int  can_enter(player_obj *);
  virtual bool is_opened() { return this->status_ == ST_OPENED && !this->fight_over_; } 
  virtual int  left_time();

  static int  parse_shou_wei_idx_cfg(const char *s);
  static int  get_shou_wei_idx(const int cid);
  static char get_shou_wei_state(const int cid);
  static void set_shou_wei_state(const int cid, const char );
  static void set_shou_wei_pos(const int cid,
                               const char dir,
                               const short x,
                               const short y);
  static int get_shou_wei_pos(const int cid, char &dir, coord_t &pos);
private:
  void on_ghz_over();
  void spawn_monster_not_in_activity();
private:
  bool fight_over_;
  int ghz_state_;
  int wang_zuo_id_;
  int begin_time_;
  ilist<int/*mst id*/> shou_wei_list_;
};

#endif // GHZ_ACTIVITY_OBJ_H_

