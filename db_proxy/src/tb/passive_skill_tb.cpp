#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "passive_skill_info.h"
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
    passive_skill_info data_;
  };

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
          mblock *mb = mblock_pool::instance()->alloc(sizeof(passive_skill_info));
          passive_skill_info *p = (passive_skill_info *)mb->data();
          p->reset();
          this->read_sql_row(p, row);
          mb->wr_ptr(sizeof(passive_skill_info));
          ar->push_back(mb);
        }
      }
      void read_sql_row(passive_skill_info *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[passive_skill_info::IDX_CHAR_ID]);
        data->cid_         = ::atoi(row[passive_skill_info::IDX_CID]);
        data->lvl_         = ::atoi(row[passive_skill_info::IDX_LVL]);
        data->up_cnt_      = ::atoi(row[passive_skill_info::IDX_UP_CNT]);
        data->bless_       = ::atoi(row[passive_skill_info::IDX_BLESS]);
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
  static tb_msg_maper tm_passive_skill_get_list(REQ_GET_PASSIVE_SKILL_LIST, new tb_msg_get(RES_GET_PASSIVE_SKILL_LIST));

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
      char bf[sizeof(passive_skill_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(passive_skill_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(passive_skill_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_PASSIVE_SKILL, new tb_msg_update(RES_UPDATE_PASSIVE_SKILL));
}
