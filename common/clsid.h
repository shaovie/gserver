// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-03-19 18:49
 */
//========================================================================

#ifndef CLSID_H_
#define CLSID_H_

#include "def.h"
#include "global_macros.h"

// Forward declarations

/**
 * @class cid
 *
 * @brief
 */
class clsid
{
public:
  inline static bool is_scene(const int cid)
  { return cid >= 1000 && cid < MAX_SCENE_CID; }
  inline static bool is_world_scene(const int cid)
  { return cid >= 1000 && cid < 2000;}
  inline static bool is_activity_scene(const int cid)
  { return cid >= 4000 && cid < 5000;}
  inline static bool is_tui_tu_scp_scene(const int cid)
  { return cid >= 2000 && cid < 3000; }
  inline static bool is_scp_scene(const int cid)
  { return cid >= 3000 && cid < 4000; }
  inline static bool is_common_scp_scene(const int cid)
  { return cid >= 3100 && cid < 3200; }
  inline static bool is_skill(const int cid)
  { return cid > 30000000 && cid < 40000000; }
  inline static bool is_char_skill(const int cid)
  { return cid > 31000000 && cid < 32000000; }
  inline static bool is_monster(const int cid)
  { return cid > 50000000 && cid < 60000000; }
  inline static bool is_item(const int cid)
  { return cid > 20000000 && cid < 30000000; }
  inline static bool is_equip(const int cid)
  { return cid > 10000000 && cid < 20000000; }
  inline static bool is_char_equip(const int cid)
  { return cid > 11000000 && cid < 12000000; }
  inline static int get_equip_cid_by_career(const char career, const int equip_cid)
  {
    if (!clsid::is_char_equip(equip_cid)) return equip_cid;
    return equip_cid / 1000000 * 1000000 + career * 100000 + equip_cid % 100000;
  }
  inline static bool is_fa_bao(const int cid)
  { return cid > 23000000 && cid < 24000000; }
  inline static bool is_char_zhu_dong_skill(const int cid)
  { return clsid::is_char_skill(cid) && (cid / 10000 % 10 == 1); }
  inline static bool is_char_bei_dong_skill(const int cid)
  { return clsid::is_char_skill(cid) && (cid / 10000 % 10 == 2); }
  inline static bool is_guild_skill(const int clsid)
  { return clsid / 1000000 == 33; }

};

#endif // CLSID_H_

