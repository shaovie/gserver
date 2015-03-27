// -*- C++ -*-

//========================================================================
/**
 * Author   : alvinhu
 * Date     : 2012-11-12 10:02
 */
//========================================================================

#ifndef MAIL_INFO_H_
#define MAIL_INFO_H_

// Lib header
#include <stdio.h>
#include <string.h>

#include "istream.h"
#include "ilist.h"
#include "global_macros.h"

// Forward declarations

#define ALL_ITEMS_LEN ((MAX_ITEM_EXTRA_INFO_LEN + 64) * MAX_ATTACH_NUM)

const char db_tb_mail_info[] = "mail";

/**
 * @class mail_header
 *
 * @brief
 */
class mail_header
{
public:
  mail_header() { reset(); }
  void reset()
  {
    this->id_           = 0;
    this->readed_       = 0;
    this->has_attach_   = 0;
    this->mail_type_    = 0;
    this->send_time_    = 0;
    ::memset(this->sender_name_, 0, sizeof(this->sender_name_));
    ::memset(this->title_, 0, sizeof(this->title_));
  }
  char readed_;                             // 是否已读
  char mail_type_;                          // 邮件类型
  char has_attach_;                         // 是否有附件  NO STORE DB
  int  id_;                                 //
  int  send_time_;                          // 发送时间
  char sender_name_[MAX_NAME_LEN + 1];      // 发件人名字
  char title_[MAX_MAIL_TITLE_LEN + 1];      // 邮件主题
};
inline out_stream & operator << (out_stream &os, const mail_header *info)
{
  stream_ostr name((const char *)info->sender_name_, ::strlen(info->sender_name_));
  stream_ostr title((const char *)info->title_, ::strlen(info->title_));
  os << info->readed_
    << info->mail_type_
    << info->has_attach_
    << info->id_
    << info->send_time_
    << name
    << title;
  return os;
}
inline in_stream & operator >> (in_stream &is, mail_header *info)
{
  stream_istr name(info->sender_name_, sizeof(info->sender_name_));
  stream_istr title(info->title_, sizeof(info->title_));
  is >> info->readed_
    >> info->mail_type_
    >> info->has_attach_
    >> info->id_
    >> info->send_time_
    >> name
    >> title;
  return is;
}
class mail_detail
{
public:
  mail_detail() { reset(); }
  void reset()
  {
    this->id_           = 0;
    this->char_id_      = 0;
    this->attach_num_   = 0;
    this->coin_         = 0;
    this->diamond_      = 0;
    this->b_diamond_    = 0;
    ::memset(this->content_, 0, sizeof(this->content_));
    ::memset(this->items_, 0, sizeof(this->items_));
  }
  char attach_num_;                         // 附件个数
  int  id_;
  int  char_id_;                            // 收件人id
  int  coin_;                               //
  int  diamond_;                            //
  int  b_diamond_;                          //
  char content_[MAX_MAIL_CONTENT_LEN + 1];  // 邮件内容
  char items_[ALL_ITEMS_LEN + 1];           // 附件中物品信息
};
inline out_stream & operator << (out_stream &os, const mail_detail *info)
{
  stream_ostr content((const char *)info->content_, ::strlen(info->content_));
  stream_ostr items((const char *)info->items_, ::strlen(info->items_));
  os << info->attach_num_
    << info->id_
    << info->char_id_
    << info->coin_
    << info->diamond_
    << info->b_diamond_
    << content
    << items;
  return os;
}
inline in_stream & operator >> (in_stream &is, mail_detail *info)
{
  stream_istr content(info->content_, sizeof(info->content_));
  stream_istr items(info->items_, sizeof(info->items_));
  is >> info->attach_num_
    >> info->id_
    >> info->char_id_
    >> info->coin_
    >> info->diamond_
    >> info->b_diamond_
    >> content
    >> items;
  return is;
}
/**
 * @class mail_info
 *
 * @brief
 */
class mail_info // send_id, mail_type, mail_idx
{
public:
  enum // send_id
  {
    MAIL_SEND_SYSTEM_ID             = 0,
  };
  enum // mail type
  {
    MAIL_TYPE_PRIVATE               = 0,
    MAIL_TYPE_GM                    = 1,
    MAIL_TYPE_MARKET                = 2,
    MAIL_TYPE_PACKAGE               = 3,
    MAIL_TYPE_CACHE                 = 4, // 全服发
    MAIL_TYPE_JING_JI               = 5,
    MAIL_TYPE_GUILD                 = 6,
    MAIL_TYPE_LUCKY_EGG             = 7,
    MAIL_TYPE_XSZC                  = 8,
  };
  enum   // For column index.
  {
    IDX_ID = 0, IDX_CHAR_ID, IDX_SENDER_ID,
    IDX_SENDER_NAME, IDX_TITLE, IDX_SEND_TIME,
    IDX_READED, IDX_ATTACH_NUM, IDX_MAIL_TYPE,
    IDX_COIN, IDX_DIAMOND, IDX_B_DIAMOND, IDX_CONTENT,
    IDX_ITEMS, IDX_DELETE_TIME, IDX_LVL_LIMIT,
    IDX_END
  };
  mail_info() { this->reset(); }

  mail_info(const int char_id, const int sender_id,
            const char *sender_name, const char *title, const char *content,
            const int send_time, const int readed,
            const int attach_num, const int mail_type,
            const int coin, const int diamond, const int b_diamond,
            const int delete_time, const char *item, const int lvl_limit = 0)
  {
    ::memset(this->mail_header_.sender_name_, 0, sizeof(this->mail_header_.sender_name_));
    ::memset(this->mail_header_.title_, 0, sizeof(this->mail_header_.title_));
    ::memset(this->mail_detail_.content_, 0, sizeof(this->mail_detail_.content_));
    ::memset(this->mail_detail_.items_, 0, sizeof(this->mail_detail_.items_));

    this->mail_header_.readed_       = readed;
    this->mail_header_.mail_type_    = mail_type;
    this->mail_detail_.attach_num_   = attach_num;
    this->char_id_                   = char_id;
    this->sender_id_                 = sender_id;
    this->mail_header_.send_time_    = send_time;
    this->mail_detail_.coin_         = coin;
    this->mail_detail_.diamond_      = diamond;
    this->mail_detail_.b_diamond_    = b_diamond;
    this->delete_time_               = delete_time;
    this->lvl_limit_                 = lvl_limit;
    ::strncpy(this->mail_header_.sender_name_, sender_name, sizeof(this->mail_header_.sender_name_) - 1);
    ::strncpy(this->mail_header_.title_, title, sizeof(this->mail_header_.title_) - 1);
    ::strncpy(this->mail_detail_.content_, content, sizeof(this->mail_detail_.content_) - 1);
    ::strncpy(this->mail_detail_.items_, item, sizeof(this->mail_detail_.items_) - 1);
  }
public: // sql
  static const char *all_col()
  {
    return "char_id,sender_id,sender_name,"
      "title,send_time,readed,attach_num,"
      "mail_type,coin,diamond,b_diamond,content,"
      "items,delete_time,lvl_limit";
  }
  int select_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select id,%s "
                      "from %s where id=%d "
                      "and delete_time=0",
                      mail_info::all_col(),
                      db_tb_mail_info, this->id_);
  }
  int get_mail_attach_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select id,"
                      "char_id,attach_num,"
                      "coin,diamond,b_diamond,items,sender_id "
                      "from %s where id=%d "
                      "and delete_time=0",
                      db_tb_mail_info, this->id_);
  }
  int get_mail_list_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select id,"
                      "sender_name,title,"
                      "send_time,readed,attach_num,"
                      "mail_type,coin,diamond,b_diamond "
                      "from %s "
                      "where char_id=%d and delete_time=0 order by send_time desc "
                      "limit %d",
                      db_tb_mail_info,
                      this->char_id_,
                      MAX_MAIL_COUNT);
  }
  int get_system_mail_list_sql(char *bf,
                               const int bf_len,
                               const int limit_time,
                               const int once_get_system_cnt,
                               const short level)
  {
    return ::snprintf(bf, bf_len,
                      "select id,%s "
                      "from %s "
                      "where mail_type=%d and send_time>%d "
                      "and delete_time=0 "
                      "and lvl_limit<=%d limit %d",
                      mail_info::all_col(),
                      db_tb_mail_info,
                      mail_info::MAIL_TYPE_CACHE, limit_time,
                      level, once_get_system_cnt);
  }
  int check_new_mail_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "select count(readed) "
                      "from %s "
                      "where char_id=%d "
                      "and readed=0 and delete_time=0",
                      db_tb_mail_info,
                      this->char_id_);
  }
  int insert_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "insert into %s(%s)"
                      "values("
                      "%d,"
                      "%d,'%s',"
                      "'%s',%d,"
                      "%d,%d,"
                      "%d,%d,"
                      "%d,%d,'%s',"
                      "'%s',%d,"
                      "%d"
                      ")",
                      db_tb_mail_info,
                      mail_info::all_col(),
                      this->char_id_,
                      this->sender_id_, this->mail_header_.sender_name_,
                      this->mail_header_.title_, this->mail_header_.send_time_,
                      this->mail_header_.readed_, this->mail_detail_.attach_num_,
                      this->mail_header_.mail_type_, this->mail_detail_.coin_,
                      this->mail_detail_.diamond_, this->mail_detail_.b_diamond_, this->mail_detail_.content_,
                      this->mail_detail_.items_, this->delete_time_,
                      this->lvl_limit_);
  }
  int update_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "items='',coin=0,diamond=0,b_diamond=0,attach_num=0 "
                      "where id=%d",
                      db_tb_mail_info,
                      this->id_);
  }
  int update_mail_readed_sql(char *bf, const int bf_len)
  {
    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "readed=1 where id=%d",
                      db_tb_mail_info,
                      this->id_);
  }
  int update_mail_delete_sql(char *bf, const int bf_len, ilist<int> &mail_id_list)
  {
    if (mail_id_list.empty()) return 0;

    static char id_bf[1024] = {0};
    ::memset(id_bf, '0', sizeof(id_bf));
    int mail_id = mail_id_list.pop_front();
    int len = ::snprintf(id_bf, sizeof(id_bf), "%d", mail_id);
    while (!mail_id_list.empty())
    {
      mail_id = mail_id_list.pop_front();
      len += ::snprintf(id_bf + len, sizeof(id_bf) - len, ",%d", mail_id);
    }

    return ::snprintf(bf, bf_len,
                      "update %s set "
                      "delete_time=%d where char_id=%d and id in (%s)",
                      db_tb_mail_info,
                      this->delete_time_, this->char_id_, id_bf);
  }
public:
  void reset()
  {
    this->id_           = 0;
    this->char_id_      = 0;
    this->sender_id_    = 0;
    this->delete_time_  = 0;
    this->lvl_limit_    = 0;
    this->mail_header_.reset();
    this->mail_detail_.reset();
  }
public:
  int  id_;                                 // 邮件id
  int  char_id_;                            // 收件人id
  int  sender_id_;                          // 发件人id
  int  delete_time_;                        // 邮件删除时间
  int  lvl_limit_;                          // 等级限制
  mail_header mail_header_;
  mail_detail mail_detail_;
};
inline out_stream & operator << (out_stream &os, const mail_info *info)
{
  os << info->id_
    << info->char_id_
    << info->sender_id_
    << info->delete_time_
    << info->lvl_limit_
    << &info->mail_header_
    << &info->mail_detail_;
  return os;
}
inline in_stream & operator >> (in_stream &is, mail_info *info)
{
  is >> info->id_
    >> info->char_id_
    >> info->sender_id_
    >> info->delete_time_
    >> info->lvl_limit_
    >> &info->mail_header_
    >> &info->mail_detail_;
  return is;
}
#endif // MAIL_INFO_H_
