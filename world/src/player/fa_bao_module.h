// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-09-10 15:06
 */
//========================================================================

#ifndef FA_BAO_MODULE_H_
#define FA_BAO_MODULE_H_

// Forward declarations
class char_obj;
class player_obj;

/**
 * @class fa_bao_module
 * 
 * @brief
 */
class fa_bao_module
{
public:
  static void destroy(char_obj *);
  static int  dispatch_msg(player_obj *, const int , const char *, const int );

  static void parse_fa_bao_value(const char *, int *fa_bao_info);
  static void set_part_value(player_obj *player, const int part);
  static char fa_bao_dj(char_obj *);

  static void on_enter_game(player_obj *player);
  static void on_transfer_scene(player_obj *player, const int to_scene_cid);
  static int do_calc_hurt_for_attacker(char_obj *, const int hurt);
  static int do_calc_hurt_for_defender(char_obj *, const int hurt);
  static void do_timeout(char_obj *, const int);
  static void do_build_fa_bao_attr(char_obj *co,
                                   const char fa_bao_dj,
                                   const char *fa_bao);
private:
  static int clt_takeup_fa_bao(player_obj *, const char *, const int );
  static int clt_he_cheng_fa_bao(player_obj *, const char *, const int );
  static int clt_fa_bao_jin_jie(player_obj *, const char *, const int );
};

#endif // FA_BAO_MODULE_H_

