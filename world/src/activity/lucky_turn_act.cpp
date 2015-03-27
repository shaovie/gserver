#include "lucky_turn_act.h"
#include "mail_info.h"
#include "mail_config.h"
#include "mail_module.h"
#include "lucky_turn_module.h"

// Lib header

int lucky_turn_act::open()
{
  lucky_turn_module::on_act_open(this->begin_time_);
  this->do_send_mail_when_opened(mail_config::LTIME_TURN_OPENED);
  return 0;
}
int lucky_turn_act::close()
{
  lucky_turn_module::on_act_close();
  return 0;
}
