// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-12-10 11:54
 */
//========================================================================

#ifndef OBJ_ATTR_H_
#define OBJ_ATTR_H_

#include "def.h"

// Lib header
#include <cstring>

// Forward declarations
class char_obj;

enum calc_attr_opt_t
{
  CALC_ALL_ATTR       = 1,
  CALC_ZHAN_LI        = 2
};
enum add_t
{
  ADD_T_BASIC = 0,
  ADD_T_EQUIP,
  ADD_T_BAO_SHI,
  ADD_T_BUFF,
  ADD_T_PASSIVE_SKILL,
  ADD_T_GUILD_SKILL,
  ADD_T_TITLE,
  ADD_T_FA_BAO,
  ADD_T_ITEM_CNT
};
class base_attr
{
public:
  base_attr() { }

  void init() { ::memset(this->attr_value_, 0, sizeof(this->attr_value_)); }
  int total(const int t);
  void clear(const add_t t) { this->attr_value_[t] = 0; }
  void add(const add_t t, const int val) { this->attr_value_[t] += val; }
protected:
  int total_all();
  int total_zhan_li();

  int attr_value_[ADD_T_ITEM_CNT];
};
class value_attr : public base_attr { public: value_attr() { } };
class rate_attr : public base_attr
{
  typedef base_attr super;
public:
  rate_attr() { }
  int total(const int t) { return super::total(t) + 10000; }
};
/**
 * @class obj_attr
 *
 * @brief
 */
class obj_attr
{
public:
  obj_attr();

  static void reset_attr_v_add()
  { ::memset(obj_attr::attr_v_add, 0, sizeof(obj_attr::attr_v_add)); }
  static void reset_attr_r_add()
  { ::memset(obj_attr::attr_r_add, 0, sizeof(obj_attr::attr_r_add)); }

  void clear_value(const add_t t);
  void clear_rate(const add_t t);

  // 累加固定值的属性
  void acc_v_attr(const int   attr_add[ATTR_T_ITEM_CNT], const add_t t);
  // 累加百分比的属性
  void acc_r_attr(const int attr_add[ATTR_T_ITEM_CNT], const add_t t);

  // attribute
  int hp()               const { return this->hp_;                             }
  int mp()               const { return this->mp_;                             }
  int total_hp()         const { return this->total_value_[ATTR_T_HP];         }
  int total_mp()         const { return this->total_value_[ATTR_T_MP];         }
  int gong_ji()          const { return this->total_value_[ATTR_T_GONG_JI];    }
  int fang_yu()          const { return this->total_value_[ATTR_T_FANG_YU];    }
  int ming_zhong()       const { return this->total_value_[ATTR_T_MING_ZHONG]; }
  int shan_bi()          const { return this->total_value_[ATTR_T_SHAN_BI];    }
  int bao_ji()           const { return this->total_value_[ATTR_T_BAO_JI];     }
  int kang_bao()         const { return this->total_value_[ATTR_T_KANG_BAO];   }
  int shang_mian()       const { return this->total_value_[ATTR_T_SHANG_MIAN]; }

  void calc_zhan_li(int ret[ATTR_T_ITEM_CNT])
  { return this->calc_attr(ret, CALC_ZHAN_LI); }
private:
  void calc_attr(int *ret, const int ct);
  void adjust_hp_mp(const int old_hp,
                    const int old_total_hp,
                    const int old_mp,
                    const int old_total_mp);
public:
  static int   attr_r_add[ATTR_T_ITEM_CNT];
  static int   attr_v_add[ATTR_T_ITEM_CNT];

  int   hp_;   // 当前生命
  int   mp_;   // 当前法力

private:
  rate_attr  acc_add_r_[ATTR_T_ITEM_CNT];   // 累积的百分比加成
  value_attr acc_add_v_[ATTR_T_ITEM_CNT];   // 累积的数值加成
  int total_value_[ATTR_T_ITEM_CNT];        // 总的结果值
};
#include "obj_attr.inl"
#endif // OBJ_ATTR_H_

