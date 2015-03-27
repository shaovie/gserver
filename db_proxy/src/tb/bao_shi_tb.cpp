#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "bao_shi_info.h"
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
    bao_shi_info data_;
  };

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }

      void read_sql_row(bao_shi_info *data, MYSQL_ROW &row)
      {
        data->char_id_ = ::atoi(row[bao_shi_info::IDX_CHAR_ID]);
        data->pos_     = ::atoi(row[bao_shi_info::IDX_POS]);
        data->bs_idx_  = ::atoi(row[bao_shi_info::IDX_BS_IDX]);
        data->bs_lvl_  = ::atoi(row[bao_shi_info::IDX_BS_LVL]);
        data->up_cnt_  = ::atoi(row[bao_shi_info::IDX_UP_CNT]);
        data->bs_exp_  = ::atoi(row[bao_shi_info::IDX_BS_EXP]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while ((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(bao_shi_info));
          ar->push_back(mb);

          bao_shi_info *info = (bao_shi_info *)mb->data();
          info->reset();
          this->read_sql_row(info, row);
          mb->wr_ptr(sizeof(bao_shi_info));
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
  static tb_msg_maper tm_get(REQ_GET_BAO_SHI, new tb_msg_get(RES_GET_BAO_SHI));

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
      char bf[sizeof(bao_shi_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(bao_shi_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(bao_shi_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_BAO_SHI, new tb_msg_insert(RES_INSERT_BAO_SHI));

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
      char bf[sizeof(bao_shi_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(bao_shi_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(bao_shi_info));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_BAO_SHI, new tb_msg_update(RES_UPDATE_BAO_SHI));
}
