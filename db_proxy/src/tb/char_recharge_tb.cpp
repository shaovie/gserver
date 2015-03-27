#include "db_async_store.h"
#include "char_recharge.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "icache.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    char_recharge data_;
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

      void read_sql_row(char_recharge *data, MYSQL_ROW &row)
      {
        data->char_id_          = ::atoi(row[char_recharge::IDX_CHAR_ID]);
        data->last_buy_mc_time_ = ::atoi(row[char_recharge::IDX_LAST_BUY_MC_TIME]);
        ::strncpy(data->data_, row[char_recharge::IDX_DATA], sizeof(data->data_) - 1);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(char_recharge));
        char_recharge *ci = (char_recharge *)mb->data();
        ci->reset();
        this->read_sql_row(ci, row);
        mb->wr_ptr(sizeof(char_recharge));
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int char_id = 0;
      in_stream is(msg, len);
      is >> db_sid >> char_id;

      sql_get *sql = new sql_get();
      sql->data_.char_id_ = char_id;
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_CHAR_RECHARGE, new tb_msg_get(RES_GET_CHAR_RECHARGE));

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
      char bf[sizeof(char_recharge) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_recharge));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(char_recharge));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_CHAR_RECHARGE, new tb_msg_update(RES_UPDATE_CHAR_RECHARGE));
}
