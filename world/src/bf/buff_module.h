// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-11-14 15:42
 */
//========================================================================

#ifndef BUFF_MODULE_H_
#define BUFF_MODULE_H_

#include "skill_config.h"
#include "buff_effect_id.h"

// Forward declarations
class char_obj;
class buff_obj;
class in_stream;
class player_obj;

/**
 * @class buff_module
 * 
 * @brief
 */
class buff_module
{
public:
  static buff_obj *alloc(const buff_effect_id_t effect_id);
  static void init(char_obj *co);
  static void destroy(char_obj *co);

  static void handle_db_get_buff_list_result(player_obj *, in_stream &);

  static void on_enter_game(player_obj *);
  static void on_char_logout(player_obj *);
  static void on_char_dead(char_obj *co);

  static void do_release_all_buff(char_obj *co);
  static void do_remove_all_buff(char_obj *co);
  static void do_remove_all_debuff(char_obj *co);
  static void do_remove_buff_by_bits(char_obj *co, const int bits);
};

#endif

