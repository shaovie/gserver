#include "db_async_store.h"
#include "mall_buy_log.h"
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
  class tb_msg_insert : public tb_msg_handler
  {
    class sql_insert : public sql_opt
    {
    public:
      virtual int id() { return this->data_.char_id_; }
      virtual int opt_type() { return sql_opt::SQL_INSERT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.insert_sql(bf, bf_len); }

      mall_buy_log data_;
    };
  public:
    tb_msg_insert(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(mall_buy_log) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(mall_buy_log));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(mall_buy_log));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_MALL_BUY_LOG, new tb_msg_insert(RES_INSERT_MALL_BUY_LOG));

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql_opt
    {
    public:
      virtual int id() { return this->data_.char_id_; }
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }

      void read_sql_row(mall_buy_log *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[mall_buy_log::IDX_CHAR_ID]);
        data->npc_cid_     = ::atoi(row[mall_buy_log::IDX_NPC_CID]);
        data->item_cid_    = ::atoi(row[mall_buy_log::IDX_ITEM_CID]);
        data->amount_      = ::atoi(row[mall_buy_log::IDX_AMOUNT]);
        data->buy_time_    = ::atoi(row[mall_buy_log::IDX_BUY_TIME]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        while ((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(mall_buy_log));
          mall_buy_log *ci = (mall_buy_log *)mb->data();
          ci->reset();
          this->read_sql_row(ci, row);
          mb->wr_ptr(sizeof(mall_buy_log));
          ar->push_back(mb);
        }
      }
      mall_buy_log data_;
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get *sql = new sql_get();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> sql->data_.buy_time_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_MALL_BUY_LOG, new tb_msg_get(RES_GET_MALL_BUY_LOG));
}
