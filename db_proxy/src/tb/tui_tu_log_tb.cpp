#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "tui_tu_log.h"
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
    tui_tu_log data_;
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
          mblock *mb = mblock_pool::instance()->alloc(sizeof(tui_tu_log));
          tui_tu_log *si = (tui_tu_log *)mb->data();
          si->reset();
          this->read_sql_row(si, row);
          mb->wr_ptr(sizeof(tui_tu_log));
          ar->push_back(mb);
        }
      }
      void read_sql_row(tui_tu_log *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[tui_tu_log::IDX_CHAR_ID]);
        data->cid_         = ::atoi(row[tui_tu_log::IDX_CID]);
        data->used_time_   = ::atoi(row[tui_tu_log::IDX_USED_TIME]);
        data->first_award_ = ::atoi(row[tui_tu_log::IDX_FIRST_AWARD]);
        data->state_       = ::atoi(row[tui_tu_log::IDX_STATE]);
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
  static tb_msg_maper tm_get(REQ_GET_TUI_TU_LOG, new tb_msg_get(RES_GET_TUI_TU_LOG));

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
      char bf[sizeof(tui_tu_log) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(tui_tu_log));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(tui_tu_log));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_TUI_TU_LOG, new tb_msg_insert(RES_INSERT_TUI_TU_LOG));

  class tb_msg_update : public tb_msg_handler
  {
    class sql_update : public sql
    {
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_sql(bf, bf_len); }
    };
  public:
    tb_msg_update(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(tui_tu_log) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(tui_tu_log));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(tui_tu_log));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_TUI_TU_LOG, new tb_msg_update(RES_UPDATE_TUI_TU_LOG));
}
