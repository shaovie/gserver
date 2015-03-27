#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "skill_info.h"
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
    skill_info data_;
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
      char bf[sizeof(skill_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(skill_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(skill_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_SKILL, new tb_msg_insert(RES_INSERT_SKILL));

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
          mblock *mb = mblock_pool::instance()->alloc(sizeof(skill_info));
          skill_info *p = (skill_info *)mb->data();
          p->reset();
          this->read_sql_row(p, row);
          mb->wr_ptr(sizeof(skill_info));
          ar->push_back(mb);
        }
      }
      void read_sql_row(skill_info *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[skill_info::IDX_CHAR_ID]);
        data->cid_         = ::atoi(row[skill_info::IDX_CID]);
        data->lvl_         = ::atoi(row[skill_info::IDX_LVL]);
        data->use_time_    = ::atoi(row[skill_info::IDX_USE_TIME]);
      }
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int arg = 0;
      sql_get *sql = new sql_get();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_ >> arg;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         arg,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_skill_get(REQ_GET_SKILL_LIST, new tb_msg_get(RES_GET_SKILL_LIST));
  static tb_msg_maper tm_skill_get_other(REQ_GET_OTHER_SKILL_LIST, new tb_msg_get(RES_GET_OTHER_SKILL_LIST));

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
      char bf[sizeof(skill_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(skill_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(skill_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_SKILL, new tb_msg_update(RES_UPDATE_SKILL));
}
