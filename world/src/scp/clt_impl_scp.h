// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-05-08 15:24
 */
//========================================================================

#ifndef CLT_IMPL_SCP_H_
#define CLT_IMPL_SCP_H_

#include "scp_obj.h"

// Forward declarations

/**
 * @class clt_impl_scp
 * 
 * @brief
 */
class clt_impl_scp : public scp_obj
{
public:
  clt_impl_scp(const int scene_cid, const int char_id);
  virtual ~clt_impl_scp();
public:
  virtual int  load_config() { return 0; }
  virtual sub_scp_obj *construct_first_sub_scp(const int scene_cid);
};

#endif // CLT_IMPL_SCP_H_

