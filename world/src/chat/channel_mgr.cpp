#include "channel_mgr.h"
#include "channel.h"
#include "def.h"
#include "util.h"
#include "chat_module.h"

// Lib header
#include <map>

class channel_mgr_impl
{
public:
  channel_mgr_impl()
  { }
  void join(const int channel_id, const int idx, const int char_id)
  {
    channel *ch = this->get_channel(channel_id, idx);
    if (ch == NULL)
    {
      ch = this->create_channel(channel_id, idx);
      if (ch == NULL) return ;
    }
    ch->join(char_id);
  }
  void quit(const int channel_id, const int idx, const int char_id)
  {
    channel *ch = this->get_channel(channel_id, idx);
    if (ch == NULL) return ;

    ch->quit(char_id);
    if (ch->empty())
      this->destroy_channel(channel_id, idx);
  }
  void post(const int channel_id, 
            const int idx,
            const int msg_id,
            out_stream *body)
  {
    channel *ch = this->get_channel(channel_id, idx);
    if (ch != NULL)
      ch->broadcast(msg_id, body);
  }
private:
  channel* get_channel(const int channel_id, const int idx = 0)
  {
    channel_map_t &ref = this->channel_map_[channel_id];
    channel_map_iter itor = ref.find(idx);
    if (itor != ref.end())
      return itor->second;
    return NULL;
  }
  channel* create_channel(const int channel_id, const int idx)
  {
    channel *p = new channel();
    this->channel_map_[channel_id][idx] = p;
    return p;
  }
  void destroy_channel(const int channel_id, const int idx)
  {
    channel_map_t &ref = this->channel_map_[channel_id];
    channel_map_iter itor = ref.find(idx);
    if (itor != ref.end())
    {
      delete itor->second;
      ref.erase(itor);
    }
  }
private:
  typedef std::map<int, channel*> channel_map_t;
  typedef std::map<int, channel*>::iterator channel_map_iter;

  channel_map_t channel_map_[CHANNEL_CNT];
};
//--------------------------------------------------
channel_mgr::channel_mgr() :
  impl_(new channel_mgr_impl())
{ }
void channel_mgr::join(const int channel_id, const int idx, const int char_id)
{ this->impl_->join(channel_id, idx, char_id); }
void channel_mgr::quit(const int channel_id, const int idx, const int char_id)
{ this->impl_->quit(channel_id, idx, char_id); }
void channel_mgr::post(const int channel_id, 
                       const int idx,
                       const int msg_id,
                       out_stream *body)
{ this->impl_->post(channel_id, idx, msg_id, body); }
