#include "tb_msg_map.h"
#include "sys_log.h"
#include "message.h"
#include "array_t.h"
#include "proxy_obj.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("base");

/**
 * @class tb_msg_map_impl
 * 
 * @brief implement of tb_msg_map
 */
class tb_msg_map_impl
{
public:
  tb_msg_map_impl() : tb_msg_handler_map_(MAX_DB_MSG_ID) { }

  void reg(const int msg_id, tb_msg_handler *tmh)
  { this->tb_msg_handler_map_.insert(msg_id, tmh); }

  int call(proxy_obj *po, const int msg_id, const char *msg, const int len)
  {
    tb_msg_handler *th = this->tb_msg_handler_map_.find(msg_id);
    if (th == NULL)
    {
      e_log->error("unknow msg id %d", msg_id);
      return 0;
    }
    return th->handle_msg(po, msg, len);
  }
private:
  array_t<tb_msg_handler *> tb_msg_handler_map_;
};
// -------------------------------------------------------
tb_msg_map::tb_msg_map() : impl_(new tb_msg_map_impl()) { }
int tb_msg_map::call(proxy_obj *po, 
                     const int msg_id, 
                     const char *msg, 
                     const int len)
{ return this->impl_->call(po, msg_id, msg, len); }
void tb_msg_map::reg(const int msg_id, tb_msg_handler *tmh)
{ this->impl_->reg(msg_id, tmh); }
// -------------------------------------------------------
tb_msg_maper::tb_msg_maper(const int msg_id, tb_msg_handler *th)
{ tb_msg_map::instance()->reg(msg_id, th); }
