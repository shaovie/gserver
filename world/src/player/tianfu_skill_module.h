// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-07 14:34
 */
//========================================================================

#ifndef TIANFU_SKILL_MODULE_H_
#define TIANFU_SKILL_MODULE_H_

// Forward declarations
class in_stream;
class out_stream;
class player_obj;
class char_obj;
class char_brief_info;
class tianfu_skill_info;

/**
 * @class tianfu_skill_module
 *
 * @brief
 */
class tianfu_skill_module
{
public:
  static int dispatch_msg(player_obj *, const int , const char *, const int );

  static int handle_db_get_tianfu_skill_list_result(player_obj *player, in_stream &);
  static void destroy(char_obj *);
  static void on_enter_game(player_obj *player);
  static tianfu_skill_info *get_skill(char_obj *player, const int skill_cid);
  static void fill_char_brief_info(player_obj *player, char_brief_info *cbi);

  static int get_tian_fu_all_lvl(player_obj *player);
private:
  static int clt_upgrade_skill(player_obj *player, const char *msg, const int len);

  static tianfu_skill_info *get_exp(player_obj *player,
                                    tianfu_skill_info *tsi,
                                    const int skill_cid,
                                    const int exp,
                                    int upgrade_exp);
};

#endif // TIANFU_SKILL_MODULE_H_

