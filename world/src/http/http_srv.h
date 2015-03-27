// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-21 19:02
 */
//========================================================================

#ifndef HTTP_SRV_H_
#define HTTP_SRV_H_

#include "client.h"

// Forward declarations

/**
 * @class http_srv
 * 
 * @brief
 */
class http_srv : public client
{
public:
  http_srv(const char *client_type);

  virtual ~http_srv();

protected:
  virtual int handle_data();
  virtual int dispatch_msg(const int id, 
                           const int res, 
                           const char *msg, 
                           const int len);

  // = GET  METHOD
  int handle_get_method();

  virtual int dispatch_get_interface(char *interface, char *param_begin);

  int if_gm_kick_player(const char *param);
  int if_st_online(const char *param);
  int if_eg_cfg_update(const char *param_begin);

  // = POST METHOD
  int handle_post_method();

  virtual int dispatch_post_interface(char *interface, char *body);
};

#endif // HTTP_SRV_H_

