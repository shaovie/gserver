// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-10-22 11:39
 */
//========================================================================

#ifndef EQUIP_MODULE_H_
#define EQUIP_MODULE_H_

// Forward declarations
namespace Json { class Value; }
class player_obj;
class item_obj;
class equip_cfg_obj;

/**
 * @class equip_module
 * 
 * @brief
 */
class equip_module
{
public:
  static int do_calc_equip_score(item_obj *);
  static void do_calc_attr_affected_by_equip_qh_lvl(item_obj *equip,
                                                    const equip_cfg_obj *eco,
                                                    Json::Value &extra_json,
                                                    int &qh_lvl);
  static void do_calc_attr_affected_by_equip_xi_lian(Json::Value &extra_json);
  static void do_calc_attr_affected_by_equip_suit(player_obj *player);
  static void do_calc_attr_affected_by_all_qh(const int all_qh_lvl);
};

#endif // EQUIP_MODULE_H_

