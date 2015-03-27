#include "spawn_monster.h"
#include "scene_config.h"
#include "monster_template.h"
#include "monster_obj.h"
#include "monster_cfg.h"
#include "scene_monster_cfg.h"
#include "sys_log.h"
#include "clsid.h"
#include "zhuzai_fen_shen_mst.h"
#include "ghz_wang_zuo_mst.h"
#include "ghz_shou_wei_mst.h"
#include "global_param_cfg.h"
#include "guild_shen_shou_mst.h"
#include "guild_scp_mst.h"
#include "tu_teng_mst.h"
#include "group_monster.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("mst");
static ilog_obj *e_log = err_log::instance()->get_ilog("mst");

ilist<pair_t<int> > spawn_monster::spawn_mst_list;

int spawn_monster::spawn_all_scene_monster()
{
  ilist<int> &slist = scene_config::instance()->scene_list();
  ilist_node<int> *itor = slist.head();
  for (; itor != NULL; itor = itor->next_)
  {
    int scene_cid = itor->value_;
    if (clsid::is_tui_tu_scp_scene(scene_cid)
        || clsid::is_scp_scene(scene_cid)
        || global_param_cfg::ghz_scene_cid == scene_cid)
      continue;
    spawn_monster::spawn_scene_monster(scene_cid,
                                       scene_cid,
                                       0,
                                       NULL,
                                       NULL);
  }

  return 0;
}
int spawn_monster::spawn_scene_monster(const int scene_id,
                                       const int scene_cid,
                                       const int idx,
                                       ilist<pair_t<int> > *mst_list,
                                       ilist<int> *exclude_mst_list)
{
  if (!scene_monster_cfg::instance()->have_monster(scene_cid))
    return 0;
  short x_len = scene_config::instance()->x_len(scene_cid);
  short y_len = scene_config::instance()->y_len(scene_cid);
  for (short y = 0; y < y_len; ++y)
  {
    for (short x = 0; x < x_len; ++x)
    {
      char cv = scene_config::instance()->coord_value(scene_cid, x, y);
      if (cv == -1)
      {
        e_log->wning("get scene %d coord value = -1 where spawan monster!",
                     scene_cid);
        continue;
      }
      if (cv < 10 || (idx != 0 && idx != cv)) continue;
      int mst_cid = 0;
      int dir = DIR_XX;
      if (scene_monster_cfg::instance()->get_monster_info(scene_cid,
                                                          cv,
                                                          mst_cid,
                                                          dir) == -1)
        continue;
      if (exclude_mst_list != NULL
          && exclude_mst_list->find(mst_cid))
        continue;

      int mst_id = spawn_monster::spawn_one(mst_cid,
                                            0,
                                            scene_id,
                                            scene_cid,
                                            dir,
                                            x,
                                            y);
      if (mst_list != NULL && mst_id > 0)
        mst_list->push_back(pair_t<int>(mst_id, mst_cid));
    }
  }
  return 0;
}
int spawn_monster::spawn_one(const int mst_cid,
                             const int delay,
                             const int scene_id,
                             const int scene_cid,
                             const char dir,
                             const short x,
                             const short y)
{
  monster_obj *mst = NULL;
  const monster_cfg_obj *mco = monster_cfg::instance()->get_monster_cfg_obj(mst_cid);
  if (mco == NULL)
  {
    e_log->wning("not found monster %d in monster cfg when spawn monster in %d",
                 mst_cid, scene_cid);
    return -1;
  }

  if (mco->mst_type_ == MST_COMMON)
    mst = new fighting_monster_obj();
  else if (mco->mst_type_ == MST_ZHU_ZAI_JING_XIANG)
    mst = new zhuzai_fen_shen_mst();
  else if (mco->mst_type_ == MST_GHZ_WANG_ZUO)
    mst = new ghz_wang_zuo_mst();
  else if (mco->mst_type_ == MST_GHZ_SHOU_WEI)
    mst = new ghz_shou_wei_mst();
  else if (mco->mst_type_ == MST_GUILD_SHEN_SHOU)
    mst = new guild_shen_shou_mst();
  else if (mco->mst_type_ == MST_GUILD_SCP_NO_HATE_MST)
    mst = new guild_scp_no_hate_mst();
  else if (mco->mst_type_ == MST_GUILD_SCP_HAD_HATE_MST)
    mst = new guild_scp_had_hate_mst();
  else if (mco->mst_type_ == MST_TU_TENG_TO_PLAYER)
    mst = new player_tu_teng_mst();
  else if (mco->mst_type_ == MST_XSZC_MAIN)
    mst = new xszc_main_mst(mco->param_);
  else if (mco->mst_type_ == MST_XSZC_BARRACK)
    mst = new xszc_barrack_mst(mco->param_);
  else if (mco->mst_type_ == MST_XSZC_DEFENDER)
    mst = new xszc_defender_mst(mco->param_);
  else if (mco->mst_type_ == MST_XSZC_ARM)
    mst = new xszc_arm_mst(mco->param_);

  if (mst == NULL)
  {
    e_log->error("unsupportable monster type %d!", mco->mst_type_);
    return -1;
  }

  if (mst->init(mst_cid, scene_id, scene_cid, dir, x, y) != 0
      || mst->do_activate(delay) != 0)
  {
    e_log->error("spawn monster %d failed!", mst_cid);
    delete mst;
    return -1;
  }
  return mst->id();
}
