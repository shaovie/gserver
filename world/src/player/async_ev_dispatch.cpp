#include "player_obj.h"
#include "task_module.h"
#include "sys_log.h"
#include "istream.h"
#include "message.h"
#include "clsid.h"
#include "global_param_cfg.h"
#include "scene_config.h"
#include "scp_module.h"
#include "scp_obj.h"
#include "mail_module.h"

// Lib header
#include "mblock.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("player");
static ilog_obj *e_log = err_log::instance()->get_ilog("player");

void player_obj::dispatch_aev(const int aev_id, mblock *mb)
{
#define AEV_SHORT_CODE(ID, FUNC) case ID:  \
  this->FUNC(mb);                          \
  break
#define AEV_SHORT_DEFAULT default:         \
  e_log->error("unknow aev id %d", aev_id);\
  break

  //
  switch (aev_id)
  {
    AEV_SHORT_CODE(AEV_ON_NEW_DAY, aev_on_new_day);
    AEV_SHORT_CODE(AEV_GOT_EXP, aev_got_exp);
    AEV_SHORT_CODE(AEV_BE_KILLED, aev_be_killed);
    AEV_SHORT_CODE(AEV_KILL_SOMEBODY, aev_kill_somebody);
    AEV_SHORT_CODE(AEV_GOT_MONEY, aev_got_money);
    AEV_SHORT_CODE(AEV_LOSE_MONEY, aev_lose_money);
    AEV_SHORT_CODE(AEV_KILL_MONSTER, aev_kill_monster);
    AEV_SHORT_CODE(AEV_ZHUZAI_FEN_SHEN_INFO, aev_zhuzai_fen_shen);
    AEV_SHORT_CODE(AEV_ZHUZAI_FEN_SHEN_DESTROY, aev_zhuzai_fen_shen_destroy);
    AEV_SHORT_CODE(AEV_KICK_FROM_SCP, aev_kick_from_scp);
    AEV_SHORT_CODE(AEV_NOTIFY_NEW_MAIL, aev_notify_new_mail);
    AEV_SHORT_CODE(AEV_TRANSFER_TO, aev_transfer_to);
    AEV_SHORT_CODE(AEV_GHZ_OVER_AWARD, aev_ghz_over_award);
    AEV_SHORT_CODE(AEV_DIRECT_SEND_MSG, aev_direct_send_msg);
    AEV_SHORT_DEFAULT;
  }
}
void player_obj::aev_on_new_day(mblock *)
{
  this->on_new_day(false, 1);
}
void player_obj::aev_got_exp(mblock *mb)
{
  this->do_got_exp(*((int *)mb->rd_ptr()));
}
void player_obj::aev_be_killed(mblock *mb)
{
  int killer_id   = 0;
  int killer_cid  = 0;
  *mb >> killer_id >> killer_cid;
  this->do_dead(killer_id, killer_cid);
}
void player_obj::aev_got_money(mblock *mb)
{
  int value = 0;
  int money_type  = 0;
  *mb >> money_type >> value;
  this->on_got_money(money_type);
}
void player_obj::aev_lose_money(mblock *mb)
{
  int value = 0;
  int money_type  = 0;
  *mb >> money_type >> value;
}
void player_obj::aev_kill_monster(mblock *mb)
{
  int mst_cid = 0;
  int sort = 0;
  *mb >> mst_cid >> sort;

  task_module::on_kill_mst(this, mst_cid, sort);
}
void player_obj::aev_kill_somebody(mblock *mb)
{
  int char_id = *((int *)mb->rd_ptr());

  this->on_kill_somebody(char_id);
}
void player_obj::aev_zhuzai_fen_shen(mblock *mb)
{
  this->zhuzai_fen_shen_.push_back(*((int *)mb->rd_ptr()));
}
void player_obj::aev_zhuzai_fen_shen_destroy(mblock *mb)
{
  this->zhuzai_fen_shen_.remove(*((int *)mb->rd_ptr()));
}
void player_obj::aev_kick_from_scp(mblock *)
{
  if (!clsid::is_scp_scene(this->scene_cid())) return ;

  this->do_exit_scp();
}
void player_obj::aev_notify_new_mail(mblock *)
{ mail_module::to_get_system_mail_list(this); }
void player_obj::aev_transfer_to(mblock *mb)
{
  int to_cid = 0;
  short to_x = 0;
  short to_y = 0;
  *mb >> to_cid >> to_x >> to_y;
  if (!scene_config::instance()->can_move(to_cid, to_x, to_y))
  {
    e_log->rinfo("can not move to %d %d,%d in %s",
                 to_cid, to_x, to_y, __func__);
    return ;
  }
  this->do_transfer_to(to_cid, to_cid, to_x, to_y);
}
void player_obj::aev_direct_send_msg(mblock *mb)
{
  int msg_id = 0; 
  *mb >> msg_id;
  out_stream os(client::send_buf, client::send_buf_len);
  ::memcpy(os.wr_ptr(), mb->rd_ptr(), mb->length());
  os.wr_ptr(mb->length());
  this->send_request(msg_id, &os);
}
