// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun
 * Date     : 2014-08-26 16:32
 */
//========================================================================

#ifndef DXC_MODULE_H_
#define DXC_MODULE_H_

#define DXC_FIRST_SCENE    3201
// Forward declarations
class player_obj;
class dxc_cfg_obj;
class in_stream;

/**
 * @class dxc_module
 * 
 * @brief
 */
class dxc_module
{
public:
  // handle client msg
  static int dispatch_msg(player_obj *,
                          const int msg_id,
                          const char *msg,
                          const int len);

  static void handle_db_get_dxc_info_result(player_obj *, in_stream &);
  static void on_char_logout(player_obj *);
private:
  static int clt_enter(player_obj *, const char *, const int );
  static int clt_exit(player_obj *, const char *, const int );
  static int clt_finish(player_obj *, const char *, const int );
  static int clt_get_info(player_obj *, const char *, const int );

private:
  static int do_enter_dxc_scp(player_obj *, const int );
};

#endif // DXC_MODULE_H_

