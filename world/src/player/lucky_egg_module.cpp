#include "lucky_egg_module.h"
#include "package_module.h"
#include "notice_module.h"
#include "mail_module.h"
#include "mail_config.h"
#include "player_obj.h"
#include "mail_info.h"
#include "time_util.h"
#include "message.h"
#include "client.h"
#include "def.h"
#include "bao_shi_info.h"

// Lib header

void lucky_egg_module::on_equip_strengthen_egg(player_obj *player,
                                               const item_obj *item,
                                               const int lvl)
{
  player->send_request(NTF_EQUIP_STRENGTHEN_LUCKY_EGG, NULL);
  notice_module::equip_strengthen_lucky_egg(player->id(),
                                            player->name(),
                                            item,
                                            lvl);
}
void lucky_egg_module::on_cheng_jiu_egg(player_obj *player,
                                        ilist<item_amount_bind_t> &award_list)
{
  player->send_request(NTF_CHENG_JIU_LUCKY_EGG, NULL);
  notice_module::cheng_jiu_lucky_egg(player->id(), player->name());

  const mail_obj *mo = mail_config::instance()->get_mail_obj(mail_config::CHENG_JIU_LUCKY_EGG_AWARD);
  if (mo == NULL) return ;

  mail_module::do_send_mail(player->id(),
                            player->career(),
                            player->db_sid(),
                            mail_info::MAIL_TYPE_LUCKY_EGG,
                            mo->sender_name_,
                            mo->title_,
                            mo->content_,
                            award_list);
}
void lucky_egg_module::on_passive_skill_egg(player_obj *player,
                                            const int skill_cid,
                                            const int lvl)
{
  player->send_request(NTF_PASSIVE_SKILL_LUCKY_EGG, NULL);
  notice_module::passive_skill_upgrade_lucky_egg(player->id(),
                                                 player->name(),
                                                 skill_cid,
                                                 lvl);
}
void lucky_egg_module::on_bao_shi_upgrade_lucky_egg(player_obj *player,
                                                    const char pos,
                                                    const short lvl)
{
  player->send_request(NTF_BAO_SHI_UPGRADE_LUCKY_EGG, NULL);
  notice_module::bao_shi_upgrade_lucky_egg(player->id(),
                                           player->name(),
                                           pos,
                                           lvl);
}
