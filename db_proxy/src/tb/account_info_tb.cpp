#include "db_async_store.h"
#include "account_info.h"
#include "mblock_pool.h"
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
    virtual int id()
    {
      int n = 0;
      char *p = this->data_.account_;
      while (*p != '\0') n += *p++;
      return n;
    }
    void read_sql_row(account_info *data, MYSQL_ROW &row)
    {
      ::strncpy(data->account_, row[account_info::IDX_ACCOUNT], sizeof(data->account_) - 1);
      data->track_st_   = ::atoi(row[account_info::IDX_TRACK_ST]);
      data->diamond_    = ::atoi(row[account_info::IDX_DIAMOND]);
      data->char_id_    = ::atoi(row[account_info::IDX_CHAR_ID]);
      ::strncpy(data->channel_, row[account_info::IDX_CHANNEL], sizeof(data->channel_) - 1);
      data->c_time_     = ::atoi(row[account_info::IDX_C_TIME]);
    }
    account_info data_;
  };
  //=
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
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(account_info));
        account_info *ci = (account_info *)mb->data();
        ci->reset();
        this->read_sql_row(ci, row);
        mb->wr_ptr(sizeof(account_info));
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get *sql = new sql_get();
      stream_istr si(sql->data_.account_, sizeof(sql->data_.account_));
      in_stream is(msg, len);
      is >> db_sid >> si;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  tb_msg_maper tm_get(REQ_GET_ACCOUNT_INFO, new tb_msg_get(RES_GET_ACCOUNT_INFO));

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
      char bf[sizeof(account_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(account_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(account_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_ACCOUNT_INFO, new tb_msg_insert(RES_INSERT_ACCOUNT_INFO));

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
    tb_msg_update(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(account_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(account_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(account_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_ACCOUNT_INFO, new tb_msg_update(RES_UPDATE_ACCOUNT_INFO));

  class tb_msg_set_diamond : public tb_msg_handler
  {
    class sql_update : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.set_diamond_sql(bf, bf_len); }
    };
  public:
    tb_msg_set_diamond(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int diamond = 0;
      char account[MAX_ACCOUNT_LEN + 4];
      stream_istr si(account, sizeof(account));

      in_stream is(msg, len);
      is >> db_sid >> si >> diamond;

      sql_update *sql = new sql_update();
      ::memcpy(sql->data_.account_, account, MAX_ACCOUNT_LEN);
      sql->data_.diamond_ = diamond;
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_set_diamond(REQ_SET_DIAMOND, new tb_msg_update(RES_SET_DIAMOND));
}
