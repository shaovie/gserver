#include "char_obj.h"
#include "sys_log.h"
#include "buff_info.h"
#include "buff_effect_id.h"
#include "buff_module.h"
#include "player_obj.h"
#include "error.h"
#include "client.h"
#include "message.h"
#include "buff_config.h"
#include "time_util.h"
#include "istream.h"
#include "buff_obj.h"
#include "db_proxy.h"
#include "scene_mgr.h"

// Lib header
#include "macros.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("bf");
static ilog_obj *e_log = err_log::instance()->get_ilog("bf");

/**
 * @class buff_time_map
 * 
 * @brief
 */
typedef std::map<int/*buff id*/, int/*time*/> buff_time_map_t;
typedef std::map<int/*buff id*/, int/*time*/>::iterator buff_time_map_itor;

class buff_time_map { public: buff_time_map_t v_; };

void buff_module::init(char_obj *co)
{ co->buff_time_map_ = new buff_time_map(); }
void buff_module::destroy(char_obj *co)
{
  delete co->buff_time_map_;
  buff_module::do_release_all_buff(co);
}
void buff_module::handle_db_get_buff_list_result(player_obj *player, in_stream &is)
{
  int cnt = 0;
  is >> cnt;
  if (cnt == 0) return ;
  char mb_bf[sizeof(buff_info) + 4] = {0};
  stream_istr mb_bf_si(mb_bf, sizeof(mb_bf));
  is >> mb_bf_si;

  buff_info bi;
  in_stream bf_is(mb_bf_si.str(), mb_bf_si.str_len());
  bf_is >> &bi;

  time_value now = time_value::gettimeofday();
  char *tok_p = NULL;
  char *token = NULL;
  for (token = ::strtok_r(bi.data_, ",", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, ",", &tok_p))
  {
    int buff_id = 0;
    int left_time = 0;
    int left_value = 0;
    ::sscanf(token, "%d:%d:%d", &buff_id, &left_time, &left_value);
    buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(buff_id);
    if (bco == NULL) continue;
    buff_obj *bo = buff_module::alloc(bco->effect_id_);
    if (bo == NULL) continue;
    player->buff_obj_list_.push_back(bo);
    bo->init(player,
             buff_id,
             bco->effect_id_,
             0,
             0, // ????
             left_value,
             left_time,
             bco->interval_,
             0,
             now,
             0,
             0);
    player->buff_effect_cnt_[bco->effect_id_] += 1;
  }
}
int player_obj::db_save_buff()
{
  buff_info bi;
  bi.char_id_ = this->id();
  int len = 0;
  for (ilist_node<buff_obj *> *buff_itor = this->buff_obj_list_.head();
       buff_itor != NULL;
       buff_itor = buff_itor->next_)
  {
    len += ::snprintf(bi.data_ + len, sizeof(bi.data_) - len,
                      "%d:%d:%d,",
                      buff_itor->value_->id_,
                      buff_itor->value_->left_time_,
                      buff_itor->value_->left_value_);
  }
  if (len > 0)
  {
    len -= 1;
    bi.data_[len] = '\0';
  }
  out_stream os(client::send_buf, client::send_buf_len);
  os << this->db_sid_ << &bi;
  if (db_proxy::instance()->send_request(this->id_,
                                         REQ_UPDATE_BUFF_LIST,
                                         &os) != 0)
    return ERR_SERVER_INTERNAL_COMMUNICATION_FAILED;
  return 0;
}
buff_obj *buff_module::alloc(const buff_effect_id_t effect)
{
  buff_obj *bo = NULL;
  int effect_id = effect;
  switch (effect_id)
  {
  case BF_SK_XU_RUO:
  case BF_SK_DING_SHEN:
  case BF_SK_XUAN_YUN:
  case BF_SK_TIE_JIA:
  case BF_SK_FAN_TAN:
    bo = only_timing_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_JIAN_SU:
  case BF_SK_JIAN_SU_MST:
    bo = jian_su_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_XUAN_FENG:
  case BF_SK_HUO_HUAN:
  case BF_SK_HUANG_CHONG:
    bo = last_area_hurt_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_HUI_CHUN:
    bo = last_add_hp_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_BIAN_SHEN:
    bo = bian_shen_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_EWAI_SHANGHAI:
    bo = last_percent_hurt_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_KUANG_BAO:
    bo = add_attr_r_buff_alloctor::instance()->alloc();
    break;
  case BF_SK_HU_DUN:
    bo = consume_value_buff_alloctor::instance()->alloc();
    break;
  }
  return bo;
}
void char_obj::do_update_all_buff(const time_value &now)
{
  if (this->buff_obj_list_.empty()) return ;
  if (now < this->next_update_buff_time_) return ;
  this->next_update_buff_time_ = now + time_value(0, 300*1000);

  int size = this->buff_obj_list_.size();
  for (int i = 0; i < size; ++i)
  {
    buff_obj *bo = this->buff_obj_list_.pop_front();
    if (bo->run(now) != 0)
    {
      this->on_remove_buff(bo);
      this->buff_effect_cnt_[bo->effect_] -= 1;
      bo->remove();
    }else
      this->buff_obj_list_.push_back(bo);
  }
}
void buff_module::on_enter_game(player_obj *player)
{
  if (player->buff_obj_list_.empty()) return ;

  mblock mb(client::send_buf, client::send_buf_len);
  mb.wr_ptr(sizeof(proto_head));

  char *c = mb.wr_ptr();
  mb << (char)0;
  for (ilist_node<buff_obj *> *itor = player->buff_obj_list_.head();
       itor != NULL;
       itor = itor->next_)
  {
    buff_obj *bo = itor->value_;
    mb << bo->id_ << bo->left_time_ << bo->left_value_;
    ++(*c);
    if (*c > 16) break;
  }
  if (*c > 0)
    player->do_delivery(NTF_PUSH_BUFF_LIST, &mb);
}
void buff_module::on_char_logout(player_obj *player)
{
  if (player->buff_obj_list_.empty()) return ;
  int size = player->buff_obj_list_.size();
  for (int i = 0; i < size; ++i)
  {
    buff_obj *bo = player->buff_obj_list_.pop_front();
    if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_OFFLINE_CLR))
    {
      player->buff_effect_cnt_[bo->effect_] -= 1;
      bo->release();
    }else
      player->buff_obj_list_.push_back(bo);
  }
  // save db
  player->db_save_buff();
}
void buff_module::do_release_all_buff(char_obj *co)
{
  while (!co->buff_obj_list_.empty())
  {
    buff_obj *bo = co->buff_obj_list_.pop_front();
    bo->release();
  }
  ::memset(co->buff_effect_cnt_, 0, sizeof(co->buff_effect_cnt_));
}
void buff_module::do_remove_all_buff(char_obj *co)
{
  while (!co->buff_obj_list_.empty())
  {
    buff_obj *bo = co->buff_obj_list_.pop_front();
    co->on_remove_buff(bo);
    co->buff_effect_cnt_[bo->effect_] -= 1;
    bo->remove();
  }
  ::memset(co->buff_effect_cnt_, 0, sizeof(co->buff_effect_cnt_));
}
void buff_module::do_remove_all_debuff(char_obj *co)
{
  int size = co->buff_obj_list_.size();
  for (int i = 0; i < size; ++i)
  {
    buff_obj *bo = co->buff_obj_list_.pop_front();
    buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(bo->id_);
    if (bco != NULL
        && bco->type_ == buff_config::TYPE_DEBUFF)
    {
      co->on_remove_buff(bo);
      co->buff_effect_cnt_[bo->effect_] -= 1;
      bo->remove();
    }else
      co->buff_obj_list_.push_back(bo);
  }
}
void buff_module::do_remove_buff_by_bits(char_obj *co, const int bits)
{
  int size = co->buff_obj_list_.size();
  for (int i = 0; i < size; ++i)
  {
    buff_obj *bo = co->buff_obj_list_.pop_front();
    if (BIT_ENABLED(bo->bits_, bits))
    {
      co->on_remove_buff(bo);
      co->buff_effect_cnt_[bo->effect_] -= 1;
      bo->remove();
    }else
      co->buff_obj_list_.push_back(bo);
  }
}
void char_obj::on_add_buff(buff_obj *bo)
{
  if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_SHOW_TO_OTHER))
    this->broadcast_add_buff(bo);
}
void char_obj::on_remove_buff(buff_obj *bo)
{
  if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_SHOW_TO_OTHER))
    this->broadcast_del_buff(bo);
}
void char_obj::broadcast_add_buff(buff_obj *bo)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_add_buff_msg(mb, bo->id_);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void char_obj::broadcast_del_buff(buff_obj *bo)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_del_buff_msg(mb, bo->id_);
  scene_mgr::instance()->broadcast(this->scene_id_,
                                   0,
                                   false,
                                   this->coord_x_,
                                   this->coord_y_,
                                   &mb);
}
void player_obj::broadcast_add_buff(buff_obj *bo)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_add_buff_msg(mb, bo->id_);
  this->do_broadcast(&mb, false);
}
void player_obj::broadcast_del_buff(buff_obj *bo)
{
  mblock mb(client::send_buf, client::send_buf_len);
  this->do_build_broadcast_del_buff_msg(mb, bo->id_);
  this->do_broadcast(&mb, false);
}
void player_obj::on_add_buff(buff_obj *bo)
{
  if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_SHOW_TO_SELF))
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << bo->id_ << bo->left_time_ << bo->left_value_;
    this->send_request(NTF_CHAR_ADD_BUFF, &os);
  }
  char_obj::on_add_buff(bo);
}
void player_obj::on_modify_buff(buff_obj *bo)
{
  if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_SHOW_TO_SELF))
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << bo->id_ << bo->left_time_ << bo->left_value_;
    this->send_request(NTF_CHAR_ADD_BUFF, &os);
  }
  char_obj::on_modify_buff(bo);
}
void player_obj::on_remove_buff(buff_obj *bo)
{
  if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_SHOW_TO_SELF))
  {
    out_stream os(client::send_buf, client::send_buf_len);
    os << bo->id_;
    this->send_request(NTF_CHAR_DEL_BUFF, &os);
  }
  char_obj::on_remove_buff(bo);
}
void buff_module::on_char_dead(char_obj *co)
{
  if (co->buff_obj_list_.empty()) return ;

  int size = co->buff_obj_list_.size();
  for (int i = 0; i < size; ++i)
  {
    buff_obj *bo = co->buff_obj_list_.pop_front();
    if (BIT_ENABLED(bo->bits_, buff_obj::BF_BIT_DEAD_CLR))
    {
      co->on_remove_buff(bo);
      co->buff_effect_cnt_[bo->effect_] -= 1;
      bo->remove();
    }else
      co->buff_obj_list_.push_back(bo);
  }
}
void char_obj::sk_do_insert_buff(char_obj *target,
                                 const int skill_cid,
                                 const int param,
                                 const time_value &now,
                                 const ilist<sk_rel_buff_info> &buff_info,
                                 const sk_buff_target_t target_type,
                                 const int bits)
{
  if (buff_info.empty()) return ;
  for (ilist_node<sk_rel_buff_info> *itor = buff_info.head();
       itor != NULL;
       itor = itor->next_)
  {
    if (target_type != itor->value_.target_type_) continue;
    buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(itor->value_.id_);
    if (bco == NULL
        || (rand() % 100 + 1) > itor->value_.rate_)
      continue;

    if (target->can_insert_buff(itor->value_.id_, bco->effect_id_) == 0)
    {
      int left_value = 0;
      if (bco->effect_id_ == BF_SK_HU_DUN)
        left_value = this->obj_attr_.gong_ji() * bco->value_ / 100;
      target->do_insert_buff(itor->value_.id_,
                             bco->effect_id_,
                             this->id(),
                             skill_cid,
                             left_value,
                             bco->last_,
                             bco->interval_,
                             bco->value_,
                             now,
                             bits,
                             param);
    }
  }
}
int char_obj::can_insert_buff(const int bf_id,
                              const buff_effect_id_t effect_id)
{
  buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(bf_id);
  if (bco == NULL) return ERR_CONFIG_NOT_EXIST;

  if (bco->ctrl_type_ == buff_config::BF_CTRL_MOVE
      && this->has_buff(BF_SK_XUAN_FENG))
    return ERR_BUFF_CAN_NOT_DJ;

  buff_obj *bo = this->get_buff_by_effect(effect_id);
  if (bo != NULL)
  {
    if (bco->dj_cnt_ == 0) return ERR_BUFF_EXIST;
    if (bo->dj_cnt_ >= bco->dj_cnt_) return ERR_BUFF_EXIST;
    if (bco->mian_yi_time_ > 0)
    {
      buff_time_map_itor itor = this->buff_time_map_->v_.find(effect_id);
      if (itor != this->buff_time_map_->v_.end())
      {
        if (time_util::now - itor->second <= bco->mian_yi_time_)
          return ERR_BUFF_CAN_NOT_DJ;
      }
    }
  }
  if (effect_id == BF_SK_XUAN_FENG)
  {
    if (this->can_move(0, 0) != 0)
      return ERR_BUFF_EXIST;
  }
  return 0;
}
buff_obj *char_obj::get_buff_by_effect(const buff_effect_id_t effect_id)
{
  for (ilist_node<buff_obj *> *buff_itor = this->buff_obj_list_.head();
       buff_itor != NULL;
       buff_itor = buff_itor->next_)
  {
    if (buff_itor->value_->effect_ == effect_id)
      return buff_itor->value_;
  }
  return NULL;
}
int char_obj::do_insert_buff(const int bf_id,
                             const buff_effect_id_t effect_id,
                             const int src_id,
                             const int buff_rel_cid,
                             const int left_value,
                             const int left_time,
                             const int interval,
                             const int value,
                             const time_value &now,
                             const int bits,
                             const int param)
{
  buff_cfg_obj *bco = buff_config::instance()->get_buff_cfg_obj(bf_id);
  if (bco == NULL) return -1;
  buff_obj *cur_bo = this->get_buff_by_effect(effect_id);
  if (cur_bo != NULL)
  {
    if (bco->mian_yi_time_ > 0)
      this->buff_time_map_->v_[effect_id] = now.sec();

    int ret = cur_bo->do_dj(left_value, left_time);
    if (ret == 0)
      this->on_modify_buff(cur_bo); // 如果需要让前端更新BUFF的时间，那就调一下这个方法
    return ret;
  }

  buff_obj *bo = buff_module::alloc(effect_id);
  if (bo == NULL) return -1;

  bo->init(this,
           bf_id,
           effect_id,
           src_id,
           buff_rel_cid,
           left_value,
           left_time,
           interval,
           value,
           now,
           bits,
           param);

  if (bco->mian_yi_time_ > 0)
    this->buff_time_map_->v_[effect_id] = now.sec();

  this->buff_obj_list_.push_back(bo);
  this->buff_effect_cnt_[effect_id] += 1;

  this->on_add_buff(bo);
  return 0;
}
