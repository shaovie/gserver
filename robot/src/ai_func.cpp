#include "ai_func.h"
#include "player.h"
#include "clsid.h"
#include "sys_log.h"
#include "message.h"
#include "scene_config.h"
#include "istream.h"
#include "social_module.h"
#include "team_module.h"
#include "chat_module.h"
#include "market_module.h"
#include "skill_config.h"
#include "scene_unit_obj.h"
#include "macros.h"
#include "util.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

static char gm_str[MAX_GM_STR] = {0};

int ai_func::do_heart_beat(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).msec() < 10*1000) return 0;
  ai.do_time_ = now;

  return p->send_request(REQ_CLT_HEART_BEAT, NULL);
}
static long int calc_move_time(const int dir,
                               const int speed)
{
  // 360P/S
  static double step_distance[] =
  {
    0.0,
    32000.0, 57688.0,  // 32*1000(msec)
    48000.0, 57688.0,
    32000.0, 57688.0,
    48000.0, 57688.0
  };
  return (long int)(step_distance[dir] / (300.0 * speed / 100.0));
}
int ai_func::do_move(player *p, const time_value &now, ai_struct &ai)
{
  if (BIT_ENABLED(p->status_, OBJ_DEAD)
      || clsid::is_tui_tu_scp_scene(p->scene_cid_)
      || p->scene_cid_ == 0
      || now <= p->reach_pos_time_)
    return 0;

  ai.do_time_ = now;

  if (!p->patrol_list_.empty())
  {
    coord_t next_pos = p->patrol_list_.pop_front();
    const int type = scene_unit_obj::PLAYER|scene_unit_obj::MONSTER;
    if (p->get_pos_scene_unit(type, next_pos.x_, next_pos.y_) != NULL)
    {
      p->patrol_list_.clear();
      return 0;
    }

    out_stream os(client::send_buf, client::send_buf_len);
    char dir = util::calc_next_dir(p->x_, p->y_, next_pos.x_, next_pos.y_);
    p->dir_ = dir;
    p->x_ = next_pos.x_;
    p->y_ = next_pos.y_;
    os << p->dir_ << p->x_ << p->y_;
    p->send_request(REQ_CHAR_MOVE, &os);

    long int move_time = calc_move_time(p->dir_, p->speed_);
    p->reach_pos_time_ = now + time_value(0, move_time * 1000);
  }else if (now < p->next_patrol_time_)
    return 0;
  else
  {
    p->do_calc_patrl_path(now);
    p->next_patrol_time_ = now + time_value(5 + rand() % 10, 0);
  }
  return 0;
}
int ai_func::do_move_to(player *p, const time_value &now, ai_struct &ai)
{
  if (ai.done_cnt_ > 0) return 0;
  if (BIT_ENABLED(p->status_, OBJ_DEAD)
      || !clsid::is_tui_tu_scp_scene(p->scene_cid_))
    return 0;

  if (ai.do_time_.sec() == 0
      || (now - ai.do_time_).msec() < 2000)
    return 0;
  ai.do_time_ = now;
  ai.done_cnt_++;

  int to_cid = 1103;
  short to_x = 0;
  short to_y = 0;
  while (!scene_config::instance()->can_move(to_cid, to_x, to_y))
  {
    to_x = rand() % scene_config::instance()->x_len(to_cid);
    to_y = rand() % scene_config::instance()->y_len(to_cid);
  }
  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR, "move %d %d %d", to_cid, to_x, to_y);
  gm_os << so;
  p->scene_cid_ = to_cid;
  p->x_ = to_x;
  p->y_ = to_y;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_lvl_up(player *p, const time_value &now, ai_struct &ai)
{
  if (ai.done_cnt_ > 0) return 0;
  if (ai.do_time_.sec() == 0
      || (now - ai.do_time_).msec() < 2000)
    return 0;

  ai.do_time_ = now;
  ai.done_cnt_++;

  int lvl[] = {15, 28, 32, 45};
  int idx = rand() % (sizeof(lvl)/sizeof(lvl[0]));
  int to_lvl = lvl[idx];
  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR, "gm %d", to_lvl);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_add_remove_socialer(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).msec() < 65*1000)
    return 0;
  ai.do_time_ = now;

  // 0:remove 1,2,3:add
  if (rand() % 4 == 0)
    social_module::do_remove_random_socialer(p);
  else
    social_module::do_add_random_socialer(p);
  return 0;
}
int ai_func::do_chat(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 30) return 0;
  ai.do_time_ = now;

  int r = rand() % 100 + 1;
  if (r > 10) return 0;

  int num = rand()%6;
  if (0 == num)
    chat_module::world_chat(p); 
  else if (1 == num)
    chat_module::team_chat(p);
  else if (2 == num)
    chat_module::private_chat(p);
  //else if (4 == num)
    //chat_module::scene_chat(p);
  else if (5 == num)
    chat_module::flaunt_item(p);
  return 0;
}
int ai_func::do_add_exp(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 33) return 0;
  ai.do_time_ = now;

  if (p->lvl_ >= 80) return 0;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR, "addexp %d", 2000);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_add_skill(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 10) return 0;
  ai.do_time_ = now;

  ilist<int> *career_skill_list = skill_config::instance()->get_skill_list(p->career_);
  if (career_skill_list == NULL
      || career_skill_list->empty())
    return 0;

  int skill_cid = 0;
  for (ilist_node<int> *itor = career_skill_list->head();
       itor != NULL;
       itor = itor->next_)
  {
    if (p->find_skill(itor->value_) == NULL)
    {
      skill_cid = itor->value_;
      break;
    }
  }

  if (skill_cid == 0) return 0;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR, "addskill %d", skill_cid);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_use_skill(player *p, const time_value &now, ai_struct &ai)
{
  if (BIT_ENABLED(p->status_, OBJ_DEAD)
      || clsid::is_tui_tu_scp_scene(p->scene_cid_)
      || now <= p->reach_pos_time_
      || !p->patrol_list_.empty()
      || p->skill_list_.empty())
    return 0;

  ai.do_time_ = now;
  
  return p->do_use_skill();
}
int ai_func::do_team(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).msec() < 60*1000) return 0;
  ai.do_time_ = now;

  if (team_module::team_empty(p))
  {
    int type = rand() % 2;
    if (type == 0)
      team_module::create_team(p);
    else if (type == 1)
      team_module::invite_member(p);

    return 0;
  }

  int type = rand() % 3;
  if (type == 0)
    team_module::kick_member(p);
  else if (type == 1)
    team_module::quit_team(p);
  else if (type == 2)
    team_module::invite_member(p);
  return 0;
}
int ai_func::do_add_monster(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 42) return 0;
  ai.do_time_ = now;

  int r = rand() % 100 + 1;
  if (r > 33) return 0;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);

  ::snprintf(gm_str, MAX_GM_STR, "addmonster %d %d", 51000999, 1);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_mpfull(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 20) return 0;
  ai.do_time_ = now;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);

  ::snprintf(gm_str, MAX_GM_STR, "mpfull");
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_look_other(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 303) return 0;
  ai.do_time_ = now;

  scene_unit_obj *su = p->get_random_scene_unit(scene_unit_obj::PLAYER, OBJ_DEAD, 100);
  if (su == NULL) return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  os << su->id_;
  return p->send_request(REQ_LOOK_OTHER_DETAIL_INFO, &os);
}
int ai_func::do_add_item(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 203) return 0;
  ai.do_time_ = now;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  int item_cid = 22120001;
  ::snprintf(gm_str, MAX_GM_STR, "additem %d 1 %d", item_cid, rand() % 2);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_add_money(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 233) return 0;
  ai.do_time_ = now;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR, "addmoney %d", 999);
  gm_os << so;
  return p->send_request(REQ_GM_CMD, &gm_os);
}
int ai_func::do_market(player *p, const time_value &now, ai_struct &ai)
{
  if ((now - ai.do_time_).sec() < 243) return 0;
  ai.do_time_ = now;

  int r = rand() % 5;
  if (r == 0)
    market_module::get_market_list(p);
  else if (r == 1)
    market_module::sort_by_money(p);
  else if (r == 2)
    market_module::sale_market_item(p);
  else if (r == 3)
    market_module::sale_market_money(p);
  else
    market_module::get_self_sale_list(p);

  return 0;
}
#if 0
int ai_func::do_sort_package(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;

  if (sec - ai.do_time_ < (rand() % 300) + 10)
    return 0;
  out_stream os(client::send_buf, client::send_buf_len);
  int package = BASE_INDEX_PACKAGE;
  os << package;
  ai.do_time_ = sec;
  return p->send_request(REQ_SORT_PACKAGE, &os);
}
int ai_func::do_meridians_xiulian(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (p->career_ == CAREER_XIN_SHOU
      || sec - ai.do_time_ < 4)
    return 0;
  out_stream os(client::send_buf, client::send_buf_len);
  for (int lvl = 1; lvl < 5; ++lvl)
  {
    os << p->meridians_cid_ << lvl;
    p->send_request(REQ_MERIDIANS_XIULIAN, &os);
  }
  p->meridians_cid_++;
  if (p->meridians_cid_ > 8)
    p->meridians_cid_ = 1;
  ai.do_time_ = sec;

  return 0;
}
int ai_func::do_modify_hp_mp(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 5)
    return 0;

  int value = 100;
  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR,
             "modifyhp %d",
             value);
  os << so;
  p->send_request(REQ_GM_CMD, &os);

  os.set_wr_ptr(0);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR,
             "modifymp %d",
             value);
  os << so;
  p->send_request(REQ_GM_CMD, &os);
  ai.do_time_ = sec;

  return 0;
}
int ai_func::do_use_item(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 5)
    return 0;
  out_stream os(client::send_buf, client::send_buf_len);
  os << BASE_INDEX_PACKAGE + rand() % 49 + 1 << (short)1;
  return p->send_request(REQ_USE_ITEM, &os);
}
int ai_func::do_add_pet(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 5)
    return 0;

  if (p->pet_id_ == 0)
  {
    const int pet_egg_cid = 24311001;
    {
      out_stream os(client::send_buf, client::send_buf_len);
      char bf[32] = {0};
      ::snprintf(bf, sizeof(bf), "additem %d 1", pet_egg_cid);
      os << stream_ostr(bf, ::strlen(bf));
      p->send_request(REQ_GM_CMD, &os);
    }
    {
      out_stream os(client::send_buf, client::send_buf_len);
      char bf[32] = {0};
      ::snprintf(bf, sizeof(bf), "useitem %d", pet_egg_cid);
      os << stream_ostr(bf, ::strlen(bf));
      p->send_request(REQ_GM_CMD, &os);
    }
  }
  return 0;
}
int ai_func::do_let_pet_out(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 30)
    return 0;

  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR,
             "petenergy 10");
  gm_os << so;
  p->send_request(REQ_GM_CMD, &gm_os);

  if (p->pet_id_ != 0)
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << p->pet_id_;
    p->send_request(REQ_PET_LET_OUT, &os);
  }
  return 0;
}
int ai_func::do_enter_scp(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (clsid::is_scp_scene(p->scene_cid_))
  {
    if (sec - ai.do_time_ < (120 + rand()%60)) return 0;
    ai.do_time_ = sec;
    return p->send_request(REQ_EXIT_SCP, NULL);
  }

  if (sec - ai.do_time_ < 30)
    return 0;

  ai.do_time_ = sec;

  if (p->scene_cid_ != 2001)
  {
    out_stream gm_os(client::send_buf, client::send_buf_len);
    stream_ostr so(gm_str, MAX_GM_STR);
    ::memset(gm_str, 0, MAX_GM_STR);
    ::snprintf(gm_str, MAX_GM_STR,
               "move 2001 140 158");
    gm_os << so;
    return p->send_request(REQ_GM_CMD, &gm_os);
  }
  out_stream gm_os(client::send_buf, client::send_buf_len);
  stream_ostr so(gm_str, MAX_GM_STR);
  ::memset(gm_str, 0, MAX_GM_STR);
  ::snprintf(gm_str, MAX_GM_STR,
             "clearscp");
  gm_os << so;
  p->send_request(REQ_GM_CMD, &gm_os);

  out_stream os(client::send_buf, client::send_buf_len);
  os << 3007 << (char)0 << (char)0;
  return p->send_request(REQ_ENTER_SCP, &os);
}
int ai_func::do_select_career(player *p, const time_value &, ai_struct &)
{
  if (p->career_ != CAREER_XIN_SHOU)
    return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  p->career_ = (char)(rand() % 4 + 1);
  os << p->career_;
  return p->send_request(REQ_SELECT_CAREER, &os);
}
int ai_func::do_relive(player *p, const time_value &now, ai_struct &ai)
{
  if (p->dead_ == 0) return 0;

  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 3) return 0;
  ai.do_time_ = sec;

  out_stream os(client::send_buf, client::send_buf_len);
  os << 1 << (char)1;
  p->dead_ = 0;
  return p->send_request(REQ_REQUEST_RELIVE, &os);
}
int ai_func::do_get_ol_gift(player *p, const time_value &now, ai_struct &ai)
{
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 30) return 0;

  ai.do_time_ = sec;
  return p->send_request(REQ_OBTAIN_OL_GIFT, NULL);
}
int ai_func::do_get_scp_zhaohui_exp(player *p, const time_value &now, ai_struct &ai)
{
  if (ai.done_cnt_ > 0) return 0;
  int sec = (int)now.sec();
  if (ai.do_time_ == 0) ai.do_time_ = sec;
  if (sec - ai.do_time_ < 30) return 0;

  ai.do_time_ = sec;
  ai.done_cnt_++;
  return p->send_request(REQ_GET_SCP_ZHAOHUI_EXP, NULL);
}
#endif
