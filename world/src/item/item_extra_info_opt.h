// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-02-27 11:40
 */
//========================================================================

#ifndef ITEM_EXTRA_INFO_OPT_H_
#define ITEM_EXTRA_INFO_OPT_H_

// Forward declarations
namespace Json { class Value; }
class item_obj;

/* {
 *   "qh_lvl":10,           强化等级
 *   "qh_cnt":10,           强化次数
 *   "qh_bless":10,         强化祝福值
 *   "xl_cnt":10,           洗练次数
 *   "cur_xl":[             当前洗练
 *    {"hp":[2,500,0]},     "属性"：[星值，属性值，是否加锁]
 *    {"hp":[3,500,0]},
 *    ......
 *   ], 
 *   "new_xl":[             可用做替换的新的洗练
 *    {"hp":[2,500,0]},     "属性"：[星值，属性值，是否加锁]
 *    {"hp":[3,500,0]},
 *    ......
 *   ],
 * }
 */
/**
 * @class item_extra_info_opt
 * 
 * @brief
 */
class item_extra_info_opt
{
public:
  static void encode_extra_info(item_obj *item, Json::Value &root);
  static void decode_extra_info(item_obj *item, Json::Value &root);

  static int get_qh_lvl(Json::Value &jv)
  { return item_extra_info_opt::get_int("qh_lvl", jv); }
  static void set_qh_lvl(item_obj *item, const int qh_lvl, Json::Value &jv)
  { item_extra_info_opt::set_value(item, "qh_lvl", sizeof("qh_lvl") - 1, qh_lvl, 4, jv); }
  static int get_qh_cnt(Json::Value &jv)
  { return item_extra_info_opt::get_int("qh_cnt", jv); }
  static void set_qh_cnt(item_obj *item, const int qh_cnt, Json::Value &jv)
  { item_extra_info_opt::set_value(item, "qh_cnt", sizeof("qh_cnt") - 1, qh_cnt, 4, jv); }
  static int get_qh_bless(Json::Value &jv)
  { return item_extra_info_opt::get_int("qh_bless", jv); }
  static void set_qh_bless(item_obj *item, const int qh_bless, Json::Value &jv)
  { item_extra_info_opt::set_value(item, "qh_bless", sizeof("qh_bless") - 1, qh_bless, 4, jv); }
  static int get_xl_cnt(Json::Value &jv)
  { return item_extra_info_opt::get_int("xl_cnt", jv); }
  static void set_xl_cnt(item_obj *item, const int xl_cnt, Json::Value &jv)
  { item_extra_info_opt::set_value(item, "xl_cnt", sizeof("xl_cnt") - 1, xl_cnt, 4, jv); }
  static void get_cur_xl(int attr[][4], Json::Value &jv)
  { item_extra_info_opt::get_xl_attr("cur_xl", attr, jv); }
  static void set_cur_xl(item_obj *item, int attr[][4], Json::Value &jv)
  { item_extra_info_opt::set_xl_attr(item, "cur_xl", attr, jv); }
  static void set_new_xl(item_obj *item, int attr[][4], Json::Value &jv)
  { item_extra_info_opt::set_xl_attr(item, "new_xl", attr, jv); }
  static int del_new_xl(item_obj *item, Json::Value &new_xl, Json::Value &jv)
  { return item_extra_info_opt::del_xl_attr(item, "new_xl", new_xl, jv); }
  static int replace_cur_by_new_xl(item_obj *item);
  static int copy_xl_info(item_obj *dest, Json::Value &src);
private:
  static int get_int(const char *k, Json::Value &jv);
  static void set_value(item_obj *item,
                        const char *k,
                        const int k_len,
                        const int v,
                        const int v_len,
                        Json::Value &root);
  static void get_xl_attr(const char *k,
                          int attr[][4],
                          Json::Value &root);
  static void set_xl_attr(item_obj *item,
                          const char *k,
                          int attr[][4],
                          Json::Value &root);
  static int del_xl_attr(item_obj *item,
                         const char *k,
                         Json::Value &new_xl,
                         Json::Value &root);
};

#endif // ITEM_EXTRA_INFO_OPT_H_

