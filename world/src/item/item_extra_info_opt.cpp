#include "item_extra_info_opt.h"
#include "item_obj.h"
#include "def.h"
#include "global_macros.h"
#include "sys_log.h"

// Lib header
#include <json/json.h>
#include "mblock.h"

#define MAX_XL_NAME_LEN           16
static ilog_obj *e_log = err_log::instance()->get_ilog("item");

static const char *s_xl_effect[ATTR_T_ITEM_CNT] = 
{
  "",
  "hp",
  "fl",
  "gj",
  "fy",
  "mz",
  "sb",
  "bj",
  "kb",
  "sm"
};

int match_xl_id(const char *name)
{
  for (int i = 0; i < ATTR_T_ITEM_CNT; ++i)
  {
    if (::strcmp(s_xl_effect[i], name) == 0)
      return i;
  }
  return -1;
}

void item_extra_info_opt::encode_extra_info(item_obj *item, Json::Value &root)
{
  Json::FastWriter j_writer;

  static std::string new_str;
  new_str = j_writer.write(root);

  if (item->extra_info_ == NULL
      || (int)new_str.length() >= item->extra_info_->size())
  {
    if (item->extra_info_ != NULL)
      mblock_pool::instance()->release(item->extra_info_);
    item->extra_info_ = mblock_pool::instance()->alloc(new_str.length() + 1);
  }

  item->extra_info_->reset();
  item->extra_info_->copy(new_str.c_str(), new_str.length());
  item->extra_info_->set_eof();
}
void item_extra_info_opt::decode_extra_info(item_obj *item, Json::Value &root)
{
  if (item->extra_info_ == NULL
      || item->extra_info_->length() < (int)sizeof("{'x':1}")) return ;
  Json::Reader j_reader;
  if (!j_reader.parse(item->extra_info_->rd_ptr(),
                      item->extra_info_->rd_ptr() + item->extra_info_->length(),
                      root,
                      false))
    e_log->rinfo("load iu json[%s] failed!", item->extra_info_->rd_ptr());
}
int item_extra_info_opt::get_int(const char *k, Json::Value &jv)
{
  if (!jv.isMember(k)) return -1;
  return jv[k].asInt();
}
void item_extra_info_opt::set_value(item_obj *item,
                                    const char *k,
                                    const int k_len,
                                    const int v,
                                    const int v_len,
                                    Json::Value &root)
{
  if (item->extra_info_ == NULL)
  {
    item->extra_info_ = mblock_pool::instance()->alloc(sizeof("{'':}")
                                                       + k_len
                                                       + v_len
                                                       + 1);
    char bf[64] = {0};
    int len = ::snprintf(bf, sizeof(bf), "{\"%s\":%d}", k, v);
    item->extra_info_->copy(bf, len);
    item->extra_info_->set_eof();
  }else
  {
    if (root.empty())
      item_extra_info_opt::decode_extra_info(item, root);
    root[k] = v;
    item_extra_info_opt::encode_extra_info(item, root);
  }
}
void item_extra_info_opt::get_xl_attr(const char *k,
                                      int attr[][4],
                                      Json::Value &root)
{
  if (!root.isMember(k)) return ;
  Json::Value &xl_attr = root[k];
  int size = xl_attr.size();
  for (int i = 0; i < size; ++i)
  {
    Json::Value &t_v = xl_attr[i];
    if (!t_v.empty())
    {
      Json::Value::iterator itor = t_v.begin();
      attr[i][0] = match_xl_id(itor.key().asCString());
      attr[i][1] = (*itor)[0u].asInt();
      attr[i][2] = (*itor)[1u].asInt();
      attr[i][3] = (*itor)[2u].asInt();
    }
  }
}
void item_extra_info_opt::set_xl_attr(item_obj *item,
                                      const char *k,
                                      int attr[][4],
                                      Json::Value &root)
{
  if (root.empty())
    item_extra_info_opt::decode_extra_info(item, root);
  Json::Value xl_attr_l(Json::arrayValue);
  for (int i = 0; i < MAX_XI_LIAN_ATTR_NUM; ++i)
  {
    Json::Value xl_attr(Json::objectValue);
    if (attr[i][0] < 0
        || attr[i][0] >= ATTR_T_ITEM_CNT
        || attr[i][1] == 0)
      break;
    const char *name = s_xl_effect[attr[i][0]];
    xl_attr[name].append(attr[i][1]);
    xl_attr[name].append(attr[i][2]);
    xl_attr[name].append(attr[i][3]);

    xl_attr_l.append(xl_attr);
  }
  if (xl_attr_l.empty()) return ;
  root[k] = xl_attr_l;
  item_extra_info_opt::encode_extra_info(item, root);
}
int item_extra_info_opt::del_xl_attr(item_obj *item,
                                     const char *k,
                                     Json::Value &xl,
                                     Json::Value &root)
{
  if (!root.isMember(k)) return -1;
  xl = root.removeMember(k);

  item_extra_info_opt::encode_extra_info(item, root);
  return 0;
}
int item_extra_info_opt::replace_cur_by_new_xl(item_obj *equip_obj)
{
  Json::Value extra_json;
  item_extra_info_opt::decode_extra_info(equip_obj, extra_json);

  if (!extra_json.isMember("new_xl")) return -1;
  Json::Value new_xl = extra_json.removeMember("new_xl");

  extra_json["cur_xl"] = new_xl;
  item_extra_info_opt::encode_extra_info(equip_obj, extra_json);
  return 0;
}
int item_extra_info_opt::copy_xl_info(item_obj *dest, Json::Value &src)
{
  Json::Value dest_extra_json;
  item_extra_info_opt::decode_extra_info(dest, dest_extra_json);

  if (!src.isMember("cur_xl")) return -1;
  dest_extra_json["cur_xl"] = src["cur_xl"];

  item_extra_info_opt::encode_extra_info(dest, dest_extra_json);
  return 0;
}
