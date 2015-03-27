#include "clt_impl_scp.h"
#include "sub_scp_obj.h"
#include "scp_mgr.h"

// Lib header

clt_impl_scp::clt_impl_scp(const int scene_cid, const int char_id) :
  scp_obj(scene_cid, char_id, 0, 0)
{
}
clt_impl_scp::~clt_impl_scp()
{
}
sub_scp_obj *clt_impl_scp::construct_first_sub_scp(const int scene_cid)
{
  int new_scp_id = scp_mgr::instance()->assign_scp_id();
  if (new_scp_id == -1) return NULL;
  return new sub_scp_obj(new_scp_id, scene_cid);
}
