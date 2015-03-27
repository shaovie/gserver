// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2012-11-29 12:00
 */
//========================================================================

#ifndef CHAR_MSG_QUEUE_MODULE_H_
#define CHAR_MSG_QUEUE_MODULE_H_

#include "istream.h"
#include "ilist.h"

// Forward declarations
class player_obj;
class char_db_msg_queue;

enum
{
  CMQ_JING_JI_AWARD,
  CMQ_LUEDUO_RESULT,

  CMQ_END
};

/**
 * @class char_msg_queue_module
 *
 * @brief
 */
class char_msg_queue_module
{
public:
  static int on_char_login(player_obj *player);
  static void handle_db_get_char_msg_queue_list_result(player_obj *player, 
                                                       in_stream &is);
  
  static void do_release_msg_queue(player_obj *player);
private:
  static int handle_char_msg_queue(player_obj *player, ilist<char_db_msg_queue*> &msg_queue);

  static void handle_jing_ji_award(char_db_msg_queue *cmq, player_obj *player);
  static void handle_lueduo_result(char_db_msg_queue *cmq, player_obj *player);

  char_msg_queue_module();
};

#endif // CHAR_MSG_QUEUE_MODULE_H_

