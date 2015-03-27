#include "player.h"
#include "message.h"
#include "sys_log.h"
#include "istream.h"
#include "ai_func.h"
#include "scene_unit_obj.h"
#include "social_module.h"
#include "global_param_cfg.h"
#include "team_module.h"
#include "scene_config.h"
#include "skill_config.h"
#include "macros.h"
#include "clsid.h"
#include "util.h"
#include "ghz_activity_obj.h"
#include "ghz_module.h"
#ifdef FOR_ONEROBOT
#include "robot_ncurses.h"
#include "director.h"
#endif
#include "item_obj.h"
#include "chat_module.h"

#include "reactor.h"
#include "connector.h"
#include "time_util.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

DIR_STEP(step);

extern int g_total_payload;
extern int g_curr_payload;

extern int l_total_cnt;
extern lua_State *L;

enum
{
  R_HEART_BEAT = 0,
#ifndef FOR_ONEROBOT
  R_MOVE,
  R_MOVE_TO,
  R_LVL_UP,
  R_MPFULL,
  R_USE_SKILL,
  R_ADD_SKILL,
  //R_ADD_MONSTER,
  R_ADD_ITEM,
  R_ADD_MONEY,
  R_ADD_EXP,
#endif
  R_ADD_REMOVE_SOCIALER,
  R_CHAT,
  R_TEAM,
  R_LOOK_OTHER,
  R_MARKET,
#if 0
  R_SORT_PACKAGE,
  R_MERIDIANS_XIULIAN,
  R_MODIFY_HP_MP,
  R_USE_ITEM,
  R_ADD_PET,
  R_LET_PET_OUT,
  //R_ENTER_SCP,
  R_SELECT_CAREER,
  R_RELIVE,
  R_GET_OL_GIFT,
  R_GET_SCP_ZHAOHUI_EXP,
#endif
  R_END
};
static int (*robot_func[R_END])(player *pl,
                                const time_value &now,
                                ai_struct &ai) =
{
  &ai_func::do_heart_beat,
#ifndef FOR_ONEROBOT
  &ai_func::do_move,
  &ai_func::do_move_to,
  &ai_func::do_lvl_up,
  &ai_func::do_mpfull,
  &ai_func::do_use_skill,
  &ai_func::do_add_skill,
  //&ai_func::do_add_monster,
  &ai_func::do_add_item,
  &ai_func::do_add_money,
  &ai_func::do_add_exp,
#endif
  &ai_func::do_add_remove_socialer,
  &ai_func::do_chat,
  &ai_func::do_team,
  &ai_func::do_look_other,
  &ai_func::do_market,
#if 0
  &ai_func::do_sort_package,
  &ai_func::do_meridians_xiulian,
  &ai_func::do_modify_hp_mp,
  &ai_func::do_use_item,
  &ai_func::do_add_pet,
  &ai_func::do_let_pet_out,
  //&ai_func::do_enter_scp,
  &ai_func::do_select_career,
  &ai_func::do_relive,
  &ai_func::do_get_ol_gift,
  &ai_func::do_get_scp_zhaohui_exp,
#endif
};

player::player(const char *ac, const char *name)
  : client(8192, "world svc"),
  scene_player(0, 0),
  lvl_(0),
  scene_cid_(0),
  last_heart_beat_time_(0),
  att_target_id_(0),
  last_use_skill_cid_(0),
  search_att_target_area_radius_(1),
  diamond_(0),
  b_diamond_(0),
  coin_(0),
  exp_(0),
  ai_(NULL),
  social_relation_(NULL),
  team_info_(NULL)
{ 
  ::strncpy(this->account_, ac, sizeof(this->account_) - 1);
  ::strncpy(this->name_, name, sizeof(this->name_) - 1);
  ++g_curr_payload;
  this->ai_ = new ai_struct[R_END]();
  time_value dt = time_value::gettimeofday() + \
                  time_value(rand() % 10, rand() % 1000 * 1000);
  for (int i = 0; i < R_END; ++i)
  {
    this->ai_[i].param_ = 0;
    this->ai_[i].done_cnt_ = 0;
    this->ai_[i].do_time_ = dt;
  }

  this->next_patrol_time_ = time_value::gettimeofday();
  social_module::init(this);
  team_module::init(this);
}
player::~player()
{
  --g_curr_payload;
#ifdef FOR_LUA
  lua_getglobal(L, "release_ai_struct");
  lua_pushlightuserdata(L, this);
  if (lua_pcall(L, 1, 0, 0) != 0)
    lua_pop(L, 1);
#endif
  if (this->ai_ != NULL)
  {
    delete [] this->ai_;
    this->ai_ = NULL;
  }
  social_module::destroy(this);
  team_module::destroy(this);
  while (!this->item_list_.empty())
    delete this->item_list_.pop_front();
  while (!this->skill_list_.empty())
    delete this->skill_list_.pop_front();
#ifdef FOR_ONEROBOT
  director::instance()->exit();
  exit(0);
#endif
}
int player::open(void *)
{
  if (supper::open(NULL) != 0)
    return -1;

  return this->do_login();
}
int player::handle_timeout(const time_value & now)
{
  if (this->sock_status_ == _SOCK_INIT)
  {
    s_log->error("connect %s timeout!", this->client_type_);
    return -1;
  }

#ifdef FOR_LUA
  lua_getglobal(L,"run_function");
  lua_pushlightuserdata(L, this);
  lua_pushnumber(L, now.sec());
  if (lua_pcall(L, 2, 0, 0) != 0)
  {
    e_log->error("ai_func lua return failed!");
    lua_pop(L, 1);
    return -1;
  }
#else
  for (int idx = R_HEART_BEAT; idx < R_END; ++idx)
  {
    if (robot_func[idx](this, now, this->ai_[idx]) != 0)
      return -1;
  }
#endif

#ifdef FOR_ONEROBOT
  if (director::instance()->draw(now.sec()) != 0)
    return -1;
#endif
  return 0;
}
int player::lua_schedule_timer()
{
  if (this->get_reactor()->schedule_timer(this, 
                                          time_value(0, 0),
                                          time_value(0, 33*1000)) == -1)
    return -1;

  return 0;
}
int player::do_login()
{
  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr ac_so(this->account_, ::strlen(this->account_));
  os << ac_so;
  return this->send_request(REQ_HELLO_WORLD, &os);
}
int player::handle_login_result(const char *msg, 
                                const int len, 
                                const int res)
{
  if (res != 0) 
  {
    e_log->rinfo("%s login failed! %d", this->account_, res);
    return -1;
  }

  char career = 0;
  int char_cnt = 0;
  int char_id = 0;
  in_stream is(msg, len);
  is >> career >> char_id >> char_cnt;

  if (char_cnt == 0)
    return this->do_create_char();

  stream_istr name(this->name_, sizeof(this->name_));
  is >> this->id_ >> name >> this->career_ >> this->lvl_;
  this->cid_ = this->id_;
  this->scene_unit_map_.insert(std::make_pair(this->id_, this));
  return this->do_start_game();
}
int player::do_create_char()
{
#ifdef FOR_ONEROBOT
  login_ncurses::input_name(this->name_);
  if (::strlen(this->name_) == 0)
    exit(0);
#endif
  out_stream os(client::send_buf, client::send_buf_len);
  stream_ostr name_so(this->name_, ::strlen(this->name_));
  os << name_so << (char)(rand() % 3 + 1);
  return this->send_request(REQ_CREATE_CHAR, &os);
}
int player::handle_create_char_result(const char *msg, 
                                      const int len, 
                                      const int res)
{
  if (res != 0)
  {
    e_log->rinfo("%s:%s create char failed %d!", 
                 this->account_, 
                 this->name_, 
                 res);
    return -1;
  }

  in_stream is(msg, len);
  is >> this->id_;
  s_log->rinfo("%s:%s create char id:%d ok!", 
               this->account_, 
               this->name_, 
               this->id_);

  return this->do_start_game();
}
int player::do_start_game()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->id_;
  s_log->rinfo("%s:%d to start game!", this->account_, this->id_);
  return this->send_request(REQ_START_GAME, &os);
}
int player::handle_start_game_result(const char *msg, 
                                     const int len, 
                                     const int res)
{
  if (res != 0) 
  {
    e_log->rinfo("%s:%d start game failed! %d", 
                 this->account_, 
                 this->id_, 
                 res);
    return -1;
  }

  int zhu_wu_cid = 0;
  int fu_wu_cid = 0;
  in_stream is(msg, len);
  stream_istr name_si(this->name_, sizeof(this->name_));
  is >> name_si
    >> this->career_
    >> this->lvl_
    >> this->exp_
    >> this->scene_cid_
    >> this->dir_
    >> this->x_
    >> this->y_
    >> zhu_wu_cid
    >> fu_wu_cid;

  s_log->rinfo("%s:%d start game ok!%d %d.%d",
               this->account_,
               this->id_,
               this->scene_cid_,
               this->x_,
               this->y_);
  if (this->get_reactor()->schedule_timer(this, 
                                          time_value(0, 0),
                                          time_value(0, 33*1000)) == -1)
    return -1;

  this->last_heart_beat_time_ = time_util::now;
  return this->do_enter_game();
}
int player::do_enter_game()
{
#ifdef FOR_ONEROBOT
  director::instance()->init(this);
#endif
  return this->send_request(REQ_ENTER_GAME, NULL);
}
int player::handle_resp_heart_beat(const char *, const int , const int )
{
  this->last_heart_beat_time_ = time_util::now;
  return 0;
}
int player::handle_scene_snap_update(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  do
  {
    scene_unit_obj *su = NULL;
    int cid = 0, id = 0;
    bool new_one = false, completed = false, check_complete = is.length() > 0;

    while (is.length() > 0)
    {
      char type = 0;
      is >> type;
      if (type == T_CID_ID)
      {
        is >> cid >> id;
        scene_unit_map_itor itor = this->scene_unit_map_.find(id);
        if (itor != this->scene_unit_map_.end()) // found
        {
          su = itor->second;
#ifdef FOR_ONEROBOT
          e_log->rinfo("snap found %d:%d %d.%d", cid, id, su->x_, su->y_);
#endif
          continue; 
        }
        //new snap obj
        new_one = true;
        if (cid == id) // is player
          su = new scene_player(cid, id);
        else if (clsid::is_monster(cid))
          su = new scene_monster(cid, id);
        else if (clsid::is_item(cid) || clsid::is_equip(cid))
          su = new scene_item(cid, id);
        else if (clsid::is_skill(cid))
          su = new effect_obj(cid, id);
      }else if (type == T_BASE_INFO)
      {
        scene_player *sp = dynamic_cast<scene_player *>(su);
        stream_istr si(sp->name_, sizeof(sp->name_));
        short lvl = 0;
        short sin_val = 0;
        int guild_id = 0;
        char wang_zu = 0;
        char bf[64] = {0};
        char vip = 0;
        short title = 0;
        char fb = 0;
        short all_qh_lvl = 0;
        short sbs = 0;
        stream_istr gn_si(bf, sizeof(bf));
        is >> si >> sp->career_ >> lvl >> sin_val >> guild_id >> gn_si >> wang_zu >> vip >> title >> fb >> all_qh_lvl >> sbs;
      }else if (type == T_POS_INFO)
      {
        is >> su->dir_ >> su->x_ >> su->y_ >> su->speed_;
      }else if (type == T_STATUS)
      {
        char st_type = 0, st_value = 0;
        is >> st_type >> st_value;
        if (st_type == SNAP_LIFE)
        {
          if (st_value == 2)
            this->on_scene_unit_exit(id);
          else if (st_value == 1)
          {
            SET_BITS(su->status_, OBJ_DEAD);
            if (id == this->id_)
            {
              out_stream os(client::send_buf, client::send_buf_len);
              os << char(2);
              this->send_request(REQ_RELIVE, &os);
            }
          }else if (st_value == 0)
            CLR_BITS(su->status_, OBJ_DEAD);
        }
      }else if (type == T_SHENG_MING_FA_LI)
      {
        is >> su->total_sheng_ming_
          >> su->sheng_ming_
          >> su->total_fa_li_
          >> su->fa_li_;
      }else if (type == T_END)
      {
        completed = true;
        break;
      }else if (type == T_EQUIP_INFO)
      {
        int zhu_wu = 0;
        int fu_wu = 0;
        is >> zhu_wu >> fu_wu;
      }else if (type == T_MST_EXTRA_INFO)
      {
        int v = 0;
        is >> v; 
      }else if (type == T_X_Y_INFO)
      {
        int master_id = 0;
        int owner_id = 0;
        is >> su->x_ >> su->y_ >> master_id >> owner_id;
#ifdef FOR_ONEROBOT
        char bf[128] = {0};
        ::snprintf(bf, sizeof(bf), "drop item: %d %d.%d", su->cid_, su->x_, su->y_);
        director::instance()->log(bf);
#endif
      }else
        assert(false);
    }

    if (check_complete)
      assert(completed);

    if (new_one)
    {
#ifdef FOR_ONEROBOT
      e_log->rinfo("add new su %d:%d", su->cid_, su->id_);
#endif
      this->scene_unit_map_.insert(std::make_pair(id, su));
    }
  } while (is.length() > 0);

  return 0;
}
void player::on_scene_unit_exit(const int id)
{
  if (id == this->att_target_id_)
    this->att_target_id_ = 0;
  scene_unit_map_itor itor = this->scene_unit_map_.find(id);
  if (itor != this->scene_unit_map_.end())
  {
#ifdef FOR_ONEROBOT
    char bf[128] = {0};
    ::snprintf(bf, sizeof(bf), "exit: %s", itor->second->name());
    director::instance()->log(bf);
    e_log->rinfo("%s", bf);
#endif
    delete itor->second;
    this->scene_unit_map_.erase(itor);
  }
}
scene_unit_obj *player::get_random_scene_unit(const int type,
                                              const int no_include_st,
                                              const int area_radius)
{
  scene_unit_obj *result[16];
  size_t cnt = 0;
  scene_unit_map_itor itor = this->scene_unit_map_.begin();
  for (; itor != this->scene_unit_map_.end(); ++itor)
  {
    if (BIT_ENABLED(itor->second->unit_type(), type)
        && util::is_inside_of_redius(this->x_,
                                     this->y_,
                                     itor->second->x_,
                                     itor->second->y_,
                                     area_radius)
        && BIT_DISABLED(itor->second->status_, no_include_st)
        && itor->second->id_ != this->id_)
    {
      result[cnt++] = itor->second;
      if (cnt == sizeof(result)/sizeof(result[0]))
        break;
    }
  }
  if (cnt == 0) return NULL;
  return result[rand() % cnt];
}
scene_unit_obj *player::get_pos_scene_unit(const int type,
                                           const short x,
                                           const short y)
{
  scene_unit_map_itor itor = this->scene_unit_map_.begin();
  for (; itor != this->scene_unit_map_.end(); ++itor)
  {
    if (BIT_ENABLED(itor->second->unit_type(), type)
        && x == itor->second->x_
        && y == itor->second->y_)
      return itor->second;
  }
  return NULL;
}
scene_unit_obj *player::find_scene_unit(const int id)
{
  scene_unit_map_itor itor = this->scene_unit_map_.find(id);
  if (itor == this->scene_unit_map_.end()) return NULL;
  return itor->second;
}
int player::handle_push_skill_list(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  char cnt = 0;
  is >> cnt;

  for (int i = 0; i < cnt; ++i)
  {
    skill_info *si = new skill_info();
    is >> si->cid_ >> si->lvl_;
    this->skill_list_.push_back(si);
  }
  return 0;
}
skill_info *player::find_skill(const int skill_cid)
{
  for (ilist_node<skill_info *> *itor = this->skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == skill_cid)
      return itor->value_;
  }
  return NULL;
}
static int s_init_skill_list[CHAR_CAREER_CNT + 1][5] = {
  {0, 0, 0, 0},
  {LI_LIANG_COMMON_SKILL_1, LI_LIANG_COMMON_SKILL_2, LI_LIANG_COMMON_SKILL_3, 0},
  {MIN_JIE_COMMON_SKILL_1, MIN_JIE_COMMON_SKILL_2, MIN_JIE_COMMON_SKILL_3, MIN_JIE_BIAN_SHEN_COMMON_SKILL, 0},
  {ZHI_LI_COMMON_SKILL_1, ZHI_LI_COMMON_SKILL_2, ZHI_LI_COMMON_SKILL_3, 0},
};
int player::do_use_skill()
{
  short x = 0;
  short y = 0;
  int att_target_id = 0;
  time_value now = time_value::gettimeofday();
  skill_info *select_skill_si = NULL;
  if ((now - this->last_use_skill_time_).msec()
      < global_param_cfg::char_common_cd[(int)this->career_])
    return 0;

  if (this->last_use_skill_cid_ == 0) // first attacking
  {
    select_skill_si = this->find_skill(s_init_skill_list[(int)this->career_][0]);
  }else if (this->last_use_skill_cid_ == s_init_skill_list[(int)this->career_][0])
  {
    select_skill_si = this->find_skill(s_init_skill_list[(int)this->career_][1]);
  }else if (this->last_use_skill_cid_ == s_init_skill_list[(int)this->career_][1])
  {
    select_skill_si = this->find_skill(s_init_skill_list[(int)this->career_][2]);
  }

  if (select_skill_si != NULL)
  {
    const skill_detail *sd = skill_config::instance()->get_detail(select_skill_si->cid_,
                                                                  select_skill_si->lvl_);
    if (sd == NULL
        || ((now - time_value(select_skill_si->use_time_, select_skill_si->use_usec_)).msec()
            < (unsigned long)sd->cd_))
      select_skill_si = NULL;
    else
      this->last_use_skill_cid_ = select_skill_si->cid_;
  }
  if (select_skill_si == NULL)
  {
    int r = rand() % 100 + 1;
    if (r > 50) return 0;
    select_skill_si = this->do_select_skill(now, att_target_id, x, y);
    if (select_skill_si != NULL)
      this->last_use_skill_cid_ = 0;
  }

  if (select_skill_si == NULL) return 0;
  this->last_use_skill_time_ = now;
  out_stream os(client::send_buf, client::send_buf_len);
  os << select_skill_si->cid_ << att_target_id << this->x_ << this->y_;
  return this->send_request(REQ_USE_SKILL, &os);
}
skill_info *player::do_select_skill(const time_value &now,
                                    int &att_target_id,
                                    short &x,
                                    short &y)
{
  for (ilist_node<skill_info *> *itor = this->skill_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    skill_info *si = itor->value_;
    if (si->cid_ == 31110012 // 冲锋
        || si->cid_ == 31210021 // 变身
        || si->cid_ == s_init_skill_list[(int)this->career_][0]
        || si->cid_ == s_init_skill_list[(int)this->career_][1]
        || si->cid_ == s_init_skill_list[(int)this->career_][2]
       )
      continue;

    const skill_detail *sd = skill_config::instance()->get_detail(si->cid_, si->lvl_);
    if (sd == NULL) continue;
    if ((now - time_value(si->use_time_, si->use_usec_)).msec()
        < (unsigned long)sd->cd_)
      continue;
    if (sd->distance_ == 0)
    {
      att_target_id = 0;
      return si;
    }else if (sd->distance_ > 0)
    {
      scene_unit_obj *su = this->find_scene_unit(this->att_target_id_);
      if (su == NULL || BIT_ENABLED(su->status_, OBJ_DEAD))
        su = this->get_random_scene_unit(scene_unit_obj::MONSTER|scene_unit_obj::PLAYER,
                                         OBJ_DEAD,
                                         sd->distance_);
      if (su == NULL) continue;
      this->att_target_id_ = su->id_;
      att_target_id = su->id_;
      x = su->x_; 
      y = su->y_; 

      // do_adjust_dir
      char dir = util::calc_next_dir(this->x_,
                                     this->y_,
                                     x,
                                     y);
      if (dir != 0 && dir != this->dir_)
      {
        this->dir_ = dir;
        out_stream os(client::send_buf, client::send_buf_len);
        os << this->dir_ << this->x_ << this->y_;
        this->send_request(REQ_CHAR_MOVE, &os);
      }
      return si;
    }
  }
  return NULL;
}
void player::do_calc_patrl_path(const time_value &now)
{
  char dir = this->dir_;
  short cur_x = this->x_;
  short cur_y = this->y_;
  for (int i = 0; i < 8; ++i)
  {
    if (rand() % 5 == 0) dir = rand() % 8 + 1;
    short next_x = cur_x + step[(int)dir][0];
    short next_y = cur_y + step[(int)dir][1];
    int c = 0;
    while (!scene_config::instance()->can_move(this->scene_cid_, next_x, next_y))
    {
      dir = rand() % 8 + 1;
      next_x = cur_x + step[(int)dir][0];
      next_y = cur_y + step[(int)dir][1];
      c++;
      if (c > 17) break;
    }
    if (c <= 17)
      this->patrol_list_.push_back(coord_t(next_x, next_y));
    cur_x = next_x;
    cur_y = next_y;
  }
}
int player::handle_other_char_move(const char *msg, const int len, const int )
{
  int id = 0;
  in_stream is(msg, len);
  is >> id;
  scene_unit_obj *su = this->find_scene_unit(id);
  if (su == NULL) return 0;
  is >> su->dir_ >> su->x_ >> su->y_;
  return 0;
}
int player::handle_char_move(const char *msg, const int len, const int res)
{
  if (res == 0) return 0;

#ifdef FOR_ONEROBOT
  char bf[128] = {0};
  ::snprintf(bf, sizeof(bf), "move err: %d", res);
  director::instance()->log(bf);
#endif

  in_stream is(msg, len);
  is >> this->dir_ >> this->x_ >> this->y_;
  return 0;
}
int player::handle_update_sheng_ming_fa_li(const char *msg, const int len, const int )
{
  int id = 0;
  int total_sheng_ming = 0;
  int sheng_ming = 0;
  int total_fa_li = 0;
  int fa_li = 0;
  in_stream is(msg, len);
  is >> id
    >> total_sheng_ming
    >> sheng_ming
    >> total_fa_li
    >> fa_li;
  scene_unit_obj *su = this->find_scene_unit(id);
  if (su == NULL) return 0;
  su->on_sheng_ming_fa_li_update(total_sheng_ming, sheng_ming, total_fa_li, fa_li);
  return 0;
}
int player::handle_broadcast_be_hurt_effect(const char *msg, const int len, const int )
{
  int id = 0;
  int skill_cid = 0;
  int hurt = 0;
  char tip = 0;
  in_stream is(msg, len);
  is >> id
    >> skill_cid
    >> hurt;
  scene_unit_obj *su = this->find_scene_unit(id);
  if (su == NULL) return 0;
  su->on_sheng_ming_fa_li_update(su->total_sheng_ming_,
                                 su->sheng_ming_ - hurt,
                                 su->total_fa_li_,
                                 su->fa_li_);
  return 0;
}
int player::handle_unit_life_status(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  char st = 0;
  int char_id = 0;
  is >> char_id >> st;
  scene_unit_obj *su = this->find_scene_unit(char_id);
  if (su == NULL) return 0;
  if (char_id == this->att_target_id_)
    this->att_target_id_ = 0;
  if (st == 1)
  {
    SET_BITS(su->status_, OBJ_DEAD);

    if (char_id == this->id_)
    {
#ifdef FOR_ONEROBOT
      director::instance()->log("i dead");
#endif
      out_stream os(client::send_buf, client::send_buf_len);
      os << char(2);
      this->send_request(REQ_RELIVE, &os);
    }
  }else if (st == 0)
  {
    CLR_BITS(su->status_, OBJ_DEAD);
#ifdef FOR_ONEROBOT
    if (char_id == this->id_)
      director::instance()->log("i relive");
#endif
  }
  return 0;
}
int player::handle_char_lvl_up(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  int char_id = 0;
  short lvl = 0;
  is >> char_id >> lvl;
  if (char_id != this->id_) return 0;
  this->lvl_ = lvl;
#ifdef FOR_ONEROBOT
  director::instance()->show_char_info(this->name_, this->lvl_, this->career_);
#endif
  return 0;
}
int player::handle_use_skill(const char *msg, const int len, const int res)
{
  in_stream is(msg, len);
  int skill_cid = 0;
  is >> skill_cid;
#ifdef FOR_ONEROBOT
  char bf[128] = {0};
  if (res == 0)
    ::snprintf(bf, sizeof(bf), "use_skill: %d", skill_cid);
  else
    ::snprintf(bf, sizeof(bf), "use_skill err: %d", res);
  director::instance()->log(bf);
#endif

  skill_info *si = this->find_skill(skill_cid);
  if (si == NULL) return 0;
  time_value now = time_value::gettimeofday();
  si->use_time_ = now.sec();
  si->use_usec_ = now.usec();
  return 0;
}
int player::handle_skill_upgrapde_or_learn(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  int skill_cid = 0;
  short lvl = 0;
  is >> skill_cid >> lvl;
#ifdef FOR_ONEROBOT
  char bf[128] = {0};
  if (this->find_skill(skill_cid) == NULL)
    ::snprintf(bf, sizeof(bf), "new skill tip:%d", skill_cid);
  else
    ::snprintf(bf, sizeof(bf), "skill:%d can upgrade", skill_cid);
  director::instance()->log(bf);
#endif
  return 0;
}
int player::handle_update_exp(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  int add_exp = 0;
  is >> this->exp_ >> add_exp;
#ifdef FOR_ONEROBOT
  char bf[128] = {0};
  ::snprintf(bf, sizeof(bf), "exp: +%d", add_exp);
  director::instance()->log(bf);
#endif
  return 0;
}
int player::handle_update_attr(const char *msg, const int len, const int )
{
  int gong_ji = 0;
  int fang_yu = 0;
  int ming_zhong = 0;
  int shan_bi = 0;
  int bao_ji = 0;
  int kang_bao = 0;
  int shang_mian = 0;
  int zhan_li = 0;
  in_stream is(msg, len);
  is >> gong_ji
    >> fang_yu
    >> ming_zhong
    >> shan_bi
    >> bao_ji
    >> kang_bao
    >> shang_mian
    >> zhan_li;
#ifdef FOR_ONEROBOT
  char bf[256] = {0};
  ::snprintf(bf, sizeof(bf),
             "gj:%d fy:%d",
             gong_ji, fang_yu);
  director::instance()->log(bf);

  ::snprintf(bf, sizeof(bf),
             "mz:%d sb:%d",
             ming_zhong, shan_bi);
  director::instance()->log(bf);

  ::snprintf(bf, sizeof(bf),
             "bj:%d kb:%d",
             bao_ji, kang_bao);
  director::instance()->log(bf);

  ::snprintf(bf, sizeof(bf),
             "zhan_li:%d",
             zhan_li);
  director::instance()->log(bf);
#endif
  return 0;
}
int player::handle_update_money(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  is >> this->diamond_ >> this->b_diamond_ >> this->coin_;
#ifdef FOR_ONEROBOT
  char bf[128] = {0};
  ::snprintf(bf, sizeof(bf), "money:%d:%d:%d", this->diamond_, this->b_diamond_, this->coin_);
  director::instance()->log(bf);
#endif
  return 0;
}
int player::handle_update_task_list(const char *msg, const int len, const int )
{
  char cnt = 0;
  in_stream is(msg, len);
  is >> cnt;
  for (int i = 0; i < cnt; ++i)
  {
    char op = 0;
    int task_cid = 0;
    char st = 0;
    int v[4];
    is >> op >> task_cid >> st >> v[0] >> v[1] >> v[2] >> v[3];
#ifdef FOR_ONEROBOT
    char bf[128] = {0};
    char *op_s[] = {"null", "add", "del", "update"};
    char *st_s[] = {"null", "acceptable", "inprogress", "completed"};
    ::snprintf(bf, sizeof(bf), "%s %s task: %d", op_s[(int)op], st_s[(int)st], task_cid);
    director::instance()->log(bf);

#endif
  }
  return 0;
}
int player::handle_push_item_list(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  char package = 0;
  short cap = 0;
  short amount = 0;
  is >> package >> cap >> amount;

  for (int i = 0; i < amount; ++i)
  {
    item_obj *io = new item_obj();
    is >> io->id_
      >> io->cid_
      >> io->amount_
      >> io->bind_
      >> io->attr_float_coe_;
    char bf[2048] = {0};
    stream_istr si(bf, sizeof(bf));
    is >> si;
    this->item_list_.push_back(io);
  }
  return 0;
}
int player::handle_add_item(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  char package = 0;
  item_obj *io = new item_obj();
  is >> package
    >> io->id_
    >> io->cid_
    >> io->amount_
    >> io->bind_
    >> io->attr_float_coe_;
  char bf[2048] = {0};
  stream_istr si(bf, sizeof(bf));
  is >> si;
  this->item_list_.push_back(io);
#ifdef FOR_ONEROBOT
  char log_bf[128] = {0};
  snprintf(log_bf, sizeof(log_bf),
           "get %d item %d bind %d",
           io->amount_, io->cid_, io->bind_);

  director::instance()->log(log_bf);
#endif
  return 0;
}
int player::handle_del_item(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  int item_id = 0;
  is >> item_id;
  item_obj *io = NULL;
  for (ilist_node<item_obj *> *itor = this->item_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->id_ == item_id)
    {
      io = itor->value_;
      break;
    }
  }
  if (io == NULL) return 0;
  this->item_list_.remove(io);
#ifdef FOR_ONEROBOT
  char log_bf[128] = {0};
  snprintf(log_bf, sizeof(log_bf),
           "lose %d item %d bind %d",
           io->amount_, io->cid_, io->bind_);

  director::instance()->log(log_bf);

#endif
  delete io;
  return 0;
}
int player::handle_update_item(const char *msg, const int len, const int )
{
  in_stream is(msg, len);
  int item_id = 0;
  is >> item_id;
  item_obj *io = NULL;
  for (ilist_node<item_obj *> *itor = this->item_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->id_ == item_id)
    {
      io = itor->value_;
      break;
    }
  }
  if (io == NULL) return 0;
  int old_amount = io->amount_;
  while (is.length() > 0)
  {
    char type = 0;
    is >> type;
    if (type == 1)
      is >> io->pkg_;
    else if (type == 2)    // item amount
      is >> io->amount_;
    else if (type == 3)
      is >> io->bind_;
    else if (type == 4)
    {
      char buff[1024] = {0};
      stream_istr si(buff, sizeof(buff));
      is >> si;
    }
  }

#ifdef FOR_ONEROBOT
  char log_bf[128] = {0};
  if (old_amount < io->amount_)
  {
    int get_amount = io->amount_ - old_amount;
    e_log->error("get amount %d", get_amount);
    snprintf(log_bf, sizeof(log_bf),
             "get %d item %d bind %d",
             get_amount, io->cid_, io->bind_);
  }
  else if (old_amount > io->amount_)
  {
    int lose_amount = old_amount - io->amount_;
    e_log->error("lose amount %d", lose_amount);
    snprintf(log_bf, sizeof(log_bf),
             "lose %d item %d bind %d",
             lose_amount, io->cid_, io->bind_);
  }
  director::instance()->log(log_bf);
#endif
  old_amount = 0;
  return 0;
}
item_obj *player::find_item(const int item_cid)
{
  for (ilist_node<item_obj *> *itor = this->item_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (itor->value_->cid_ == item_cid)
      return itor->value_;
  }
  return NULL;
}
int player::dispatch_msg(const int id, const int res, const char *msg, const int len)
{
  int major_id = MAJOR_ID(id);
#ifdef FOR_LUA
  lua_getglobal(L,"dispatch_msg");
  lua_pushlightuserdata(L, this);
  lua_pushnumber(L, id);
  lua_pushnumber(L, res);
  lua_pushlstring(L, msg, len);
  lua_pushnumber(L, len);
  if (lua_pcall(L, 5, 0, 0) != 0)
  {
    e_log->error("dispatch_msg lua return failed!");
    lua_pop(L, 1);
    return -1;
  }
#else
  if (major_id == MJ_LOGIN_MODULE)
  {
    switch (id)
    {
    case RES_HELLO_WORLD:
      return this->handle_login_result(msg, len, res);
    case RES_CREATE_CHAR:
      return this->handle_create_char_result(msg, len, res);
    case RES_START_GAME:
      return this->handle_start_game_result(msg, len, res);
    case NTF_OPERATE_RESULT:
      return this->print_operate_err(id, res);
    case NTF_PLAYER_LOGOUT:
      return this->print_operate_err(id, res);
    }
  }else if (major_id == MJ_BASIC_MODULE)
  {
    switch(id)
    {
    case RES_CLT_HEART_BEAT:
      return this->handle_resp_heart_beat(msg, len, res);
    case NTF_PUSH_SNAP_INFO:
      return this->handle_scene_snap_update(msg, len, res);
    case NTF_BROADCAST_UNIT_LIFE_STATUS:
      return this->handle_unit_life_status(msg, len, res);
    case NTF_BROADCAST_OTHER_CHAR_MOVE:
      return this->handle_other_char_move(msg, len, res);
    case NTF_BROADCAST_HP_MP:
      return this->handle_update_sheng_ming_fa_li(msg, len, res);
    case NTF_BROADCAST_BE_HURT_EFFECT:
      return this->handle_broadcast_be_hurt_effect(msg, len, res);
    case RES_CHAR_MOVE: 
      return this->handle_char_move(msg, len, res);
    case RES_USE_SKILL:
      return this->handle_use_skill(msg, len, res);
    case NTF_BROADCAST_LVL_UP:
      return this->handle_char_lvl_up(msg, len, res);
    case NTF_UPDATE_MONEY:
      return this->handle_update_money(msg, len, res);
    case NTF_UPDATE_EXP:
      return this->handle_update_exp(msg, len, res);
    case NTF_CHAR_ATTR_UPDATE:
      return this->handle_update_attr(msg, len, res);
    }
  }else if (major_id == MJ_PACKAGE_MODULE)
  {
    switch(id)
    {
    case NTF_PUSH_ITEM_LIST:
    case RES_OBTAIN_ITEM_LIST:
      return this->handle_push_item_list(msg, len, res);
    case NTF_ADD_ITEM:
      return this->handle_add_item(msg, len, res);
    case NTF_DEL_ITEM:
      return this->handle_del_item(msg, len, res);
    case NTF_UPD_ITEM:
      return this->handle_update_item(msg, len, res);
    }
  }else if (major_id == MJ_SKILL_MODULE)
  {
    if (id == NTF_PUSH_CHAR_SKILL_LIST)
      return this->handle_push_skill_list(msg, len, res);
    if (id == NTF_CHAR_SKILL_UPDATE)
      return this->handle_skill_upgrapde_or_learn(msg, len, res);
  }else if (major_id == MJ_TASK_MODULE)
  {
    if (id == NTF_TASK_UPDATE)
      return this->handle_update_task_list(msg, len, res);
  }else
  {
    switch(major_id)
    {
    case MJ_SOCIAL_MODULE:
      return social_module::dispatch_msg(this, id, msg, len);
    case MJ_CHAT_MODULE:
      return chat_module::dispatch_msg(this, id, msg, len);
    case MJ_TEAM_MODULE:
      return team_module::dispatch_msg(this, id, msg, len);
    }
  }
#endif
  return 0;
}
int player::print_operate_err(const int msg_id, const int res)
{
  if (res != 0)
  {
#ifdef FOR_ONEROBOT
    char bf[128] = {0};
    ::snprintf(bf, sizeof(bf), "msg:%d err:%d", msg_id, res);
    director::instance()->log(bf);
    s_log->rinfo("msg id %d, result = %d", msg_id, res);
#endif
  }
  return 0;
}
