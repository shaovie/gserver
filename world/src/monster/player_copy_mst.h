// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-06 11:43
 */
//========================================================================

#ifndef PLAYER_COPY_MST_H_
#define PLAYER_COPY_MST_H_

#include "fighting_monster_obj.h"

// Forward declarations

/**
 * @class player_copy_mst
 * 
 * @brief
 */
class player_copy_mst : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  player_copy_mst();
  virtual ~player_copy_mst();

  virtual int unit_type() const { return scene_unit::PLAYER; }
  virtual int master_id() const { return this->char_id_;     }
  virtual void master_id(const int );
  virtual void do_repulse(char_obj *, const time_value &, const int, const int) { }
  virtual void on_attack_somebody(const int /*target_id*/,
                                  const int /*obj_type*/,
                                  const time_value &/*now*/,
                                  const int /*skill_cid*/,
                                  const int /*real_hurt*/);
  virtual int  can_be_moved() { return ERR_CAN_NOT_BE_MOVED; }
  virtual void do_back(const time_value &);
  virtual skill_info *do_select_skill(const time_value &now,
                                      char_obj *target,
                                      int &result,
                                      int &skill_hurt_delay,
                                      const skill_detail *&sd);
  virtual void on_use_skill_ok(skill_info *si,
                               const skill_detail *,
                               const int ,
                               const time_value &);
  virtual void on_attr_update(const int, const int);

  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_unit_list,
                                  ilist<pair_t<int> > *new_snap_unit_list);
  static int get_player_copy_id(const int);
protected:
  virtual int  do_assign_id();
  virtual void do_release_id();
  virtual int  do_think(const time_value &);
  virtual int  do_rebirth_init(const time_value &now);
  virtual void do_patrol(const time_value &now);
  virtual void do_dead(const int /*killer_id*/);
protected:
  virtual void dispatch_aev(const int aev_id, mblock *);
private:
  void do_build_snap_base_info(out_stream &);
  void do_build_snap_pos_info(out_stream &os);
  void do_build_snap_status_info(out_stream &os);
  void do_build_snap_equip_info(out_stream &os);
protected:
  //= aev
  void aev_push_skill(mblock *mb);
  void aev_zhuzai_fen_shen(mblock *mb);
  void aev_zhuzai_fen_shen_destroy(mblock *mb);
  void aev_be_killed(mblock *mb);
  void aev_jing_ji_end(mblock *mb);
  void aev_to_patrol(mblock *mb);
private:
  skill_info *get_skill(const int skill_cid);
  skill_info *do_select_common_skill(const time_value &now);
  skill_info *do_select_skill_for_c1(const time_value &now, char_obj *, int &);
  skill_info *do_select_skill_for_c2(const time_value &now, char_obj *, int &);
  skill_info *do_select_skill_for_c3(const time_value &now, char_obj *, int &);

  void to_clear_zhu_zai_fen_shen();
private:
  int char_id_;
  int guild_id_;
  int last_use_skill_cid_;
  int last_select_skill_idx_;
  int jing_ji_end_time_;
  int to_patrol_time_;
  ilist<int> zhuzai_fen_shen_;
  ilist<coord_t> move_path_;
};

#endif // PLAYER_COPY_MST_H_

