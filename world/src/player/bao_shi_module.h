// -*- C++ -*-
//========================================================================
/**
 * Author   : michaelwang
 * Date     : 2014-04-24 17:49
 */
//========================================================================

#ifndef BAO_SHI_MODULE_H_
#define BAO_SHI_MODULE_H_

// Forward declarations
class player_obj;
class in_stream;
class out_stream;
class bao_shi_info;

/**
 * @class bao_shi_module
 *
 * @brief
 */
class bao_shi_module
{
public:
  static int  handle_db_get_bao_shi_result(player_obj *, in_stream &is);
  static int  dispatch_msg(player_obj *, const int , const char *, const int );
  static void destroy(player_obj *);

  static int get_bao_shi_all_lvl(player_obj *player);
  static int get_sum_bao_shi_lvl(player_obj *player);
private:
  static int clt_obtain_bao_shi(player_obj *, const char *, const int );
  static int clt_activate_bao_shi(player_obj *, const char *, const int );
  static int clt_upgrade_bao_shi(player_obj *, const char *, const int );
  static int clt_upgrade_bao_shi_to_next(player_obj *, const char *, const int );
private:
  static void do_update_bao_shi_2_db(player_obj *, bao_shi_info *);
  static void do_insert_bao_shi_2_db(player_obj *, bao_shi_info *);
  static void do_notify_bao_shi_2_clt(player_obj *, bao_shi_info *);
  static void do_fetch_bao_shi(bao_shi_info *, out_stream &os);
  static bao_shi_info *find_bao_shi(player_obj *, const char pos, const char bs_idx);
  static short last_row_min_lvl(player_obj *r, const char bs_idx);
  static void do_add_bs_exp(player_obj *, bao_shi_info *, const int add_exp);
  static void on_lvl_up(player_obj *, bao_shi_info *);
  static bool do_luck_lvl_up(player_obj *, bao_shi_info *);
  static void on_all_lvl_update(player_obj *);
  static int do_cnt_upgrade_bao_shi_and_up_stop(player_obj *,
                                                bao_shi_info *,
                                                const char auto_buy,
                                                int &up_cnt,
                                                int &cost_item,
                                                char &result);
};
#endif // BAO_SHI_MODULE_H_
