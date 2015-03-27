#include "timing_mst_module.h"
#include "timing_mst_config.h"
#include "scene_config.h"
#include "spawn_monster.h"
#include "sys_log.h"
#include "monster_mgr.h"
#include "date_time.h"
#include "notice_module.h"
#include "player_mgr.h"
#include "mblock.h"
#include "client.h"
#include "message.h"

// Lib header

// Defines
#define MST_TIMING_TURN_TIME_CD 60

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

static int s_last_mst_timing_turn = 0;

static int s_timing_mst_ids[MAX_TIMING_MST_IDX] = {0}; // when one no be killed another spawn

// ctime have a value named "time"
void timing_mst_module::do_timeout(const int now)
{
  if (now - s_last_mst_timing_turn < MST_TIMING_TURN_TIME_CD)
    return ;
  s_last_mst_timing_turn = now;

  date_time dt((time_t)now);
  const int month = dt.month();
  const int wday  = dt.wday() == 0 ? 7 : dt.wday();
  const int mday  = dt.mday();
  pair_t<char> hm((char)dt.hour(), (char)dt.min());

  ilist<timing_mst_cfg_obj *> *obj_list = timing_mst_config::instance()->get_timing_mst_list();

  bool if_birth = false;
  for (ilist_node<timing_mst_cfg_obj *> *iter = obj_list->head();
       iter != NULL;
       iter = iter->next_)
  {
    timing_mst_cfg_obj *obj = iter->value_;
    if (monster_mgr::instance()->find(s_timing_mst_ids[(int)obj->index_]))
      continue;

    if (!obj->time_list_.find(hm)
        || (!obj->wday_list_.find(wday) && !obj->mday_list_.find(mday))
        || !obj->month_list_.find(month))
      continue;

    coord_t random_pos = scene_config::instance()->get_random_pos(obj->scene_cid_,
                                                                  obj->coord_x_,
                                                                  obj->coord_y_,
                                                                  obj->range_);

    const int mst_id = spawn_monster::spawn_one(obj->mst_cid_,
                                                0,
                                                obj->scene_cid_,
                                                obj->scene_cid_,
                                                DIR_XX,
                                                random_pos.x_,
                                                random_pos.y_);
    if (mst_id < 0) continue;
    s_timing_mst_ids[(int)obj->index_] = mst_id;

    notice_module::boss_appear(obj->mst_cid_, obj->scene_cid_);
    s_log->rinfo("scene [%d] refresh monster clsid [%d] id [%d]!!",
                 obj->scene_cid_,
                 obj->mst_cid_,
                 mst_id);
    if_birth = true;
  }

  if (if_birth)
  {
    mblock mb(client::send_buf, client::send_buf_len);
    mb.wr_ptr(sizeof(proto_head));
    player_mgr::instance()->broadcast_to_world(NTF_BROADCAST_BOSS_APPEAR, &mb);
  }
}
