#include "mail_info.h"
#include "mblock_pool.h"
#include "db_async_store.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    mail_info data_;
  };
  class tb_msg_get : public tb_msg_handler
  {
    friend class tb_msg_get_system_mail_list;
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }

      void read_sql_row(mail_info *data, MYSQL_ROW &row)
      {
        data->id_          = ::atoi(row[mail_info::IDX_ID]);
        data->char_id_     = ::atoi(row[mail_info::IDX_CHAR_ID]);
        data->sender_id_   = ::atoi(row[mail_info::IDX_SENDER_ID]);
        data->delete_time_ = ::atoi(row[mail_info::IDX_DELETE_TIME]);
        data->lvl_limit_   = ::atoi(row[mail_info::IDX_LVL_LIMIT]);
        data->mail_header_.send_time_  = ::atoi(row[mail_info::IDX_SEND_TIME]);
        data->mail_header_.readed_     = ::atoi(row[mail_info::IDX_READED]);
        data->mail_detail_.attach_num_ = ::atoi(row[mail_info::IDX_ATTACH_NUM]);
        data->mail_header_.mail_type_  = ::atoi(row[mail_info::IDX_MAIL_TYPE]);
        data->mail_detail_.coin_       = ::atoi(row[mail_info::IDX_COIN]);
        data->mail_detail_.diamond_    = ::atoi(row[mail_info::IDX_DIAMOND]);
        data->mail_detail_.b_diamond_  = ::atoi(row[mail_info::IDX_B_DIAMOND]);

        const int len = sizeof(data->mail_header_.sender_name_) - 1;
        ::strncpy(data->mail_header_.sender_name_, row[mail_info::IDX_SENDER_NAME], len);
        ::strncpy(data->mail_header_.title_, row[mail_info::IDX_TITLE], sizeof(data->mail_header_.title_) - 1);
        ::strncpy(data->mail_detail_.content_, row[mail_info::IDX_CONTENT], sizeof(data->mail_detail_.content_) - 1);
        ::strncpy(data->mail_detail_.items_, row[mail_info::IDX_ITEMS], sizeof(data->mail_detail_.items_) - 1);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while ((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(mail_info));
          ar->push_back(mb);

          mail_info info;
          this->read_sql_row(&info, row);
          out_stream os(mb->wr_ptr(), mb->size());
          os << &info;
          mb->wr_ptr(os.length());
        }
      }
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get *sql = new sql_get();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_MAIL_INFO, new tb_msg_get(RES_GET_MAIL_INFO));

  class tb_msg_get_mail_attach : public tb_msg_handler
  {
    class sql_get_mail_attach : public sql
    {
    public:
      sql_get_mail_attach() : sender_id_(0) { }
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.get_mail_attach_sql(bf, bf_len); }

      void read_sql_row(mail_detail *data, MYSQL_ROW &row)
      {
        data->id_            = ::atoi(row[0]);
        data->char_id_       = ::atoi(row[1]);
        data->attach_num_    = ::atoi(row[2]);
        data->coin_          = ::atoi(row[3]);
        data->diamond_       = ::atoi(row[4]);
        data->b_diamond_     = ::atoi(row[5]);
        ::strncpy(data->items_, row[6], sizeof(data->items_) - 1);
        this->sender_id_ = ::atoi(row[7]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(mail_detail) + sizeof(int));
        ar->push_back(mb);

        mail_detail info;
        this->read_sql_row(&info, row);
        out_stream os(mb->wr_ptr(), mb->size());
        os << &info << this->sender_id_;
        mb->wr_ptr(os.length());
      }
    private:
      int sender_id_;
    };
  public:
    tb_msg_get_mail_attach(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get_mail_attach *sql = new sql_get_mail_attach();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get_mail_attach(REQ_GET_MAIL_ATTACH, new tb_msg_get_mail_attach(RES_GET_MAIL_ATTACH));

  class tb_msg_get_mail_list : public tb_msg_handler
  {
    class sql_get_mail_list : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.get_mail_list_sql(bf, bf_len); }

      void read_sql_row(mail_header *data, MYSQL_ROW &row)
      {
        data->id_            = ::atoi(row[0]);
        ::strncpy(data->sender_name_, row[1], sizeof(data->sender_name_) - 1);
        ::strncpy(data->title_,       row[2], sizeof(data->title_) - 1);
        data->send_time_     = ::atoi(row[3]);
        data->readed_        = ::atoi(row[4]);
        int attach_num       = ::atoi(row[5]);
        data->mail_type_     = ::atoi(row[6]);
        int coin             = ::atoi(row[7]);
        int diamond          = ::atoi(row[8]);
        int b_diamond        = ::atoi(row[9]);
        if (attach_num > 0 || coin > 0 || diamond > 0 || b_diamond > 0)
          data->has_attach_ = 1;
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(mail_header));
          ar->push_back(mb);

          mail_header info;
          this->read_sql_row(&info, row);
          out_stream os(mb->wr_ptr(), mb->size());
          os << &info;
          mb->wr_ptr(os.length());
        }
      }
    };
  public:
    tb_msg_get_mail_list(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get_mail_list *sql = new sql_get_mail_list();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get_mail_list(REQ_GET_MAIL_LIST, new tb_msg_get_mail_list(RES_GET_MAIL_LIST));

  class tb_msg_get_system_mail_list : public tb_msg_handler
  {
    class sql_get_system_mail_list : public tb_msg_get::sql_get
    {
    public:
      sql_get_system_mail_list() :
        level_(0),
        limit_time_(0),
        once_get_system_cnt_(0)
      { }
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      {
        return this->data_.get_system_mail_list_sql(bf, bf_len,
                                                    this->limit_time_,
                                                    this->once_get_system_cnt_,
                                                    this->level_);
      }
    public:
      short level_;
      int   limit_time_;
      char  once_get_system_cnt_;
    };
  public:
    tb_msg_get_system_mail_list(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get_system_mail_list *sql = new sql_get_system_mail_list();
      in_stream is(msg, len);
      is >> db_sid
        >> sql->data_.char_id_
        >> sql->limit_time_
        >> sql->once_get_system_cnt_
        >> sql->level_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get_system_mail_list(REQ_GET_SYSTEM_MAIL_LIST,
                                              new tb_msg_get_system_mail_list(RES_GET_SYSTEM_MAIL_LIST));

  class tb_msg_check_new_mail : public tb_msg_handler
  {
    class sql_check_new_mail : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.check_new_mail_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;

        mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
        ar->push_back(mb);
        *mb << ::atoi(row[0]);
      }
    };
  public:
    tb_msg_check_new_mail(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_check_new_mail *sql = new sql_check_new_mail();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_check_new_mail(REQ_CHECK_NEW_MAIL, new tb_msg_check_new_mail(RES_CHECK_NEW_MAIL));

  class tb_msg_insert : public tb_msg_handler
  {
    class sql_insert : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_INSERT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.insert_sql(bf, bf_len); }
    };
  public:
    tb_msg_insert(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      in_stream is(msg, len);
      sql_insert *sql = new sql_insert();
      is >> db_sid >> &sql->data_;
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_MAIL, new tb_msg_insert(RES_INSERT_MAIL));

  class tb_msg_update_mail_attach : public tb_msg_handler
  {
    class sql_update_mail_attach : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_sql(bf, bf_len); }
    };
  public:
    tb_msg_update_mail_attach(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_update_mail_attach *sql = new sql_update_mail_attach();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update_mail_attach(REQ_UPDATE_MAIL_ATTACH, new tb_msg_update_mail_attach(RES_UPDATE_MAIL_ATTACH));

  class tb_msg_update_mail_readed : public tb_msg_handler
  {
    class sql_update_mail_readed : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_mail_readed_sql(bf, bf_len); }
    };
  public:
    tb_msg_update_mail_readed(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_update_mail_readed *sql = new sql_update_mail_readed();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.id_; // sid, char_id, mail_id

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update_mail_readed(REQ_UPDATE_MAIL_READED, new tb_msg_update_mail_readed(RES_UPDATE_MAIL_READED));

  class tb_msg_delete : public tb_msg_handler
  {
    class sql_delete : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_mail_delete_sql(bf, bf_len, this->mail_id_list_); }

      ilist<int> mail_id_list_;
    };
  public:
    tb_msg_delete(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(mail_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      sql_delete *sql = new sql_delete();
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.delete_time_;

      int count = 0;
      is >> count;
      int mail_id  = 0;
      for (int i = 0; i < count; ++i)
      {
        is >> mail_id;
        sql->mail_id_list_.push_back(mail_id);
      }

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_delete(REQ_UPDATE_MAIL_DELETE, new tb_msg_delete(RES_UPDATE_MAIL_DELETE));
}
