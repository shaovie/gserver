// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-06-27 16:01
 */
//========================================================================

#ifndef TB_MSG_MAP_H_
#define TB_MSG_MAP_H_

#include "singleton.h"

// Forward declarations
class proxy_obj;
class tb_msg_handler;
class tb_msg_map_impl;

class tb_msg_map : public singleton<tb_msg_map>
{
  friend class tb_msg_maper;
  friend class singleton<tb_msg_map>;
public:
  int call(proxy_obj *, const int msg_id, const char *msg, const int len);
private:
  void reg(const int msg_id, tb_msg_handler *tmh);
private:
  tb_msg_map();

  tb_msg_map_impl *impl_;
};

class tb_msg_maper
{
public:
  tb_msg_maper(const int msg_id, tb_msg_handler *tmh);
};
#endif // TB_MSG_MAP_H_

