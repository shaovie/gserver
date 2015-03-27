#include "player_mgr.h"
#include "player_obj.h"
#include "mblock_pool.h"
#include "sys_log.h"
#include "ilist.h"

// Lib header
#include <string>
#include <tr1/unordered_map>
#include "ev_handler.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

class player_loop_timer : public ev_handler
{
public:
  void insert(player_obj *obj) { this->player_obj_list_.push_back(obj); }
  void remove(player_obj *obj) { this->player_obj_list_.remove(obj); }

  virtual int handle_timeout(const time_value &now)
  {
    int size = this->player_obj_list_.size();
    for (int i = 0; i < size; ++i)
    {
      player_obj *p = this->player_obj_list_.pop_front();
      if (p->do_timeout(now) != 0)
        p->handle_close(-1, ev_handler::timer_mask);
      else
        this->player_obj_list_.push_back(p);
    }
    return 0;
  }
private:
  ilist<player_obj *> player_obj_list_;
};
/**
 * @class player_mgr_impl
 *
 * @brief implement of player_mgr
 */
class player_mgr_impl
{
public:
  typedef std::tr1::unordered_map<std::string, player_obj *> account_map_t;
  typedef std::tr1::unordered_map<std::string, player_obj *>::iterator account_map_itor;
  typedef std::tr1::unordered_map<int, player_obj *> player_obj_map_t;
  typedef std::tr1::unordered_map<int, player_obj *>::iterator player_obj_map_itor;

  player_mgr_impl() :
    account_map_(2500),
    player_obj_map_(2500)
  { }
  int init(reactor *r)
  {
    for (size_t i = 0;
         i < sizeof(this->player_loop_timer_)/sizeof(this->player_loop_timer_[0]);
         ++i)
    {
      this->player_loop_timer_[i].set_reactor(r);
      if (r->schedule_timer(&this->player_loop_timer_[i],
                            time_value(0, (rand() % 500 + 100)*1000),
                            time_value(0, 30*1000)) == -1)
        return -1;
    }
    return 0;
  }
  void attach_timer(player_obj *p)
  {
    int idx = p->db_sid() % (sizeof(this->player_loop_timer_)/sizeof(this->player_loop_timer_[0]));
    this->player_loop_timer_[idx].insert(p);
  }
  void deattach_timer(player_obj *p)
  {
    int idx = p->db_sid() % (sizeof(this->player_loop_timer_)/sizeof(this->player_loop_timer_[0]));
    this->player_loop_timer_[idx].remove(p);
  }
  //= account
  void insert(const char *account, player_obj *obj)
  { this->account_map_.insert(std::make_pair(std::string(account), obj)); }
  player_obj *find(const char *account)
  {
    account_map_itor itor = this->account_map_.find(std::string(account));
    return itor == this->account_map_.end() ? NULL : itor->second;
  }
  void remove(const char *account)
  { this->account_map_.erase(std::string(account)); }

  //= char
  void insert(const int char_id, player_obj *obj)
  { this->player_obj_map_.insert(std::make_pair(char_id, obj)); }
  player_obj *find(const int char_id)
  {
    player_obj_map_itor itor = this->player_obj_map_.find(char_id);
    return itor == this->player_obj_map_.end() ? NULL : itor->second;
  }
  void remove(const int char_id) { this->player_obj_map_.erase(char_id); }
  int size() { return this->player_obj_map_.size(); }

  void post_aev_to_world(const int aev_id, mblock *ev_mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
    {
      mblock *mb = NULL;
      if (ev_mb != NULL)
      {
        mb = mblock_pool::instance()->alloc(ev_mb->length());
        mb->copy(ev_mb->rd_ptr(), ev_mb->length());
      }
      itor->second->post_aev(aev_id, mb);
    }
    if (ev_mb != NULL)
      mblock_pool::instance()->release(ev_mb);
  }
  void post_aev_to_scene(const int scene_id, const int aev_id, mblock *ev_mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
    {
      if (itor->second->scene_id() != scene_id) continue;
      mblock *mb = NULL;
      if (ev_mb != NULL)
      {
        mb = mblock_pool::instance()->alloc(ev_mb->length());
        mb->copy(ev_mb->rd_ptr(), ev_mb->length());
      }
      itor->second->post_aev(aev_id, mb);
    }
    if (ev_mb != NULL)
      mblock_pool::instance()->release(ev_mb);
  }
  void post_aev_to_view_area(const int scene_id, const short x, const short y, const int aev_id, mblock *ev_mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
    {
      if (itor->second->scene_id() != scene_id
          || !util::is_in_view_area(x, y,
                                    itor->second->coord_x(), itor->second->coord_y(),
                                    CLT_VIEW_WIDTH_RADIUS, CLT_VIEW_HEIGHT_RADIUS))
        continue;
      mblock *mb = NULL;
      if (ev_mb != NULL)
      {
        mb = mblock_pool::instance()->alloc(ev_mb->length());
        mb->copy(ev_mb->rd_ptr(), ev_mb->length());
      }
      itor->second->post_aev(aev_id, mb);
    }
    if (ev_mb != NULL)
      mblock_pool::instance()->release(ev_mb);
  }
  void post_aev_to_guild(const int guild_id, const int aev_id, mblock *ev_mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
    {
      if (itor->second->guild_id() != guild_id) continue;
      mblock *mb = NULL;
      if (ev_mb != NULL)
      {
        mb = mblock_pool::instance()->alloc(ev_mb->length());
        mb->copy(ev_mb->rd_ptr(), ev_mb->length());
      }
      itor->second->post_aev(aev_id, mb);
    }
    if (ev_mb != NULL)
      mblock_pool::instance()->release(ev_mb);
  }
  void broadcast_to_world(const int msg_id, mblock *mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
      itor->second->do_delivery(msg_id, mb);
  }
  void broadcast_to_scene(const int scene_id, const int msg_id, mblock *mb)
  {
    player_obj_map_itor itor = this->player_obj_map_.begin();
    for (; itor != this->player_obj_map_.end(); ++itor)
    {
      if (itor->second->scene_id() == scene_id)
        itor->second->do_delivery(msg_id, mb);
    }
  }
  void get_ol_char_list(ilist<int/*char_id*/> &list)
  {
    for (player_obj_map_itor itor = this->player_obj_map_.begin();
         itor != this->player_obj_map_.end();
         ++itor)
      list.push_back(itor->first);
  }
private:
  account_map_t account_map_;
  player_obj_map_t player_obj_map_;
  player_loop_timer player_loop_timer_[3];
};
player_mgr::player_mgr() : impl_(new player_mgr_impl()) { }
int player_mgr::init(reactor *r)
{ return this->impl_->init(r); }
void player_mgr::attach_timer(player_obj *p)
{ this->impl_->attach_timer(p); }
void player_mgr::deattach_timer(player_obj *p)
{ this->impl_->deattach_timer(p); }
//
void player_mgr::insert(const char *ac, player_obj *obj)
{ this->impl_->insert(ac, obj); }
player_obj *player_mgr::find(const char *ac)
{ return this->impl_->find(ac); }
void player_mgr::remove(const char *ac)
{ this->impl_->remove(ac); }
//
void player_mgr::insert(const int char_id, player_obj *obj)
{ this->impl_->insert(char_id, obj); }
player_obj *player_mgr::find(const int char_id)
{ return this->impl_->find(char_id); }
void player_mgr::remove(const int char_id)
{ this->impl_->remove(char_id); }
//
int player_mgr::size()
{ return this->impl_->size(); }
void player_mgr::post_aev_to_world(const int aev_id, mblock *mb)
{ this->impl_->post_aev_to_world(aev_id, mb); }
void player_mgr::post_aev_to_scene(const int scene_id, const int aev_id, mblock *mb)
{ this->impl_->post_aev_to_scene(scene_id, aev_id, mb); }
void player_mgr::post_aev_to_view_area(const int scene_id, const short x, const short y, const int aev_id, mblock *mb)
{ this->impl_->post_aev_to_view_area(scene_id, x, y, aev_id, mb); }
void player_mgr::post_aev_to_guild(const int guild_id, const int aev_id, mblock *mb)
{ this->impl_->post_aev_to_guild(guild_id, aev_id, mb); }
void player_mgr::broadcast_to_world(const int msg_id, mblock *mb)
{ this->impl_->broadcast_to_world(msg_id, mb); }
void player_mgr::broadcast_to_scene(const int scene_id, const int msg_id, mblock *mb)
{ this->impl_->broadcast_to_scene(scene_id, msg_id, mb); }
void player_mgr::get_ol_char_list(ilist<int> &list)
{ this->impl_->get_ol_char_list(list); }
