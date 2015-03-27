#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "task_bit_array.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "icache.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  static icache<int, task_bit_array, obj_pool_std_allocator<task_bit_array> > s_cache(CACHE_OBJ_SIZE);

  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    task_bit_array data_;
  };
  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      void read_sql_row(task_bit_array *data, MYSQL_ROW &row)
      {
        data->char_id_     = ::atoi(row[task_bit_array::IDX_CHAR_ID]);
        ::strncpy(data->bit_[0], row[task_bit_array::IDX_TRUNK_BIT_ID], HALF_TASK_NUM);
        ::strncpy(data->bit_[1], row[task_bit_array::IDX_BRANCH_BIT_ID], HALF_TASK_NUM);
        data->len_[0]      = ::strlen(data->bit_[0]);
        data->len_[1]      = ::strlen(data->bit_[1]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;

        task_bit_array tba;
        this->read_sql_row(&tba, row);
        s_cache.insert(tba.char_id_, &tba);

        mblock *mb = mblock_pool::instance()->alloc(sizeof(task_bit_array));
        out_stream os(mb->data(), mb->space());
        os << &tba;
        mb->wr_ptr(os.length());
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int char_id = 0;
      in_stream is(msg, len);
      is >> db_sid >> char_id;

      // use cache
      task_bit_array *info = s_cache.find(char_id);
      if (info != NULL)
      {
        // 如果重载了 operator << (out_stream &os, const T *) 要注意了
        char bf[sizeof(task_bit_array)] = {0};
        out_stream os(bf, sizeof(bf));
        os << info;
        return po->proc_result(db_sid,
                               0,
                               RES_GET_TASK_BIT_ARRAY,
                               DB_ASYNC_OK,
                               os.length(),
                               bf);
      }

      sql_get *sql = new sql_get();
      sql->data_.char_id_ = char_id;
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         RES_GET_TASK_BIT_ARRAY,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_TASK_BIT_ARRAY, new tb_msg_get(RES_GET_TASK_BIT_ARRAY));

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
  static tb_msg_maper tm_update(REQ_UPDATE_TASK_BIT_ARRAY, new tb_msg_update(RES_UPDATE_TASK_BIT_ARRAY));
}
