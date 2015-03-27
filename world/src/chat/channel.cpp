#include "channel.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "istream.h"
#include "ilog.h"
#include "sys_log.h"
#include "mblock_pool.h"

// Lib header
#include <set>
#include <cassert>

static ilog_obj *s_log = sys_log::instance()->get_ilog("channel");
static ilog_obj *e_log = err_log::instance()->get_ilog("channel");

/**
 * @class channel_impl
 * 
 * @brief implement of channel
 */
class channel_impl
{
public:
  bool empty() { return this->char_set_.empty(); }
  void join(const int char_id)
  { this->char_set_.insert(char_id); }
  void quit(const int char_id)
  { this->char_set_.erase(char_id); }
  void broadcast(const int msg_id, out_stream *os)
  {
    static char bf[MAX_CHAT_CONTENT_LEN + sizeof(proto_head) + 1] = {0};
    mblock mb(bf, sizeof(bf));
    mb.wr_ptr(sizeof(proto_head));
    mb.copy(os->get_ptr(), os->length()); 

    for (char_set_iter iter = this->char_set_.begin();
         iter != this->char_set_.end();
         ++iter)
    {
      player_obj* player = player_mgr::instance()->find(*iter);
      if (player != NULL)
        player->do_delivery(msg_id, &mb);
    }
  }
private:
  typedef std::set<int/*char_id*/> char_set_t;
  typedef std::set<int/*char_id*/>::iterator char_set_iter;
  char_set_t char_set_;
};
channel::channel() : impl_(new channel_impl()) { }
channel::~channel()
{
  if (this->impl_)
    delete this->impl_;
}
bool channel::empty() { return this->impl_->empty(); }
void channel::join(const int char_id)
{ this->impl_->join(char_id); }
void channel::quit(const int char_id)
{ return this->impl_->quit(char_id); }
void channel::broadcast(const int msg_id, out_stream *body)
{ this->impl_->broadcast(msg_id, body); }
