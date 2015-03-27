#include "social_module.h"
#include "istream.h"
#include "player.h"
#include "message.h"
#include "sys_log.h"
#include "scene_unit_obj.h"

// Lib header
#include <ext/hash_map>

#define MAX_SOCIAL_F_SIZE                      200
#define MAX_SOCIAL_B_SIZE                      50
#define MAX_SOCIAL_E_SIZE                      50
#define R_FRIEND                               1
#define R_BLACK                                2

using namespace __gnu_cxx;

static ilog_obj *s_log = sys_log::instance()->get_ilog("social");
static ilog_obj *e_log = err_log::instance()->get_ilog("social");

/**
 * @class socialer_info
 *
 * @brief
 */
class socialer_info
{
public:
  socialer_info()
    : career_(0),
    online_(0),
    lvl_(0),
    fight_score_(0),
    char_id_(0)
  {
    ::memset(this->name_, 0, sizeof(this->name_));
  }

  char career_;
  char online_;
  short lvl_;
  int fight_score_;
  int char_id_;
  char name_[MAX_NAME_LEN + 1];
};

typedef hash_map<int/*char_id*/, socialer_info *> social_map_t;
typedef hash_map<int/*char_id*/, socialer_info *>::iterator social_map_iter;

/**
 * @class social_relation
 *
 * @brief
 */
class social_relation
{
public:
  social_relation()
    : friend_map_(MAX_SOCIAL_F_SIZE),
    black_map_(MAX_SOCIAL_B_SIZE)
  { }
  ~social_relation()
  {
    social_map_iter f_iter = this->friend_map_.begin();
    for (; f_iter != this->friend_map_.end(); ++f_iter)
      delete f_iter->second;
    social_map_iter b_iter = this->black_map_.begin();
    for (; b_iter != this->black_map_.end(); ++b_iter)
      delete b_iter->second;
  }

  int get_random_friend_id()
  {
    int index = rand() % this->friend_map_.size();
    for (social_map_iter iter = this->friend_map_.begin();
         iter != this->friend_map_.end();
         ++iter, --index)
    {
      if (index == 0)
        return iter->first;
    }
    return 0;
  }
  int get_random_black_id()
  {
    int index = rand() % this->black_map_.size();
    for (social_map_iter iter = this->black_map_.begin();
         iter != this->black_map_.end();
         ++iter, --index)
    {
      if (index == 0)
        return iter->first;
    }
    return 0;
  }

  social_map_t friend_map_;
  social_map_t black_map_;
};

void handle_one_social_info(player *p, const char relation, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    socialer_info *si = new socialer_info();
    stream_istr name_si(si->name_, sizeof(si->name_));
    is >> si->char_id_ >> name_si >> si->career_ >> si->lvl_ >> si->fight_score_ >> si->online_;

    social_module::do_insert_socialer(p, relation, si);
  }
}
void social_module::init(player *p)
{
  p->social_relation_ = new social_relation();
  if (p->social_relation_ == NULL)
    e_log->error("social module NULL");
}
void social_module::destroy(player *p)
{
  delete p->social_relation_;
}
int social_module::dispatch_msg(player *p, const int id, const char *msg, const int len)
{
#define SHORT_CODE(ID, FUNC) case ID:             \
  ret = social_module::FUNC(p, msg, len);    \
  break
#define SHORT_DEFAULT default:                    \
  e_log->error("unknow msg id %d", id);       \
  break

  int ret = 0;
  switch(id)
  {
    SHORT_CODE(NTF_SOCIAL_LIST,             obtain_social_list);
    SHORT_CODE(NTF_ADD_FRIEND,              invite_friend);
    SHORT_CODE(NTF_SOCIAL_OTHER_CHAR_STATE, social_other_char_state);
    SHORT_CODE(NTF_ADD_SOCIALER,            add_socialer);
    SHORT_CODE(NTF_REMOVE_SOCIALER,         remove_socialer);
    SHORT_CODE(NTF_SOCIAL_OTHER_CHAR_LVL_UP, social_other_char_lvl_up);

    //SHORT_DEFAULT;
  }
  if (ret != 0)
    e_log->wning("handle social msg %d return %d", id, ret);
  return ret;
}
int social_module::obtain_social_list(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  while (is.length() > 0)
  {
    char relation = 0;
    is >> relation;
    handle_one_social_info(p, relation, is);
  }
  return 0;
}
int social_module::invite_friend(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  is >> char_id;

  out_stream os(client::send_buf, client::send_buf_len);
  os << char_id;
  // 0:refuse 1:agree
  if (rand() % 2 == 0)
    p->send_request(REQ_ADD_FRIEND, &os);

  return 0;
}
int social_module::social_other_char_state(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  char state = 0;
  char relation = 0;
  is >> char_id >> state >> relation;

  socialer_info *si = social_module::find_socialer_info(p, char_id, relation);
  if (si != NULL)
    si->online_ = state;

  return 0;
}
int social_module::add_socialer(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  char relation = 0;
  is >> relation;

  socialer_info *si = new socialer_info();
  stream_istr name_si(si->name_, sizeof(si->name_));
  is >> si->char_id_ >> name_si >> si->career_ >> si->lvl_ >> si->online_;

  social_module::do_insert_socialer(p, relation, si);

  return 0;
}
int social_module::remove_socialer(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  char relation = 0;
  int char_id = 0;
  is >> relation >> char_id;

  social_module::do_remove_socialer(p, relation, char_id);

  return 0;
}
int social_module::social_other_char_lvl_up(player *p, const char *msg, const int len)
{
  in_stream is(msg, len);
  int char_id = 0;
  short lvl = 0;
  char relation = 0;
  is >> char_id >> lvl >> relation;

  socialer_info *si = social_module::find_socialer_info(p, char_id, relation);
  if (si != NULL)
    si->lvl_ = lvl;

  return 0;
}

void social_module::do_insert_socialer(player *p, const char relation, socialer_info *si)
{
  if (relation == R_FRIEND)
    p->social_relation_->friend_map_.insert(std::make_pair(si->char_id_, si));
  else if (relation == R_BLACK)
    p->social_relation_->black_map_.insert(std::make_pair(si->char_id_, si));
}
void social_module::do_remove_socialer(player *p, const char relation, const int char_id)
{
  socialer_info *si = NULL;
  if (relation == R_FRIEND)
  {
    social_map_iter iter = p->social_relation_->friend_map_.find(char_id);
    if (iter != p->social_relation_->friend_map_.end())
    {
      si = iter->second;
      p->social_relation_->friend_map_.erase(iter);
    }
  }
  else if (relation == R_BLACK)
  {
    social_map_iter iter = p->social_relation_->black_map_.find(char_id);
    if (iter != p->social_relation_->black_map_.end())
    {
      si = iter->second;
      p->social_relation_->black_map_.erase(iter);
    }
  }
  if (si != NULL)
    delete si;
}
socialer_info* social_module::find_socialer_info(player *p,
                                                 const int char_id,
                                                 const char relation)
{
  social_map_iter iter;
  if (relation == R_FRIEND)
  {
    iter = p->social_relation_->friend_map_.find(char_id);
    if (iter == p->social_relation_->friend_map_.end()) return NULL;
  }
  else if (relation == R_BLACK)
  {
    iter = p->social_relation_->black_map_.find(char_id);
    if (iter == p->social_relation_->black_map_.end()) return NULL;
  }else
  {
    return NULL;
  }
  return iter->second;
}
void social_module::do_add_random_socialer(player* p)
{
  scene_unit_obj *su = p->get_random_scene_unit(scene_unit_obj::PLAYER, 0, 100);
  if (su == NULL) return ;

  out_stream os(client::send_buf, client::send_buf_len);
  os << su->id_;
  // 0:friend 1:black
  if (rand() % 2 == 0)
    p->send_request(REQ_ADD_FRIEND, &os);
  else
    p->send_request(REQ_ADD_BLACK, &os);
}
void social_module::do_remove_random_socialer(player* p)
{
  out_stream os(client::send_buf, client::send_buf_len);
  // 0:friend 1:black
  if (rand() % 2 == 0)
  {
    int char_id = p->social_relation_->get_random_friend_id();
    os << 1 << char_id;
    p->send_request(REQ_REMOVE_FRIEND, &os);
  }
  else
  {
    int char_id = p->social_relation_->get_random_black_id();
    os << char_id;
    p->send_request(REQ_REMOVE_BLACK, &os);
  }
}

