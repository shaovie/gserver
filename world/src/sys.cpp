#include "sys.h"
#include "def.h"
#include "sys_log.h"
#include "time_util.h"
#include "db_proxy.h"
#include "web_service.h"
#include "preload_db.h"
#include "svc_config.h"
#include "global_timer.h"
#include "behavior_log.h"
#include "player_obj.h"
#include "player_mgr.h"
#include "monster_mgr.h"
#include "db_proxy_logic.h"
#include "scene_config.h"
#include "scene_mgr.h"
#include "task_config.h"
#include "item_config.h"
#include "skill_config.h"
#include "passive_skill_cfg.h"
#include "passive_skill_upgrade_cfg.h"
#include "monster_cfg.h"
#include "global_param_cfg.h"
#include "lvl_param_cfg.h"
#include "basic_attr_cfg.h"
#include "zhan_li_calc_cfg.h"
#include "equip_score_cfg.h"
#include "buff_config.h"
#include "spawn_monster.h"
#include "monster_drop_cfg.h"
#include "exit_transfer_cfg.h"
#include "mail_config.h"
#include "scene_monster_cfg.h"
#include "gift_config.h"
#include "shop_config.h"
#include "wild_boss_config.h"
#include "timing_mst_config.h"
#include "iu_config.h"
#include "guild_config.h"
#include "scp_config.h"
#include "tui_tu_config.h"
#include "mis_award_cfg.h"
#include "rank_module.h"
#include "vip_config.h"
#include "zhan_xing_cfg.h"
#include "cheng_jiu_cfg.h"
#include "title_cfg.h"
#include "bao_shi_cfg.h"
#include "huo_yue_du_cfg.h"
#include "jing_ji_cfg.h"
#include "activity_cfg.h"
#include "ltime_act_cfg.h"
#include "activity_module.h"
#include "service_info.h"
#include "name_storage_cfg.h"
#include "recharge_config.h"
#include "lucky_turn_cfg.h"
#include "water_tree_cfg.h"
#include "cross_svc.h"
#include "product_cfg.h"
#include "worship_info.h"
#include "tianfu_skill_upgrade_cfg.h"
#include "tianfu_skill_cfg.h"
#include "suit_config.h"
#include "dxc_config.h"
#include "monster_pos_cfg.h"
#include "fa_bao_config.h"
#include "strength_addition_cfg.h"
#include "lucky_goods_turn_cfg.h"
#include "lucky_goods_draw_cfg.h"
#include "baowu_mall_config.h"
#include "lue_duo_config.h"
#include "kai_fu_act_cfg.h"

// Lib header
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "reactor.h"
#include "acceptor.h"
#include "date_time.h"
#include "time_value.h"

bool     sys::friendly_exit          = false;
short    sys::world_lvl              = 0;
int      sys::last_char_id           = 0;
int      sys::last_guild_id          = 0;
int      sys::last_forbid_id         = 0;
int      sys::svc_launch_time        = 0;
service_info *sys::svc_info          = new service_info();
reactor *sys::r                      = NULL;

static bool s_in_emergency = false;
static int s_selected_career[CHAR_CAREER_CNT + 1];

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

extern const char *g_svc_name;

int sys::init_svc()
{
  time_util::init();
  srand(time_value::start_time.sec() + time_value::start_time.usec());
  ::memset(s_selected_career, 0, sizeof(s_selected_career));

  // check sys config
  if (init_sys_log(g_svc_name) != 0)
    return -1;

  s_log->rinfo("                          init %s", g_svc_name);

  if (svc_config::instance()->load_config(".") != 0)
  {
    e_log->rinfo("load svc config failed!");
    return -1;
  }

  if (sys::load_config() != 0
      || scene_mgr::instance()->init() != 0
      || player_mgr::instance()->init(sys::r) != 0
      || monster_mgr::instance()->init(sys::r) != 0
      || spawn_monster::spawn_all_scene_monster() != 0
      || preload_db::do_preload_db() != 0
      || rank_module::init() != 0
      || activity_module::init() != 0
      || sys::r->schedule_timer(global_timer::instance(),
                                time_value(0, 600*1000),
                                time_value(0, 600*1000)) == -1
     )
    return -1;

  // MUST 上面的逻辑可能会很耗时，导致time_util中缓存的时间不准
  time_util::now = time_value::gettimeofday().sec();
  return sys::launch_net_module();
}
void sys::to_friendly_exit()
{ sys::friendly_exit = true; }
void sys::emergency_exit(const int sig)
{
  struct sigaction action;
  ::sigaction(sig, NULL, &action);
  action.sa_handler = SIG_DFL;
  ::sigaction(sig, &action, NULL);

  if (s_in_emergency) return ;

  s_in_emergency = true;
  s_log->rinfo("there is %d players befor emergency_shutdown",
               player_mgr::instance()->size());

  sev_notify::instance()->notify(SEV_SHUTDOWN, NULL);
  s_log->rinfo("there is %d players after emergency_shutdown",
               player_mgr::instance()->size());
}
void sys::select_career(const char career)
{
  if (util::is_career_ok(career))
    ++(s_selected_career[(int)career]);
}
char sys::assign_career()
{
  int c = CAREER_LI_LIANG;
  int min = s_selected_career[CAREER_LI_LIANG];
  for (int i = CAREER_MIN_JIE; i < CHAR_CAREER_CNT+1; ++i)
  {
    if (s_selected_career[i] < min)
    {
      min = s_selected_career[i];
      c = i;
    }
  }
  return c;
}
int sys::assign_char_id()
{
  if (sys::last_char_id
      >= ((svc_config::instance()->service_sn() + 1) * CHAR_ID_RANGE - 1))
    return -1;
  return ++sys::last_char_id;
}
int sys::assign_guild_id()
{
  if (sys::last_guild_id
      >= ((svc_config::instance()->service_sn() + 1) * GUILD_ID_RANGE - 1))
    return -1;
  return ++sys::last_guild_id;
}
int sys::assign_forbid_id()
{
  if (sys::last_forbid_id
      >= ((svc_config::instance()->service_sn() + 1) * FORBID_ID_RANGE - 1))
    return -1;
  return ++sys::last_forbid_id;
}
void sys::update_svc_info()
{
  out_stream os(client::send_buf, client::send_buf_len);
  os << 0 << stream_ostr((char *)sys::svc_info, sizeof(service_info));
  db_proxy::instance()->send_request(0, REQ_UPDATE_SERVICE_INFO, &os);
}
int sys::launch_net_module()
{
  static acceptor<player_obj> gc_acceptor;
  inet_address local_addr(svc_config::instance()->world_port(), "0.0.0.0");
  if (gc_acceptor.open(local_addr, sys::r) != 0)
  {
    e_log->error("game client acceptor open failed! port: %d",
                 svc_config::instance()->world_port());
    return -1;
  }

  //
  local_addr.set(svc_config::instance()->world_gm_port(),
                 svc_config::instance()->world_gm_host());
  static acceptor<web_service> web_service_acceptor;
  if (web_service_acceptor.open(local_addr, sys::r) != 0)
  {
    e_log->error("web service acceptor open failed! host: %s:%d",
                 svc_config::instance()->world_gm_host(),
                 svc_config::instance()->world_gm_port());
    return -1;
  }

  //
  if (db_proxy::instance()->open(sys::r,
                                 new db_proxy_logic(),
                                 3,
                                 svc_config::instance()->db_proxy_port(),
                                 svc_config::instance()->db_proxy_host()) != 0)
  {
    e_log->error("init db_proxy failed!");
    return -1;
  }

#if 0
  if (cross_svc::instance()->init(svc_config::instance()->cross_host(),
                                  svc_config::instance()->cross_port()) != 0)
  {
    e_log->error("init cross_svc failed!");
    return -1;
  }
#endif

  s_log->rinfo("launch net module ok!");
  return 0;
}
int sys::load_config()
{
#define CFG_LOAD_CODE(OBJ_NAME) \
  if (OBJ_NAME::instance()->load_config(svc_config::instance()->cfg_root()) != 0) \
  { \
    e_log->error("load " #OBJ_NAME " failed!"); \
    return -1; \
  }

  if (behavior_log::instance()->open(svc_config::instance()->cfg_root()) != 0)
  {
    e_log->error("load behavior log config failed!");
    return -1;
  }

  CFG_LOAD_CODE(global_param_cfg);
  CFG_LOAD_CODE(lvl_param_cfg);
  CFG_LOAD_CODE(basic_attr_cfg);
  CFG_LOAD_CODE(zhan_li_calc_cfg);
  CFG_LOAD_CODE(equip_score_cfg);
  CFG_LOAD_CODE(item_config);
  CFG_LOAD_CODE(scene_config);
  CFG_LOAD_CODE(exit_transfer_cfg);
  CFG_LOAD_CODE(buff_config);
  CFG_LOAD_CODE(skill_config);
  CFG_LOAD_CODE(passive_skill_cfg);
  CFG_LOAD_CODE(passive_skill_upgrade_cfg);
  CFG_LOAD_CODE(monster_cfg);
  CFG_LOAD_CODE(monster_drop_cfg);
  CFG_LOAD_CODE(box_item_cfg);
  CFG_LOAD_CODE(scene_monster_cfg);
  CFG_LOAD_CODE(task_config);
  CFG_LOAD_CODE(task_guide_cfg);
  CFG_LOAD_CODE(mail_config);
  CFG_LOAD_CODE(gift_config);
  CFG_LOAD_CODE(shop_config);
  CFG_LOAD_CODE(title_cfg);
  CFG_LOAD_CODE(wild_boss_config);
  CFG_LOAD_CODE(timing_mst_config);
  CFG_LOAD_CODE(equip_strengthen_cfg);
  CFG_LOAD_CODE(equip_fen_jie_cfg);
  CFG_LOAD_CODE(equip_rong_he_cfg);
  CFG_LOAD_CODE(equip_xi_lian_config);
  CFG_LOAD_CODE(xi_lian_rule_config);
  CFG_LOAD_CODE(xi_lian_value_config);
  CFG_LOAD_CODE(guild_lvl_config);
  CFG_LOAD_CODE(guild_pos_config);
  CFG_LOAD_CODE(guild_skill_config);
  CFG_LOAD_CODE(guild_scp_config);
  CFG_LOAD_CODE(scp_config);
  CFG_LOAD_CODE(tui_tu_config);
  CFG_LOAD_CODE(tui_tu_turn_config);
  CFG_LOAD_CODE(tui_tu_chapter_config);
  CFG_LOAD_CODE(tui_tu_jy_chapter_config);
  CFG_LOAD_CODE(online_award_cfg);
  CFG_LOAD_CODE(login_award_cfg);
  CFG_LOAD_CODE(seven_day_login_cfg);
  CFG_LOAD_CODE(vip_config);
  CFG_LOAD_CODE(vip_purchase_cfg);
  CFG_LOAD_CODE(zhan_xing_cfg);
  CFG_LOAD_CODE(zhan_xing_ten_cfg);
  CFG_LOAD_CODE(cheng_jiu_cfg);
  CFG_LOAD_CODE(bao_shi_lvl_cfg);
  CFG_LOAD_CODE(bao_shi_eff_cfg);
  CFG_LOAD_CODE(huo_yue_du_cfg);
  CFG_LOAD_CODE(jing_ji_award_cfg);
  CFG_LOAD_CODE(jing_ji_first_award_cfg);
  CFG_LOAD_CODE(activity_cfg);
  CFG_LOAD_CODE(ltime_recharge_cfg);
  CFG_LOAD_CODE(name_storage_cfg);
  CFG_LOAD_CODE(recharge_config);
  CFG_LOAD_CODE(lucky_turn_cfg);
  CFG_LOAD_CODE(lucky_turn_rank_award_cfg);
  CFG_LOAD_CODE(product_cfg);
  CFG_LOAD_CODE(water_tree_cfg);
  CFG_LOAD_CODE(tianfu_skill_upgrade_cfg);
  CFG_LOAD_CODE(tianfu_skill_cfg);
  CFG_LOAD_CODE(suit_config);
  CFG_LOAD_CODE(dxc_config);
  CFG_LOAD_CODE(monster_pos_cfg);
  CFG_LOAD_CODE(fa_bao_dj_cfg);
  CFG_LOAD_CODE(fa_bao_he_cheng_cfg);
  CFG_LOAD_CODE(strength_addition_cfg);
  CFG_LOAD_CODE(lucky_goods_turn_cfg);
  CFG_LOAD_CODE(lucky_goods_draw_cfg);
  CFG_LOAD_CODE(baowu_mall_config);
  CFG_LOAD_CODE(gift_byvip_config);
  CFG_LOAD_CODE(lue_duo_config);
  CFG_LOAD_CODE(equip_jin_jie_cfg);
  CFG_LOAD_CODE(kai_fu_act_cfg);

  if (sys::do_check_config() != 0)
    return -1;
  s_log->rinfo("load all game config ok!");
  return 0;
}
int sys::do_check_config()
{
  if (!scene_config::instance()->can_move(global_param_cfg::capital_relive_coord.cid_,
                                          global_param_cfg::capital_relive_coord.coord_.x_,
                                          global_param_cfg::capital_relive_coord.coord_.y_))
  {
    e_log->rinfo("capital_relive_coord %d:%d,%d can not move!",
                 global_param_cfg::capital_relive_coord.cid_,
                 global_param_cfg::capital_relive_coord.coord_.x_,
                 global_param_cfg::capital_relive_coord.coord_.y_);
    return -1;
  }
  return 0;
}
int sys::reload_config(const char *msg,
                       const int /*len*/,
                       char *result,
                       const int res_in_len,
                       int &result_len)
{
  result_len += ::snprintf(result + result_len,
                           res_in_len - result_len,
                           "\"%s\":{", g_svc_name);

  char *tok_p = NULL;
  char *token = NULL;
  char bf[256] = {0};
  ::strncpy(bf, msg, sizeof(bf) - 1);

  int reload_cnt = 0;
  for (token = ::strtok_r(bf, ",", &tok_p);
       token != NULL;
       token = ::strtok_r(NULL, ",", &tok_p))
  {
    if (sys::dispatch_config(token, result, res_in_len, result_len) != -2)
      ++reload_cnt;
  }
  if (reload_cnt > 0)
    result_len -= 2;
  result_len += ::snprintf(result + result_len, res_in_len - result_len, "}");

  return 0;
}
int sys::dispatch_config(const char *name,
                         char *result,
                         const int res_in_len,
                         int &result_len)
{
  int ret = -2;
  bool all = false;
  if (::strncmp(name, "all", 3) == 0) all = true;

#define CFG_RELOAD_CODE(CFG_NAME, OBJ_NAME) \
  const char * OBJ_NAME##_base_name = util::istrchr(CFG_NAME, '/'); \
  if (OBJ_NAME##_base_name == NULL) \
  { \
    OBJ_NAME##_base_name = CFG_NAME; \
  } \
  else \
  { \
    OBJ_NAME##_base_name = OBJ_NAME##_base_name + 1; \
  } \
  if (all || ::strcmp(OBJ_NAME##_base_name, name) == 0) \
  { \
    ret = OBJ_NAME::instance()->reload_config(svc_config::instance()->cfg_root()); \
    result_len += ::snprintf(result + result_len, \
                             res_in_len - result_len, \
                             "\"%s\": %d, ", CFG_NAME, ret); \
  }

  CFG_RELOAD_CODE(BEHAVIOR_LOG_CFG_PATH, behavior_log);
  CFG_RELOAD_CODE(GLOBAL_PARAM_CFG_PATH, global_param_cfg);
  CFG_RELOAD_CODE(LVL_PARAM_CFG_PATH, lvl_param_cfg);
  CFG_RELOAD_CODE(BASIC_ATTR_CFG_PATH, basic_attr_cfg);
  CFG_RELOAD_CODE(ZHAN_LI_CALC_CFG_PATH, zhan_li_calc_cfg);
  CFG_RELOAD_CODE(EQUIP_SCORE_CFG_PATH, equip_score_cfg);
  CFG_RELOAD_CODE(ITEM_CFG_PATH, item_config);
  CFG_RELOAD_CODE(SCENE_CFG_PATH, scene_config);
  CFG_RELOAD_CODE(SCENE_TRANSFER_CFG_PATH, exit_transfer_cfg);
  CFG_RELOAD_CODE(BUFF_CFG_PATH, buff_config);
  CFG_RELOAD_CODE(SKILL_CFG_PATH, skill_config);
  CFG_RELOAD_CODE(PASSIVE_SKILL_CFG_PATH, passive_skill_cfg);
  CFG_RELOAD_CODE(PASSIVE_SKILL_UPGRADE_CFG_PATH, passive_skill_upgrade_cfg);
  CFG_RELOAD_CODE(MONSTER_CFG_PATH, monster_cfg);
  CFG_RELOAD_CODE(MONSTER_DROP_CFG_PATH, monster_drop_cfg);
  CFG_RELOAD_CODE(DROP_BOX_CFG_PATH, box_item_cfg);
  CFG_RELOAD_CODE(SCENE_MONSTER_CFG_PATH, scene_monster_cfg);
  CFG_RELOAD_CODE(TASK_CFG_PATH, task_config);
  CFG_RELOAD_CODE(TASK_GUIDE_CFG_PATH, task_guide_cfg);
  CFG_RELOAD_CODE(MAIL_CFG_PATH, mail_config);
  CFG_RELOAD_CODE(GIFT_CFG_PATH, gift_config);
  CFG_RELOAD_CODE(SHOP_CONFIG_PATH, shop_config);
  CFG_RELOAD_CODE(TITLE_CFG_PATH, title_cfg);
  CFG_RELOAD_CODE(WILD_BOSS_CFG_PATH, wild_boss_config);
  CFG_RELOAD_CODE(TIMING_MST_CONFIG_PATH, timing_mst_config);
  CFG_RELOAD_CODE(EQUIP_STRENGTHEN_CFG_PATH, equip_strengthen_cfg);
  CFG_RELOAD_CODE(EQUIP_FEN_JIE_CFG_PATH, equip_fen_jie_cfg);
  CFG_RELOAD_CODE(EQUIP_RONG_HE_CFG_PATH, equip_rong_he_cfg);
  CFG_RELOAD_CODE(EQUIP_XI_LIAN_CFG_PATH, equip_xi_lian_config);
  CFG_RELOAD_CODE(XI_LIAN_RULE_CFG_PATH, xi_lian_rule_config);
  CFG_RELOAD_CODE(XI_LIAN_VALUE_CFG_PATH, xi_lian_value_config);
  CFG_RELOAD_CODE(GUILD_LVL_CFG_PATH, guild_lvl_config);
  CFG_RELOAD_CODE(GUILD_POS_CFG_PATH, guild_pos_config);
  CFG_RELOAD_CODE(GUILD_SKILL_CFG_PATH, guild_skill_config);
  CFG_RELOAD_CODE(GUILD_SCP_CFG_PATH, guild_scp_config);
  CFG_RELOAD_CODE(SCP_CFG_PATH, scp_config);
  CFG_RELOAD_CODE(TUI_TU_CONFIG_PATH, tui_tu_config);
  CFG_RELOAD_CODE(TUI_TU_TURN_CONFIG_PATH, tui_tu_turn_config);
  CFG_RELOAD_CODE(TUI_TU_CHAPTER_CONFIG_PATH, tui_tu_chapter_config);
  CFG_RELOAD_CODE(TUI_TU_JY_CHAPTER_CONFIG_PATH, tui_tu_jy_chapter_config);
  CFG_RELOAD_CODE(ONLINE_AWARD_CFG_PATH, online_award_cfg);
  CFG_RELOAD_CODE(LOGIN_AWARD_CFG_PATH, login_award_cfg);
  CFG_RELOAD_CODE(SEVEN_DAY_LOGIN_CFG_PATH, seven_day_login_cfg);
  CFG_RELOAD_CODE(VIP_CFG_PATH, vip_config);
  CFG_RELOAD_CODE(VIP_PURCHASE_CFG_PATH, vip_purchase_cfg);
  CFG_RELOAD_CODE(ZHAN_XING_CFG_PATH, zhan_xing_cfg);
  CFG_RELOAD_CODE(ZHAN_XING_TEN_CFG_PATH, zhan_xing_ten_cfg);
  CFG_RELOAD_CODE(CHENG_JIU_CFG_PATH, cheng_jiu_cfg);
  CFG_RELOAD_CODE(BAO_SHI_LVL_CFG_PATH, bao_shi_lvl_cfg);
  CFG_RELOAD_CODE(BAO_SHI_EFF_CFG_PATH, bao_shi_eff_cfg);
  CFG_RELOAD_CODE(HUO_YUE_DU_CFG_PATH, huo_yue_du_cfg);
  CFG_RELOAD_CODE(JING_JI_AWARD_CFG_PATH, jing_ji_award_cfg);
  CFG_RELOAD_CODE(JING_JI_FIRST_AWARD_CFG_PATH, jing_ji_first_award_cfg);
  CFG_RELOAD_CODE(ACTIVITY_CFG_PATH, activity_cfg);
  CFG_RELOAD_CODE(LTIME_RECHARGE_CFG_PATH, ltime_recharge_cfg);
  CFG_RELOAD_CODE(NAME_STORAGE_CFG_PATH, name_storage_cfg);
  CFG_RELOAD_CODE(RECHARGE_CFG_PATH, recharge_config);
  CFG_RELOAD_CODE(LUCKY_TURN_CFG_PATH, lucky_turn_cfg);
  CFG_RELOAD_CODE(LUCKY_TURN_RANK_AWARD_CFG_PATH, lucky_turn_rank_award_cfg);
  CFG_RELOAD_CODE(PRODUCT_CFG_PATH, product_cfg);
  CFG_RELOAD_CODE(WATER_TREE_CFG_PATH, water_tree_cfg);
  CFG_RELOAD_CODE(TIANFU_SKILL_UPGRADE_CFG_PATH, tianfu_skill_upgrade_cfg);
  CFG_RELOAD_CODE(TIANFU_SKILL_CFG_PATH, tianfu_skill_cfg);
  CFG_RELOAD_CODE(SUIT_CFG_PATH, suit_config);
  CFG_RELOAD_CODE(DXC_CONFIG_PATH, dxc_config);
  CFG_RELOAD_CODE(MONSTAR_POS_CFG_PATH, monster_pos_cfg);
  CFG_RELOAD_CODE(FA_BAO_DENG_JIE_CFG_PATH, fa_bao_dj_cfg);
  CFG_RELOAD_CODE(FA_BAO_HE_CHENG_CFG_PATH, fa_bao_he_cheng_cfg);
  CFG_RELOAD_CODE(STRENGTH_ADDITION_CFG_PATH, strength_addition_cfg);
  CFG_RELOAD_CODE(LUCKY_GOODS_TURN_CFG_PATH, lucky_goods_turn_cfg);
  CFG_RELOAD_CODE(LUCKY_GOODS_DRAW_CFG_PATH, lucky_goods_draw_cfg);
  CFG_RELOAD_CODE(BAOWU_MALL_CONFIG_PATH, baowu_mall_config);
  CFG_RELOAD_CODE(GIFT_BYVIP_CFG_PATH, gift_byvip_config);
  CFG_RELOAD_CODE(LUE_DUO_CONFIG_PATH, lue_duo_config);
  CFG_RELOAD_CODE(EQUIP_JIN_JIE_CFG_PATH, equip_jin_jie_cfg);
  CFG_RELOAD_CODE(KAI_FU_ACT_CFG_PATH, kai_fu_act_cfg);
  return ret;
}
