// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-12-27 18:06
 */
//========================================================================

#ifndef IU_MODULE_H_
#define IU_MODULE_H_

// Forward declarations
class player_obj;
class item_obj;
class equip_strengthen_cfg_obj;
class equip_xi_lian_obj;
class xi_lian_value_obj;

namespace Json { class Value; }

/**
 * @class iu_module
 * 
 * @brief
 */
class iu_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static void on_new_equip_for_xi_lian(item_obj *equip_obj);

private:
  // 装备强化
  static int clt_equip_strengthen(player_obj *, const char *msg, const int len);
  static int clt_equip_one_key_strengthen(player_obj *, const char *msg, const int len);
  static int do_equip_strengthen(player_obj *player,
                                 item_obj *equip_obj,
                                 const equip_strengthen_cfg_obj *esco,
                                 Json::Value &extra_json,
                                 const char auto_buy,
                                 int &cost_coin_amt,
                                 int &total_cost_cai_liao_amt,
                                 int &cost_cai_liao_amt,
                                 int &auto_buy_cai_liao_amt,
                                 int &new_qh_lvl,
                                 char &error);
  static void on_equip_strengthen_over(player_obj *player,
                                       item_obj *equip_obj);

  // 装备分解
  static int clt_equip_fen_jie(player_obj *, const char *msg, const int len);

  // 装备融合
  static int clt_equip_rong_he(player_obj *, const char *msg, const int len);
  static item_obj *do_equip_rong_he(player_obj *, item_obj *, item_obj *);

  // 装备洗练
  static int clt_equip_xi_lian(player_obj *, const char *msg, const int len);
  static int do_equip_xi_lian(player_obj *player,
                              item_obj *equip_obj,
                              equip_xi_lian_obj *exlo,
                              xi_lian_value_obj *xlvo,
                              const char lock_cnt,
                              const char lock[],
                              const int auto_buy_cai_liao_amt,
                              const int auto_buy_lock_amt);
  static void new_xi_lian_attr(xi_lian_value_obj *xlvo,
                               const int xl_cnt,
                               int count,
                               int cur_xl[][4],
                               int new_xl[][4]);
  static void on_equip_xi_lian_over(player_obj *player,
                                    item_obj *equip_obj,
                                    Json::Value &extra_json,
                                    const int xl_cnt,
                                    const bool cur_change,
                                    int cur_xl[][4],
                                    int new_xl[][4]);
  static int clt_xi_lian_replace(player_obj *, const char *msg, const int len);

  // 装备进阶
  static int clt_equip_jin_jie(player_obj *, const char *msg, const int len);
};

#endif // IU_MODULE_H_

