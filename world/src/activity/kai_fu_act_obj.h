// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-10-29 11:09
 */
//========================================================================

#ifndef KAI_FU_ACT_OBJ_H_
#define KAI_FU_ACT_OBJ_H_

#include "def.h"
#include "ilist.h"
#include "activity_obj.h"

// Forward declarations
class player_obj;
class in_stream;
class kai_fu_act_cfg_obj;
class kai_fu_act_log;

/**
 * @class kai_fu_act_obj
 * 
 * @brief
 */
class kai_fu_act_obj : public activity_obj
{
  typedef activity_obj super;
public:
  enum
  {

  };
  kai_fu_act_obj();
  virtual ~kai_fu_act_obj() { }

  virtual int close();

  //= static method
  static void destroy(player_obj *player);
  static int  parse_cfg_param(const char *);
  static void handle_db_get_kai_fu_act_log_result(player_obj *, in_stream &is); 

  static void on_enter_game(player_obj *); 
  static void on_char_lvl_up(player_obj *); 
  static void on_zhan_li_update(player_obj *); 
  static void on_all_qh_lvl_update(player_obj *, const int all_lvl);
  static void on_all_equip_color_update(player_obj *, const int all_color);
  static void on_all_bao_shi_lvl_update(player_obj *, const int all_lvl);
  static void on_all_pskill_lvl_update(player_obj *, const int all_lvl);
  static void on_all_tian_fu_lvl_update(player_obj *, const int all_lvl);
  static void on_fa_bao_dj_update(player_obj *, const int dj);
  static void on_tui_tu_end(player_obj *, const int chapter);

  static void do_give_act_award(const int char_id,
                                const int mail_id,
                                const int value,
                                ilist<item_amount_bind_t> &award_list);
  static void do_obtain_kai_fu_act_rank(player_obj *player,
                                        const int act_type);
  static void do_obtain_kai_fu_act_state(player_obj *player,
                                         const int act_type);
protected:
  virtual bool do_check_if_opened(const int now);

  static void db_insert_log(player_obj *player, kai_fu_act_log *kp);
  static void do_update_act_log(player_obj *player,
                                const int mail_id,
                                const int act_type,
                                const int value);
  static bool had_got_act_award(player_obj *player,
                                const int act_type,
                                const int value);
  static void build_rank_award_info(const int delay,
                                    const int mail_id,
                                    ilist<kai_fu_act_cfg_obj *> *lp,
                                    ilist<int> &char_list);
  static int do_obtain_kai_fu_act_rank_i(const int id, ilist<int> &id_list);
};

#endif // KAI_FU_ACT_OBJ_H_

