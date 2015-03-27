#include "db_async_store.h"
#include "huo_yue_du_info.h"
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
    virtual int id() { return this->data_.char_id_; }
    huo_yue_du_info data_;
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

      void read_sql_row(huo_yue_du_info *data, MYSQL_ROW &row)
      {
        data->char_id_   = ::atoi(row[huo_yue_du_info::IDX_CHAR_ID]);
        data->cid_       = ::atoi(row[huo_yue_du_info::IDX_CID]);
        data->score_     = ::atoi(row[huo_yue_du_info::IDX_SCORE]);
        data->get_award_ = ::atoi(row[huo_yue_du_info::IDX_GET_AWARD]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(huo_yue_du_info));
          huo_yue_du_info *hydi = (huo_yue_du_info *)mb->data();
          hydi->reset();
          this->read_sql_row(hydi, row);
          mb->wr_ptr(sizeof(huo_yue_du_info));
          ar->push_back(mb);
        }
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
  static tb_msg_maper tm_get(REQ_GET_HUO_YUE_DU_INFO, new tb_msg_get(RES_GET_HUO_YUE_DU_INFO));

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
      char bf[sizeof(huo_yue_du_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(huo_yue_du_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(huo_yue_du_info));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_HUO_YUE_DU_INFO, new tb_msg_update(RES_UPDATE_HUO_YUE_DU_INFO));

  class tb_msg_clear : public tb_msg_handler
  {
    class sql_clear : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.clear_sql(bf, bf_len); }
    };
  public:
    tb_msg_clear(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_clear *sql = new sql_clear();
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
  static tb_msg_maper tm_clear(REQ_CLEAR_HUO_YUE_DU_INFO, new tb_msg_clear(RES_CLEAR_HUO_YUE_DU_INFO));
}
