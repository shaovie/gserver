#include "player_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "scene_mgr.h"
#include "scene_unit.h"
#include "mblock_pool.h"
#include "package_module.h"
#include "guild_module.h"
#include "vip_module.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

int player_obj::do_scan_snap(const time_value &now)
{
  if (!this->enter_scene_ok_) return 0;
  if (now < this->next_scan_snap_time_) return 0;
  this->next_scan_snap_time_ = now + time_value(0, 1000*360);

  if (this->do_send_snap_slice_list() != 0) return -1;

  int scan_cnt = scene_mgr::instance()->scan_scene_unit_info(this->scene_id_,
                                                             this->coord_x_,
                                                             this->coord_y_,
                                                             this->id_,
                                                             this->snap_slice_list_,
                                                             this->old_snap_unit_list_,
                                                             this->new_snap_unit_list_);

  if (!this->old_snap_unit_list_->empty())
  {
    mblock *slice_mb = mblock_pool::instance()->alloc(MAX_SNAP_SLICE_MB_SIZE);
    while (!this->old_snap_unit_list_->empty())
    {
      pair_t<int> v = this->old_snap_unit_list_->pop_front();
      if (slice_mb->space() < (int)(sizeof(char)*5 + sizeof(int)*3))
      {
        this->snap_slice_list_.push_back(slice_mb);
        slice_mb = mblock_pool::instance()->alloc(MAX_SNAP_SLICE_MB_SIZE);
      }
      *slice_mb << T_CID_ID << v.second_ << v.first_;
      *slice_mb << T_STATUS << (char)SNAP_LIFE << (char)LIFE_V_EXIT;
      *slice_mb << T_END;
      ++(scan_cnt);
    }
    if (slice_mb->length() == 0)
      mblock_pool::instance()->release(slice_mb);
    else
      this->snap_slice_list_.push_back(slice_mb);
  }
  // swap
  ilist<pair_t<int> > *sol = this->new_snap_unit_list_;
  this->new_snap_unit_list_ = this->old_snap_unit_list_;
  this->old_snap_unit_list_ = sol;
  this->new_snap_unit_list_->clear();

  if (scan_cnt == 0) return 0;
  return this->do_send_snap_slice_list();
}
int player_obj::do_send_snap_slice_list()
{
  if (this->snap_slice_list_.empty()) return 0;

  out_stream os(client::send_buf, client::send_buf_len);
  while (!this->snap_slice_list_.empty()
         && os.length() < 2800)
  {
    mblock *mb = this->snap_slice_list_.pop_front();
    ::memcpy(os.wr_ptr(), mb->rd_ptr(), mb->length());
    os.wr_ptr(mb->length());
    mblock_pool::instance()->release(mb);
  }
  return this->send_request(NTF_PUSH_SNAP_INFO, &os);
}
void player_obj::do_clear_snap_units()
{
  if (this->old_snap_unit_list_->empty())
    return ;
  this->old_snap_unit_list_->clear();

  while (!this->snap_slice_list_.empty())
  {
    mblock *mb = this->snap_slice_list_.pop_front();
    mblock_pool::instance()->release(mb);
  }
}
bool player_obj::do_build_snap_info(const int char_id,
                                    out_stream &os,
                                    ilist<pair_t<int> > *old_snap_unit_list,
                                    ilist<pair_t<int> > *new_snap_unit_list)
{
  new_snap_unit_list->push_back(pair_t<int>(this->id_, this->cid_));

  if (this->is_new_one(old_snap_unit_list))
  {
    os << T_CID_ID << this->cid_ << this->id_;
    if (char_id != this->id_)
    {
      this->do_build_snap_base_info(os);
      this->do_build_snap_pos_info(os);
      this->do_build_snap_equip_info(os);
    }
    this->do_build_snap_hp_mp_info(os);
    this->do_build_snap_status_info(os);

    os << T_END;
    return true;
  }

  return false;
}
void player_obj::do_build_snap_base_info(out_stream &os)
{
  os << T_BASE_INFO
    << stream_ostr(this->char_info_->name_,
                   ::strlen(this->char_info_->name_))
    << this->char_info_->career_
    << this->char_info_->lvl_
    << this->char_info_->sin_val_;
  guild_module::do_fetch_char_guild_info(this->guild_id(), os);
  os << vip_module::vip_lvl(this)
    << this->char_info_->cur_title_
    << this->fa_bao_dj()
    << this->all_qh_lvl_
    << this->sum_bao_shi_lvl_;
}
void player_obj::do_build_snap_pos_info(out_stream &os)
{
  os << T_POS_INFO
    << this->dir_
    << this->coord_x_
    << this->coord_y_
    << this->move_speed();
}
void player_obj::do_build_snap_status_info(out_stream &os)
{
  if (BIT_ENABLED(this->unit_status_, OBJ_DEAD))
    os << T_STATUS << (char)SNAP_LIFE << (char)1;
  else
    os << T_STATUS << (char)SNAP_LIFE << (char)0;
}
void player_obj::do_build_snap_equip_info(out_stream &os)
{
  int zhu_wu_cid = 0;
  int fu_wu_cid = 0;
  package_module::get_equip_for_view(this, zhu_wu_cid, fu_wu_cid);
  os << T_EQUIP_INFO << zhu_wu_cid << fu_wu_cid;
}
