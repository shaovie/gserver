// -*- C++ -*-

//========================================================================
/**
 * Author   : alvinhu
 * Date     : 2012-11-14 15:10
 */
//========================================================================

#ifndef MAIL_MODULE_H_
#define MAIL_MODULE_H_

#include "def.h"
#include "ilist.h"
#include "global_macros.h"

#define MAX_REPLACE_SIZE      10
#define MAX_REPLACE_STR       512

// Forward declarations
class player_obj;
class mail_info;
class mail_header;
class mail_detail;
class out_stream;
class in_stream;
class item_obj;

/**
 * @class mail_module
 *
 * @brief
 */
class mail_module
{
public: // db_result
  static int handle_db_get_mail_detail_result(player_obj *, in_stream &is);
  static int handle_db_get_mail_attach_result(player_obj *, in_stream &is);
  static int handle_db_get_mail_list_result(player_obj *, in_stream &is);
  static int handle_db_get_system_mail_list_result(player_obj *, in_stream &is);
  static int handle_db_get_check_new_mail_result(player_obj *, in_stream &is);
  static int dispatch_msg(player_obj *, const int , const char *, const int );
public:
  static void do_send_mail(const int char_id,
                           const char career,
                           const int db_sid,
                           const int mail_type,
                           const char *sender_name,
                           const char *title,
                           const char *conent,
                           ilist<item_amount_bind_t> &attach_list);

  static void do_send_mail(const int recv_id, const int sender_id,
                           const char *sender_name, const char *title, const char *content,
                           const int mail_type,
                           const int coin, const int diamond, const int b_diamond,
                           const int attach_num, item_obj *attach_list,
                           const int db_sid, const int send_time, const int lvl_limit = 0);
  static int do_send_mail(const int recv_id, const int sender_id,
                          const char *sender_name, const char *title, const char *content,
                          const int mail_type,
                          const int coin, const int diamond, const int b_diamond,
                          const char *items,
                          const int db_sid, const int send_time, const int lvl_limit);
  static void item2json(item_obj *attach_list,
                        const int attach_num,
                        char *items,
                        const int items_size);
  static int json2item(ilist<item_obj*> &attach_list,
                       const char *items,
                       const int items_size);
  static void replace_mail_info(const char *mail_info,
                                const char replace_str[][MAX_REPLACE_STR + 1],
                                const char replace_value[][MAX_REPLACE_STR + 1],
                                const int size,
                                char *new_mail_info);
  static void do_gen_mail_item(item_obj &item,
                               const int item_cid,
                               const int amount,
                               const char bind_type);
  static int do_send_mail_2_all(const char *, const char *title, const char *content,
                                const int coin, const int diamond, const int b_diamond,
                                const char *items, const short lvl);
public:
  static void on_enter_game(player_obj *);
  static void on_new_day();
  static void do_notify_haved_new_mail(player_obj *, const int cnt);
  static int  to_get_system_mail_list(player_obj *);
private: // = client
  static int clt_get_mail_list(player_obj *, const char *msg, const int len);
  static int clt_send_mail(player_obj *, const char *msg, const int len);
  static int clt_obtain_mail_detail(player_obj *, const char *msg, const int len);
  static int clt_pick_up_mail_attach(player_obj *, const char *msg, const int len);
  static int clt_delete_mail(player_obj *, const char *msg, const int len);
private:
  static void to_check_haved_new_mail(player_obj *);
  static int  do_build_mail_head_info(mail_header *, out_stream &out);
  static void do_build_mail_detail_info(player_obj *, mail_info *, out_stream &out);
  static int  do_set_mail_readed(player_obj *, const int mail_id);
  static void do_set_mail_attach_removed(player_obj *, const int mail_id);
  static int  do_get_mail_attach(player_obj *player, mail_detail &md, const int sender_id);
public:
  static char replace_str[MAX_REPLACE_SIZE][MAX_REPLACE_STR + 1];
  static char replace_value[MAX_REPLACE_SIZE][MAX_REPLACE_STR + 1];
};
#endif // MAIL_MODULE_H_
