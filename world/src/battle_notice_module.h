// -*- C++ -*-
//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-09-10 15:49
 */
//========================================================================

#ifndef BATTLE_NOTICE_MODULE_H_
#define BATTLE_NOTICE_MODULE_H_

class out_stream;
/**
 * @class battle_notice_module
 *
 * @brief
 */
class battle_notice_module
{
public:
  static void char_enter_battle(const int scene_id,
                                const int char_id,
                                const char *name);
  static void char_exit_battle(const int scene_id,
                               const int char_id,
                               const char *name);
  static void char_killed_by_char(const int scene_id,
                                  const int char_id_1,
                                  const char *name_1,
                                  const int char_id_2,
                                  const char *name_2);
  static void char_kill_tz_build(const int scene_id,
                                 const int char_id,
                                 const char *name);
  static void char_kill_jw_build(const int scene_id,
                                 const int char_id,
                                 const char *name);
  static void char_killed_by_tz(const int scene_id,
                                const int char_id,
                                const char *name);
  static void char_killed_by_jw(const int scene_id,
                                const int char_id,
                                const char *name);
private:
  static void build_char(const int char_id, const char *name, out_stream &os);
};

#endif  // BATTLE_NOTICE_MODULE_H_
