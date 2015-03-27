// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-03-24 12:50
 */
//========================================================================

#ifndef ZHUZAI_FEN_SHEN_MST_H_
#define ZHUZAI_FEN_SHEN_MST_H_

#include "fighting_monster_obj.h"

// Forward declarations

/**
 * @class zhuzai_fen_shen_mst
 * 
 * @brief
 */
class zhuzai_fen_shen_mst : public fighting_monster_obj
{
  typedef fighting_monster_obj super;
public:
  zhuzai_fen_shen_mst();
  ~zhuzai_fen_shen_mst();

  virtual int master_id()const { return this->master_id_; } 
  virtual char pk_mode() const { return this->pk_mode_; } 
  virtual int team_id()  const { return this->team_id_; }
  virtual int guild_id() const { return this->guild_id_;}
  virtual int attack_target() const { return scene_unit::PLAYER|scene_unit::MONSTER; }
  virtual int can_attack(char_obj *target);
  virtual int can_be_attacked(char_obj * /*attacker*/);
  virtual void do_fight_back(const int ) { }
  virtual int  do_think(const time_value &);
protected:
  virtual int  do_enter_scene();
  virtual int do_calc_hurt_by_other_effect_for_defender(int &attacked_tip, const int hurt)
  { return super::do_calc_hurt_by_other_effect_for_defender(attacked_tip, hurt) * 2; }
  virtual void dispatch_aev(const int aev_id, mblock *mb);
private:
  void aev_fen_shen_info(mblock *mb);
  void aev_notify_attack_target_id(mblock *mb);
  void aev_sync_master_use_skill(mblock *mb);
  void aev_zhuzai_fen_shen_to_destroy(mblock *mb);
private:
  char pk_mode_;
  int master_id_;
  int team_id_;
  int guild_id_;
};

#endif // ZHUZAI_FEN_SHEN_MST_H_

