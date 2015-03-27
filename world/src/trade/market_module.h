// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-03-31 15:00
 */
//========================================================================

#ifndef MARKET_MODULE_H_
#define MARKET_MODULE_H_

// Forward declarations
class player_obj;
class market_info;
class out_stream;

#include "ilist.h"

/**
 * @class market_module
 *
 * @brief
 */
class market_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static int clt_get_market_list(player_obj *, const char *, const int ); 
  static int clt_sort_by_money(player_obj *, const char *, const int ); 
  static int clt_buy_market_item(player_obj *, const char *, const int ); 
  static int clt_sale_market_item(player_obj *, const char *, const int ); 
  static int clt_sale_market_money(player_obj *, const char *, const int ); 
  static int clt_cancel_market_item(player_obj *, const char *, const int ); 
  static int clt_get_self_sale_list(player_obj *, const char *, const int ); 
  static int clt_search_market_by_name(player_obj *, const char *, const int ); 
  //
  static void market_init();
  static void do_insert_market_info(market_info *mi);
  static void fetch_market_info(market_info *mi, out_stream &os);
  static void insert_market_info_2_db(market_info *mi);
  static void delete_market_info_2_db(const int market_id);
  static int  assign_market_id();
  static void reclaim_market_id(const int market_id);
  static void do_timeout(const int now);

  //
public:
  static int last_check_time;
  static int max_market_id;
  static ilist<int> market_id_pool;
};

#endif  // MARKET_MODULE_H_
