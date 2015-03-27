#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "item_obj.h"
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
    item_obj data_;
  };
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
      sql_insert *sql = new sql_insert();
      in_stream is(msg, len);
      is >> db_sid >> &(sql->data_);

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_ITEM, new tb_msg_insert(RES_INSERT_ITEM));

  class tb_msg_get : public tb_msg_handler
  {
  public:
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while ((row = mysql_fetch_row(res)) != NULL)
        {
          item_obj item;
          this->read_sql_row(&item, row);
          mblock *mb = mblock_pool::instance()->alloc(sizeof(item_obj) + MAX_ITEM_EXTRA_INFO_LEN + 8);
          out_stream os(mb->wr_ptr(), mb->space());
          os << &item;
          mb->wr_ptr(os.length());
          ar->push_back(mb);
        }
      }
      void read_sql_row(item_obj *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[item_obj::IDX_CHAR_ID]);
        data->id_          = ::atoi(row[item_obj::IDX_ID]);
        data->pkg_         = ::atoi(row[item_obj::IDX_PKG]);
        data->cid_         = ::atoi(row[item_obj::IDX_CID]);
        data->bind_        = ::atoi(row[item_obj::IDX_BIND]);
        data->amount_      = ::atoi(row[item_obj::IDX_AMOUNT]);
        data->attr_float_coe_ = ::atoi(row[item_obj::IDX_ATTR_FLOAT_COE]);

        int l = ::strlen(row[item_obj::IDX_EXTRA_INFO]);
        if (l > 0)
        {
          data->extra_info_ = mblock_pool::instance()->alloc(l + 1);
          data->extra_info_->copy(row[item_obj::IDX_EXTRA_INFO], l);
          data->extra_info_->set_eof();
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
  static tb_msg_maper tm_get(REQ_GET_ITEM_LIST, new tb_msg_get(RES_GET_ITEM_LIST));

  class tb_msg_get_single_pkg : public tb_msg_handler
  {
    class sql_get_single_pkg : public tb_msg_get::sql_get
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_single_pkg_sql(bf, bf_len); }
    };
  public:
    tb_msg_get_single_pkg(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get_single_pkg *sql = new sql_get_single_pkg();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.pkg_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         sql->data_.char_id_,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get_single_pkg(REQ_GET_ITEM_SINGLE_PKG,
                                        new tb_msg_get_single_pkg(RES_GET_ITEM_SINGLE_PKG));

  static tb_msg_maper tm_corss_get_single_pkg(REQ_CROSS_GET_ITEM_SINGLE_PKG,
                                              new tb_msg_get_single_pkg(RES_CROSS_GET_ITEM_SINGLE_PKG));

  static tb_msg_maper tm_get_castellan_single_pkg(REQ_GET_CASTELLAN_ITEM_SINGLE_PKG,
                                              new tb_msg_get_single_pkg(RES_GET_CASTELLAN_ITEM_SINGLE_PKG));


  class tb_msg_update : public tb_msg_handler
  {
    class sql_update : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_sql(bf, bf_len); }
    };
  public:
    tb_msg_update(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_update *sql = new sql_update();
      in_stream is(msg, len);
      is >> db_sid >> &(sql->data_);

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_ITEM, new tb_msg_update(RES_UPDATE_ITEM));

  class tb_msg_delete : public tb_msg_handler
  {
    class sql_delete : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_DELETE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.delete_sql(bf, bf_len); }
    };
  public:
    tb_msg_delete(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_delete *sql = new sql_delete();
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
  static tb_msg_maper tm_delete(REQ_DELETE_ITEM, new tb_msg_delete(RES_DELETE_ITEM));
}
