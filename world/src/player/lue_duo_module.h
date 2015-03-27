// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-09-03 10:00:00
 */
//========================================================================

#ifndef LUE_DUO_MODULE_H_
#define LUE_DUO_MODULE_H_

// Forward declarations
class player_obj;
class in_stream;

/**
 * @class lue_duo_module
 * 
 * @brief
 */
class lue_duo_module
{
public:
  static void destroy(player_obj *player);

  // handle client msg
  static int dispatch_msg(player_obj *,
                          const int msg_id,
                          const char *msg,
                          const int len);

  static void on_lue_duo_end(const int, const bool);
  static void deal_preload(const int, const int, const int, const int);
  static void on_char_logout(player_obj *);
  static void on_enter_scene(player_obj *);

  static void save_lueduo_log(player_obj *, const int , const int, const int,
                              const bool, const int time = 0);

  static void handle_db_get_lueduo_log_result(player_obj *, in_stream &);
private:
  static int clt_get_material_holder_list(player_obj *, const char *, const int);
  static int clt_rob_someone(player_obj *, const char *, const int);
  static int clt_get_log_robbed(player_obj *, const char *, const int);
  static int clt_revenge(player_obj *, const char *, const int);
  static int clt_fanpai(player_obj *, const char *, const int);

private:
  static bool is_material_ok(const int );
  static bool has_material(const int , const  int);
  static int  material_cnt(const int , const  int);

  static bool rand_if_get_material(player_obj * , const int );
  static int  transfer_item_robbed(player_obj *, const int , const int);

  static int do_rob_someone(player_obj *, const int, const int );
};

#endif // LUE_DUO_MODULE_H_

