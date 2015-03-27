#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "vip_info.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "icache.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  static icache<int, vip_info, obj_pool_std_allocator<vip_info> > s_cache(CACHE_OBJ_SIZE);

  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    vip_info data_;
  };

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }

      void read_sql_row(vip_info *data, MYSQL_ROW &row)
      {
        data->char_id_ = ::atoi(row[vip_info::IDX_CHAR_ID]);
        data->vip_lvl_ = ::atoi(row[vip_info::IDX_VIP_LVL]);
        data->vip_exp_ = ::atoi(row[vip_info::IDX_VIP_EXP]);
        data->guild_jb_cnt_ = ::atoi(row[vip_info::IDX_GUILD_JB_CNT]);
        data->buy_ti_li_cnt_ = ::atoi(row[vip_info::IDX_BUY_TI_LI_CNT]);
        data->exchange_coin_cnt_ = ::atoi(row[vip_info::IDX_EXCHANGE_COIN_CNT]);
        data->free_relive_cnt_ = ::atoi(row[vip_info::IDX_FREE_RELIVE_CNT]);
        ::strncpy(data->get_idxs_, row[vip_info::IDX_GET_IDXS], sizeof(data->get_idxs_) - 1);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(vip_info));
        ar->push_back(mb);

        vip_info info;
        this->read_sql_row(&info, row);
        s_cache.insert(info.char_id_, &info);
        out_stream os(mb->wr_ptr(), mb->size());
        os << &info;
        mb->wr_ptr(os.length());
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
      vip_info *info = s_cache.find(char_id);
      if (info != NULL)
      {
        // 如果重载了 operator << (out_stream &os, const T *) 要注意了
        char bf[sizeof(vip_info)] = {0};
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
  static tb_msg_maper tm_get(REQ_GET_VIP, new tb_msg_get(RES_GET_VIP));

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
      sql_insert *sql = new sql_insert();
      in_stream is(msg, len);
      is >> db_sid >> &sql->data_;

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
  static tb_msg_maper tm_insert(REQ_INSERT_VIP, new tb_msg_insert(RES_INSERT_VIP));

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
      is >> db_sid >> &sql->data_;

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
  static tb_msg_maper tm_update(REQ_UPDATE_VIP, new tb_msg_update(RES_UPDATE_VIP));
}
