#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "social_info.h"
#include "sys_log.h"
#include "istream.h"
#include "message.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    social_info data_;
  };

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(social_info));
          social_info *si = (social_info *)mb->data();
          si->reset();
          this->read_sql_row(si, row);
          mb->wr_ptr(sizeof(social_info));
          ar->push_back(mb);
        }
      }
      void read_sql_row(social_info *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[social_info::IDX_CHAR_ID]);
        data->relation_    = ::atoi(row[social_info::IDX_RELATION]);
        data->socialer_id_ = ::atol(row[social_info::IDX_SOCIALER_ID]);
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
  static tb_msg_maper tm_get(REQ_GET_SOCIAL_LIST, new tb_msg_get(RES_GET_SOCIAL_LIST));

  class tb_msg_insert : public tb_msg_handler
  {
    class sql_insert : public sql
    {
      virtual int opt_type() { return sql_opt::SQL_INSERT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.insert_sql(bf, bf_len); }
    };
  public:
    tb_msg_insert(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(social_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(social_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(social_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_SOCIAL, new tb_msg_insert(RES_INSERT_SOCIAL));

  class tb_msg_delete : public tb_msg_handler
  {
    class sql_delete : public sql
    {
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
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.relation_ >> sql->data_.socialer_id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_delete(REQ_DELETE_SOCIAL, new tb_msg_delete(RES_DELETE_SOCIAL));
}
