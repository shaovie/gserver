#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "sys_settings.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "icache.h"

#include <string>

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  static icache<int, sys_settings, obj_pool_std_allocator<sys_settings> > s_cache(CACHE_OBJ_SIZE);

  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    sys_settings data_;
  };
  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      void read_sql_row(sys_settings *data, MYSQL_ROW &row)
      {
        data->char_id_ = ::atoi(row[sys_settings::IDX_CHAR_ID]);
        ::strncpy(data->data_, row[sys_settings::IDX_DATA], sizeof(data->data_) - 1);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;

        sys_settings tb;
        this->read_sql_row(&tb, row);
        s_cache.insert(tb.char_id_, &tb);

        mblock *mb = mblock_pool::instance()->alloc(sizeof(sys_settings));
        out_stream os(mb->data(), mb->space());
        os << &tb;
        mb->wr_ptr(os.length());
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

      // use cache
      sys_settings *info = s_cache.find(char_id);
      if (info != NULL)
      {
        // 如果重载了 operator << (out_stream &os, const T *) 要注意了
        char bf[sizeof(sys_settings)] = {0};
        out_stream os(bf, sizeof(bf));
        os << info;
        return po->proc_result(db_sid,
                               0,
                               this->resp_id_,
                               DB_ASYNC_OK,
                               os.length(),
                               bf);
      }

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
  static tb_msg_maper tm_get_list(REQ_GET_SYS_SETTINGS, new tb_msg_get(RES_GET_SYS_SETTINGS));

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
      sql_update *sql = new sql_update();
      in_stream is(msg, len);
      is >> db_sid >> &(sql->data_);

      // use cache
      s_cache.insert(sql->data_.char_id_, &(sql->data_));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_SYS_SETTINGS, new tb_msg_update(RES_UPDATE_SYS_SETTINGS));
}
