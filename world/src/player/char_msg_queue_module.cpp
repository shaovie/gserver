#include "char_msg_queue_module.h"
#include "char_db_msg_queue.h"
#include "player_obj.h"
#include "db_mq_id.h"
#include "db_proxy.h"
#include "behavior_id.h"
#include "message.h"
#include "error.h"
#include "def.h"
#include "sys_log.h"
#include "lue_duo_module.h"
#include "package_module.h"
#include "behavior_id.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("msg_queue");

int char_msg_queue_module::on_char_login(player_obj *player)
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id();
  if (db_proxy::instance()->send_request(player->id(), REQ_GET_CHAR_DB_MSG_LIST, &os) != 0)
    return player->send_respond_err(NTF_OPERATE_RESULT,
                                    ERR_SERVER_INTERNAL_COMMUNICATION_FAILED);
  return 0;
}
void char_msg_queue_module::handle_db_get_char_msg_queue_list_result(player_obj *player,
                                                                     in_stream &is)
{
  int res_cnt = 0;
  is >> res_cnt;
  if (res_cnt == 0) return ;

  ilist<char_db_msg_queue*> msg_queue;

  for (int i = 0; i < res_cnt; ++i)
  {
    char char_msg_bf[sizeof(char_db_msg_queue) + 4] = {0};
    stream_istr char_msg_si(char_msg_bf, sizeof(char_msg_bf));
    is >> char_msg_si;
    char_db_msg_queue *cmq = char_db_msg_queue_pool::instance()->alloc();
    ::memcpy((char *)cmq, char_msg_bf, sizeof(char_db_msg_queue));
    msg_queue.push_back(cmq);
  }
  char_msg_queue_module::handle_char_msg_queue(player, msg_queue);
}
int char_msg_queue_module::handle_char_msg_queue(player_obj *player,
                                                 ilist<char_db_msg_queue*> &msg_queue)
{
  if (msg_queue.empty()) return 0;

  while (!msg_queue.empty())
  {
    char_db_msg_queue *cmq = msg_queue.pop_front();

    //process(player, cmq) here!!!
    switch (cmq->msg_id_)
    {
    case CMQ_JING_JI_AWARD:
      char_msg_queue_module::handle_jing_ji_award(cmq, player);
      break;
    case CMQ_LUEDUO_RESULT:
      char_msg_queue_module::handle_lueduo_result(cmq, player);
      break;
    default:
      e_log->error("unknown db_msg_queue id %d", cmq->msg_id_);
    }

    char_db_msg_queue_pool::instance()->release(cmq);
  }

  out_stream os(client::send_buf, client::send_buf_len);
  os << player->db_sid() << player->id();
  db_proxy::instance()->send_request(player->id(), REQ_DELETE_CHAR_DB_MSG, &os);
  return 0;
}
void char_msg_queue_module::handle_jing_ji_award(char_db_msg_queue *cmq,
                                                 player_obj *player)
{
  int score = 0;
  ::sscanf(cmq->param_, "%d", &score);

  player->char_extra_info_->jing_ji_score_ += score;
  player->db_save_char_info();
}
void char_msg_queue_module::handle_lueduo_result(char_db_msg_queue *cmq,
                                                 player_obj *player)
{
  int robber = 0, material = 0, count = 0, time = 0, result = 0;
  ::sscanf(cmq->param_, "%d,%d,%d,%d,%d",
           &robber, &material, &count, &time, &result);

  if (material != 0 && count != 0)
  {
    int remove_amount = package_module::do_remove_item(player,
                                                       material,
                                                       count,
                                                       ITEM_LOSE_LUEDUO,
                                                       robber);
    if (remove_amount != count)
      e_log->error("%s:just removed %d material(s), need remove %d",
                   __func__, remove_amount, count);
  }

  lue_duo_module::save_lueduo_log(player, robber, material, count, result, time);
}
