#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "char_db_msg_queue.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    char_db_msg_queue data_;
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
    tb_msg_insert(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(char_db_msg_queue) + 4] = {0};
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_db_msg_queue));

      sql_insert *sql = new sql_insert();
      ::memcpy((char*)&sql->data_, si.str(), sizeof(char_db_msg_queue));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;

    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_CHAR_DB_MSG, new tb_msg_insert(RES_INSERT_CHAR_DB_MSG));

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
    tb_msg_delete(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_delete *sql = new sql_delete();
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
  static tb_msg_maper tm_delete(REQ_DELETE_CHAR_DB_MSG, new tb_msg_delete(RES_DELETE_CHAR_DB_MSG));

  class tb_msg_get : public tb_msg_handler
  {
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
          mblock *mb = mblock_pool::instance()->alloc(sizeof(char_db_msg_queue));
          char_db_msg_queue *p = (char_db_msg_queue*)mb->data();
          p->reset();
          this->read_sql_row(p, row);
          mb->wr_ptr(sizeof(char_db_msg_queue));
          ar->push_back(mb);
        }
      }
      void read_sql_row(char_db_msg_queue *data, MYSQL_ROW &row)
      {
        data->char_id_      = ::atoi(row[char_db_msg_queue::IDX_CHAR_ID]);
        data->msg_id_       = ::atoi(row[char_db_msg_queue::IDX_MSG_ID]);
        ::strncpy(data->param_, row[char_db_msg_queue::IDX_PARAM], sizeof(data->param_) - 1);
      }
    };
  public:
    tb_msg_get(const int resp_id) : resp_id_(resp_id) { }
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
  static tb_msg_maper tm_get(REQ_GET_CHAR_DB_MSG_LIST, new tb_msg_get(RES_GET_CHAR_DB_MSG_LIST));
}
